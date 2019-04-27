
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_id gobj_getUnique (t_gobj *x)
{
    return x->g_id[0];
}

static void gobj_setUnique (t_gobj *x, t_id u)
{
    x->g_id[0] = u;
}

void gobj_changeUnique (t_gobj *x, t_id u)
{
    instance_registerRename (x, u); gobj_setUnique (x, u);
}

void gobj_serializeUnique (t_gobj *x, t_symbol *s, t_buffer *b)
{
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, s);
    utils_appendUnique (b, gobj_getUnique (x));
    buffer_appendSemicolon (b);
}

void gobj_saveUniques (t_gobj *x, t_buffer *b, int flags)
{
    if (flags & SAVE_UNDO)        { gobj_serializeUnique (x, sym__tagobject, b); }
    if (flags & SAVE_ENCAPSULATE) { gobj_serializeUnique (x, sym__tagsource, b); }
    if (flags & SAVE_UPDATE)      { gobj_serializeSource (x, sym__tagsource, b); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_id gobj_getSource (t_gobj *x)
{
    return x->g_id[1];
}

static void gobj_setSource (t_gobj *x, t_id u)
{
    x->g_id[1] = u;
}

void gobj_changeSource (t_gobj *x, t_id u)
{
    gobj_setSource (x, u);
}

void gobj_serializeSource (t_gobj *x, t_symbol *s, t_buffer *b)
{
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, s);
    utils_appendUnique (b, gobj_getSource (x));
    buffer_appendSemicolon (b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_id gobj_getNative (t_gobj *x)
{
    return x->g_id[2];
}

static void gobj_setNative (t_gobj *x, t_id u)
{
    x->g_id[2] = u;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void gobj_setIdentifiers (t_gobj *x, t_id u)
{
    gobj_setUnique (x, u);
    gobj_setSource (x, u);
    gobj_setNative (x, u);
}

void gobj_changeIdentifiers (t_gobj *x, t_id u)
{
    gobj_changeUnique (x, u);
    gobj_changeSource (x, u);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int gobj_identifiersHaveChanged (t_gobj *x)
{
    t_id u = gobj_getUnique (x);
    t_id s = gobj_getSource (x);
    t_id n = gobj_getNative (x);
    
    if (n != u || s != u) { return 1; }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
