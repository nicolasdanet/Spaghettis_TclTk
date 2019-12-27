
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* For external objects. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

// -- TODO: In the future, consider to inline those functions.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float space_getFloat0 (t_space *space)
{
    return space->s_float0;
}

t_float space_getFloat1 (t_space *space)
{
    return space->s_float1;
}

t_float space_getFloat2 (t_space *space)
{
    return space->s_float2;
}

t_float space_getFloat3 (t_space *space)
{
    return space->s_float3;
}

t_float space_getFloat4 (t_space *space)
{
    return space->s_float4;
}

t_float space_getFloat5 (t_space *space)
{
    return space->s_float5;
}

t_float space_getFloat6 (t_space *space)
{
    return space->s_float6;
}

t_float space_getFloat7 (t_space *space)
{
    return space->s_float7;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void space_setFloat0 (t_space *space, t_float f)
{
    space->s_float0 = f;
}

void space_setFloat1 (t_space *space, t_float f)
{
    space->s_float1 = f;
}

void space_setFloat2 (t_space *space, t_float f)
{
    space->s_float2 = f;
}

void space_setFloat3 (t_space *space, t_float f)
{
    space->s_float3 = f;
}

void space_setFloat4 (t_space *space, t_float f)
{
    space->s_float4 = f;
}

void space_setFloat5 (t_space *space, t_float f)
{
    space->s_float5 = f;
}

void space_setFloat6 (t_space *space, t_float f)
{
    space->s_float6 = f;
}

void space_setFloat7 (t_space *space, t_float f)
{
    space->s_float7 = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
