#include "dllmain.h"
#include "sdk.h"
#include <thread>
#include <atomic>

Present oPresent = nullptr;
std::atomic<Present*> pOPresent{ &oPresent };
std::atomic<WNDPROC> oWndProc = nullptr;
std::atomic<HWND> gameWindow = nullptr;
std::atomic<HMODULE> g_hModule = NULL;

std::unique_ptr<ID3D11Device, com_deleter> pDevice;
std::unique_ptr<ID3D11DeviceContext, com_deleter> pContext;
std::unique_ptr<ID3D11RenderTargetView, com_deleter> mainRenderTargetView;

std::atomic<bool> isShuttingDown = false;
std::atomic<bool> initialized = false;

std::mutex imguiMutex;

void* m_pThisThread = nullptr;

void InitImGui()
{
    std::lock_guard<std::mutex> lock(imguiMutex);
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    ImGui_ImplWin32_Init(gameWindow);
    ImGui_ImplDX11_Init(pDevice.get(), pContext.get());
    Logger::Info("ImGui initialized");
}

void RenderImGui()
{
    if (!initialized) return;

    std::lock_guard<std::mutex> lock(imguiMutex);
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("ImGui Window");
    // Add ImGui widgets here
    ImGui::End();

    ImGui::Render();
    if (mainRenderTargetView) {
        ID3D11RenderTargetView* rtv = mainRenderTargetView.get();
        pContext->OMSetRenderTargets(1, &rtv, NULL);
    }
    else {
        pContext->OMSetRenderTargets(1, nullptr, NULL);
    }
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void freeConsoles()
{
    if (FreeConsole()) {
        FILE* f;
        freopen_s(&f, "NUL", "w", stdout);
        freopen_s(&f, "NUL", "w", stderr);
        freopen_s(&f, "NUL", "r", stdin);
    }
    PostMessage(GetConsoleWindow(), WM_CLOSE, 0, 0);
    Logger::Info("Console freed and closed");
}

void Shutdown()
{
    if (isShuttingDown.exchange(true) || !initialized)
        return;
    Logger::Critical("Shutting down");
    auto start = std::chrono::high_resolution_clock::now();


    // ImGui cleanup
    {
        std::lock_guard<std::mutex> lock(imguiMutex);
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        if (ImGui::GetCurrentContext())
            ImGui::DestroyContext();
    }

    if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = NULL; }
    if (pContext) { pContext->Release(); pContext = NULL; }
    if (pDevice) { pDevice->Release(); pDevice = NULL; }

    functions::detach(m_pThisThread);
    functions::Uninitialize();

    // Restore original window procedure
    if (oWndProc && gameWindow)
    {
        SetWindowLongPtr(gameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc.load()));
        oWndProc = nullptr;
    }

    // Unbind and shutdown kiero
    kiero::unbind(8);
    kiero::shutdown();

    // Set initialized to false
    initialized = false;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    Logger::Critical("Shutdown took %f seconds", diff.count());

    freeConsoles();

    CreateThread(nullptr, 0, [](LPVOID) -> DWORD {
        FreeLibraryAndExitThread(g_hModule, 0);
        return 0;
        }, nullptr, 0, nullptr);
}

HRESULT __stdcall HookPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    m_pThisThread = functions::attach(functions::getDomain());
    if (!initialized)
    {
        ID3D11Device* deviceTemp = nullptr;
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&deviceTemp))))
        {
            pDevice.reset(deviceTemp);

            ID3D11DeviceContext* contextTemp = nullptr;
            pDevice->GetImmediateContext(&contextTemp);
            pContext.reset(contextTemp);

            DXGI_SWAP_CHAIN_DESC sd;
            if (SUCCEEDED(pSwapChain->GetDesc(&sd)))
            {
                gameWindow = sd.OutputWindow;
            }
            else
            {
                Logger::Critical("Failed to get swap chain description");
                return oPresent ? oPresent(pSwapChain, SyncInterval, Flags) : S_OK;
            }

            ID3D11Texture2D* pBackBuffer = nullptr;
            if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer))))
            {
                ID3D11RenderTargetView* renderTargetViewTemp = nullptr;
                if (SUCCEEDED(pDevice->CreateRenderTargetView(pBackBuffer, NULL, &renderTargetViewTemp)))
                {
                    mainRenderTargetView.reset(renderTargetViewTemp);
                }
                pBackBuffer->Release();
            }

            oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(gameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
            InitImGui();
            initialized = true;
        }
        else {
            Logger::Critical("Failed to initialize D3D11 device");
            return oPresent ? oPresent(pSwapChain, SyncInterval, Flags) : S_OK;
        }
    }

    if (GetAsyncKeyState(VK_END) & 1)
    {
        Shutdown();
        return oPresent ? oPresent(pSwapChain, SyncInterval, Flags) : S_OK;
    }

    RenderImGui(); // Render ImGui in the same thread

    return oPresent ? oPresent(pSwapChain, SyncInterval, Flags) : S_OK;
}

void initvars() {
    if (functions::Initialize(true)) {
        Logger::Info("Il2Cpp initialized");
    }
    else {
        Logger::Critical("Il2Cpp initialization failed, quitting...");
        Sleep(300);
        exit(0);
    }

    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA(NULL));
    uintptr_t gameAssembly = reinterpret_cast<uintptr_t>(GetModuleHandleA("GameAssembly.dll"));
    uintptr_t unityPlayer = reinterpret_cast<uintptr_t>(GetModuleHandleA("UnityPlayer.dll"));

    sdk::GameAssembly = gameAssembly;

    Logger::Info("Base Address: 0x%p", reinterpret_cast<void*>(base));
    Logger::Info("GameAssembly Base Address: 0x%p", reinterpret_cast<void*>(gameAssembly));
    Logger::Info("UnityPlayer Base Address: 0x%p", reinterpret_cast<void*>(unityPlayer));
    printf("---------------------------------------------------------- \n");
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
    Logger::Init();
    system("cls");
    initvars();
    functions::init();

    while (kiero::init(kiero::RenderType::D3D11) != kiero::Status::Success)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    Logger::Info("Hook initialized successfully");
    kiero::bind(8, reinterpret_cast<void**>(&oPresent), HookPresent);

    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        g_hModule = hModule;
        std::thread(MainThread, hModule).detach();

        break;
    case DLL_PROCESS_DETACH:
        Shutdown();
        break;
    }
    return TRUE;
}