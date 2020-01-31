
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "s_utf8.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int sys_isMainThread (void)
{
    #if defined ( PD_BUILDING_APPLICATION )
    
    static pthread_t main; static int once = 0;
    
    pthread_t t = pthread_self();
    
    if (!once) { once = 1; main = t; return 1; }

    return (pthread_equal (main, t) != 0);
    
    #else
    
    return 0;
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if defined ( PD_BUILDING_APPLICATION )

/* Runtime check. */

t_error main_assert (void)
{
    t_error err = PD_ERROR_NONE;
    
    err |= (sizeof (int)        < 4);
    err |= (sizeof (t_keycode)  != sizeof (UCS4_CODE_POINT));
    err |= (sizeof (t_word)     != sizeof (t_float));
    err |= (sizeof (t_word)     != sizeof (double));
    err |= (sizeof (t_word)     != sizeof (t_float64Atomic));
    err |= (sizeof (t_word)     != 8);
    
    return err;
}

int main (int argc, char **argv)
{
    if (main_assert()) { PD_BUG; return EXIT_FAILURE; }
    else {
        PD_ASSERT (sys_isMainThread()); return main_entry (argc, argv);
    }
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
