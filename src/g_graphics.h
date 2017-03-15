
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __g_graphics_h_
#define __g_graphics_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* It aims to manage properties window. */
/* While sending attributes it creates a proxy bound to a unique key name. */
/* This key is used as a label in order to forward changes from the GUI. */
/* This key is used as a master name for the GUI widgets. */
/* Destroying the proxy from the owner side closes the window. */
/* It can also be freely canceled from the interpreter side. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error         stub_new                            (t_pd *owner, void *key, const char *cmd);
void            stub_destroyWithKey                 (void *key);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_guiconnect    *guiconnect_new                     (t_pd *owner);
char            *guiconnect_getBoundAsString        (t_guiconnect *x);

void            guiconnect_release                  (t_guiconnect *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "g_patch.h"
#include "g_canvas.h"
#include "g_iem.h"
#include "g_scalar.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_graphics_h_
