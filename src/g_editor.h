
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __g_editor_h_
#define __g_editor_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_motionfn)  (void *z, t_float deltaX, t_float deltaY, t_float modifier);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _selection {
    t_gobj              *sel_what;
    struct _selection   *sel_next;
    } t_selection;
    
typedef struct _editor {
    t_glist             *e_owner;
    t_proxy             *e_proxy;
    t_box               *e_boxes;
    t_box               *e_selectedText;
    t_selection         *e_selectedObjects;
    t_gobj              *e_grabbed;
    t_clock             *e_clock;
    t_buffer            *e_cachedLines;
    t_outconnect        *e_selectedLineConnection;
    t_atom              e_selectedLine[4];
    t_motionfn          e_fnMotion;
    int                 e_previousX;
    int                 e_previousY;
    int                 e_newX;
    int                 e_newY;
    int                 e_action;
    int                 e_isTextDirty;
    int                 e_hasSelectedline;
    } t_editor;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_gobj *selection_getObject (t_selection *x)
{
    return x->sel_what;
}

static inline t_selection *selection_getNext (t_selection *x)
{
    return x->sel_next;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_editor    *editor_new                 (t_glist *owner);

void        editor_free                 (t_editor *x);
void        editor_selectionAdd         (t_editor *x, t_gobj *y);
int         editor_selectionRemove      (t_editor *x, t_gobj *y);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_editor_h_
