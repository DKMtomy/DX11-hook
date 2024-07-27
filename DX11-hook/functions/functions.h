#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "../pch.h"

// Forward declare Unity::CCamera if necessary
namespace Unity { class CCamera; }

namespace functions
{
    Unity::CCamera* GetMainCamera();
    bool init();
    void detach(void* m_Thread);
    void* attach(void* m_Thread);
    bool Initialize(bool m_WaitForModule = false, int m_MaxSecondsWait = 60);

    void Uninitialize();

    void* getDomain();
}

#endif // FUNCTIONS_H