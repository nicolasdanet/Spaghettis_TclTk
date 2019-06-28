
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int sys_isMainThread (void)
{
    #if PD_WITH_MAIN
    
    static pthread_t main; static int once = 0;
    
    pthread_t t = pthread_self();
    
    if (!once) { once = 1; main = t; return 1; }

    return (pthread_equal (main, t) != 0);
    
    #else
    
    return 0;
    
    #endif // PD_WITH_MAIN
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{ 
    __try {
        main_entry (__argc, __argv);
    }
    __finally { }
    
    return 0;
}

#else
#if PD_WITH_MAIN

int main (int argc, char **argv)
{
    PD_ASSERT (sys_isMainThread()); return main_entry (argc, argv);
}

#endif // PD_WITH_MAIN
#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
