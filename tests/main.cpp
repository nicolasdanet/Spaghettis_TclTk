
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PD_BUILDING_APPLICATION     1
#define PD_WITH_DEADCODE            1
#define PD_WITH_TINYEXPR            0
#define PD_WITH_MAIN                0
#define PD_WITH_DUMMY               1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../src/amalgam.cpp"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../../T/T.c"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "t_atomic.c"
#include "t_random.c"
#include "t_time.c"
#include "t_utf8.c"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int main()
{
    return (ttt_testAll() != TTT_GOOD);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
