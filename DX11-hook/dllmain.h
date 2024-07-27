// dllmain.h
#pragma once
#include "pch.h"
#include "includes.h"

typedef HRESULT(__stdcall* Present)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

struct com_deleter {
    template<typename T>
    void operator()(T* ptr) {
        if (ptr) ptr->Release();
    }
};

extern Present oPresent;
extern std::atomic<Present*> pOPresent;
extern std::atomic<WNDPROC> oWndProc;
extern std::atomic<HWND> gameWindow;
extern std::atomic<HMODULE> g_hModule;

extern std::unique_ptr<ID3D11Device, com_deleter> pDevice;
extern std::unique_ptr<ID3D11DeviceContext, com_deleter> pContext;
extern std::unique_ptr<ID3D11RenderTargetView, com_deleter> mainRenderTargetView;

extern std::atomic<bool> isShuttingDown;
extern std::atomic<bool> initialized;
extern std::atomic<bool> g_firstInjection;

extern std::mutex imguiMutex;

extern void* m_pThisThread;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void InitImGui();
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void freeConsoles();
void Shutdown();
HRESULT __stdcall HookPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
void initvars();
DWORD WINAPI MainThread(LPVOID lpReserved);
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);