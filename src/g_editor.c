
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void editor_selectionAdd (t_editor *x, t_gobj *y)
{
    t_selection *s = (t_selection *)PD_MEMORY_GET (sizeof (t_selection));
    
    s->sel_next = x->e_selectedObjects;
    s->sel_what = y;
    
    x->e_selectedObjects = s;
}

int editor_selectionRemove (t_editor *x, t_gobj *y)
{
    t_selection *s1 = NULL;
    t_selection *s2 = NULL;
    
    s1 = x->e_selectedObjects;
    
    if (selection_getObject (s1) == y) {
        x->e_selectedObjects = selection_getNext (x->e_selectedObjects);
        PD_MEMORY_FREE (s1);
        return 1;
        
    } else {
        for (; (s2 = selection_getNext (s1)); (s1 = s2)) {
            if (selection_getObject (s2) == y) {
                s1->sel_next = selection_getNext (s2);
                PD_MEMORY_FREE (s2);
                return 1;
            }
        }
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
