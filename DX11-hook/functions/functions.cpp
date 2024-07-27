#include "functions.h"
#include "IL2CPP_Resolver.hpp"
#include "../sdk.h"

namespace functions
{


    ::Unity::CCamera* GetMainCamera()
    {
        return ::Unity::Camera::GetMain();
    }

    bool init()
	{
		IL2CPP::Callback::Initialize();
        Logger::Info("Initialized IL2CPP resolver");
        sdk::il2cpp_initialized = true;
		return true;
	}

	bool Initialize(bool m_WaitForModule, int m_MaxSecondsWait)
	{
		return IL2CPP::Initialize(m_WaitForModule, m_MaxSecondsWait);
	}

	void Uninitialize()
	{
		return IL2CPP::Callback::Uninitialize();
	}

    void detach(void* m_Thread)
	{
		IL2CPP::Thread::Detach(m_Thread);
	}

	void* attach(void* m_Thread)
	{
		return IL2CPP::Thread::Attach(m_Thread);
	}

	void* getDomain()
	{
		return IL2CPP::Domain::Get();
	}
}