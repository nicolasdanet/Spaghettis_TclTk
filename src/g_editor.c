
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

static void editor_task (t_editor *x)
{
    int deltaX = x->e_newX - x->e_previousX;
    int deltaY = x->e_newY - x->e_previousY;
    
    canvas_displaceSelectedObjects (x->e_owner, deltaX, deltaY);
        
    x->e_previousX = x->e_newX;
    x->e_previousY = x->e_newY;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_box *editor_fetchBox (t_editor *x, t_object *object)
{
    t_box *box = NULL;
    
    for (box = x->e_boxes; box && box->box_object != object; box = box->box_next) { }
    
    PD_ASSERT (box);
    
    return box;
}

void editor_addBox (t_editor *x, t_object *object)
{
    t_box *box = (t_box *)PD_MEMORY_GET (sizeof (t_box));

    box->box_next               = x->e_boxes;
    box->box_object             = object;
    box->box_owner              = x->e_owner;
    box->box_string             = (char *)PD_MEMORY_GET (0);
    box->box_stringSizeInBytes  = 0;
    
    {
    //
    t_glist *view = canvas_getView (x->e_owner);
    t_error err = string_sprintf (box->box_tag, BOX_TAG_SIZE, ".x%lx.%lxBOX", view, box);
    PD_ASSERT (!err);
    //
    }
    
    x->e_boxes = box;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

t_editor *editor_new (t_glist *owner)
{
    t_editor *x = (t_editor *)PD_MEMORY_GET (sizeof (t_editor));
    
    x->e_owner          = owner;
    x->e_proxy          = proxy_new (cast_pd (owner));
    x->e_clock          = clock_new ((void *)x, (t_method)editor_task);
    x->e_cachedLines    = buffer_new();
    
    return x;
}

void editor_free (t_editor *x)
{
    buffer_free (x->e_cachedLines);
    clock_free (x->e_clock);
    proxy_release (x->e_proxy);
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
