
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __g_graphics_h_
#define __g_graphics_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* A stub aims to manage properties window. */
/* While sending content, a remote is created bound to a unique key name. */
/* This key is used as a label in order to forward changes from the GUI. */
/* This key is used also as a master name for the GUI widgets. */
/* Destroying the remote from the owner side closes the window. */
/* It can be freely canceled (signoff) from the interpreter side. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error stub_new                (t_pd *owner, void *key, const char *cmd);
void    stub_destroyWithKey     (void *key);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* A proxy is used to bind a listener to an unique symbol. */
/* Messages sent to this symbol are forwarded to it. */
/* The listener destroyed, the proxy is kept alive (muted) for a short time. */
/* When the sender is destroyed (signoff) listening is cancelled. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _proxy {
    t_pd        x_pd;           /* Must be the first. */
    t_pd        *x_owner;
    t_symbol    *x_bound;
    } t_proxy;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_proxy *proxy_new      (t_pd *owner);

void    proxy_release   (t_proxy *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_symbol *proxy_getTag (t_proxy *x)
{
    PD_ASSERT (x->x_bound);
    
    return x->x_bound;
}

static inline const char *proxy_getTagAsString (t_proxy *x)
{
    return proxy_getTag (x)->s_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "graphics/patch/g_patch.h"
#include "graphics/patch/g_editor.h"
#include "graphics/patch/g_glist.h"
#include "graphics/patch/g_unique.h"
#include "graphics/patch/g_objects.h"
#include "graphics/iem/g_iem.h"
#include "graphics/scalars/g_scalar.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_graphics_h_
