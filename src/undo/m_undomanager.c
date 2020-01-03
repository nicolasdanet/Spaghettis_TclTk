
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../d_dsp.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define UNDOMANAGER_DELAY   227.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undomanager_collapse (t_undomanager *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void undomanager_task (t_undomanager *x)
{
    if (editor_getAction (glist_getEditor (x->um_owner)) == ACTION_NONE) { undomanager_appendSeparator (x); }
    else {
        clock_delay (x->um_clock, UNDOMANAGER_DELAY);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction *undomanager_getUndoAction (t_undomanager *x)
{
    t_undoaction *a = x->um_tail;
    
    while (a && a->ua_previous) {
    //
    if (undoaction_getType (a->ua_previous) == UNDO_SEPARATOR) { return a; }
    
    a = a->ua_previous;
    //
    }
    
    return NULL;
}

t_undoaction *undomanager_getRedoAction (t_undomanager *x)
{
    t_undoaction *a = x->um_tail;
    
    while (a) {
    //
    if (undoaction_getType (a) != UNDO_SEPARATOR) { return a; }
    
    a = a->ua_next;
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_symbol *undomanager_getUndoLabel (t_undomanager *x)
{
    t_undoaction *a = undomanager_getUndoAction (x);
    
    if (a) { return undoaction_getLabel (a); }
    
    return symbol_nil();
}

t_symbol *undomanager_getRedoLabel (t_undomanager *x)
{
    t_undoaction *a = undomanager_getRedoAction (x);
    
    if (a) { return undoaction_getLabel (a); }
    
    return symbol_nil();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int undomanager_hasSeparatorAtLast (t_undomanager *x)
{
    return (x->um_tail && (undoaction_getType (x->um_tail) == UNDO_SEPARATOR));
}

void undomanager_appendSeparatorLater (t_undomanager *x)
{
    clock_delay (x->um_clock, UNDOMANAGER_DELAY);
}

void undomanager_appendSeparator (t_undomanager *x)
{
    if (!x->um_tail || (undoaction_getType (x->um_tail) != UNDO_SEPARATOR)) {
    //
    undomanager_append (x, undoseparator_new()); undomanager_collapse (x);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void undomanager_appendProceed (t_undomanager *x, t_undoaction *a)
{
    t_undoaction *t = x->um_tail;
    
    a->ua_next      = NULL;
    a->ua_previous  = NULL;
    
    if (t == NULL) { x->um_head = x->um_tail = a; }
    else {
    //
    undoaction_releaseAllFrom (t->ua_next, x);
        
    x->um_tail = a; t->ua_next = a; a->ua_previous = t;
    //
    }
    
    x->um_count++;
}

void undomanager_append (t_undomanager *x, t_undoaction *a)
{
    clock_unset (x->um_clock);
    
    if (!instance_undoIsRecursive()) { undomanager_appendProceed (x, a); }
    else {
        undoaction_releaseAllFrom (a, NULL);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that DSP is suspended globally only to avoid unnecessary consecutive rebuilds. */

void undomanager_undo (t_undomanager *x)
{
    clock_unset (x->um_clock);
    
    if (!instance_undoIsRecursive()) {
    //
    int dspState, dspSuspended = undomanager_undoNeedToSuspend (x);
    
    if (dspSuspended) { dspState = dsp_suspend(); }
    
    instance_undoSetRecursive();

    {
    //
    instance_pendingBegin();
    
    t_undoaction *a = x->um_tail;
    
    while (a && a->ua_previous) {
    
        if (class_hasMethod (pd_class (a), sym_undo)) { pd_message (cast_pd (a), sym_undo, 0, NULL); }
        
        a = a->ua_previous;
        
        if (undoaction_getType (a) == UNDO_SEPARATOR) { break; }
    }
    
    x->um_tail = a;
    
    instance_pendingEnd();
    //
    }
    
    instance_undoUnsetRecursive();
    
    if (dspSuspended) { dsp_resume (dspState); }
    //
    }
}

void undomanager_redo (t_undomanager *x)
{
    clock_unset (x->um_clock);
    
    if (!instance_undoIsRecursive()) {
    //
    int dspState, dspSuspended = undomanager_redoNeedToSuspend (x);
    
    if (dspSuspended) { dspState = dsp_suspend(); }
    
    instance_undoSetRecursive();

    {
    //
    instance_pendingBegin();
    
    t_undoaction *a = x->um_tail;
    
    while (a) {
    
        if (class_hasMethod (pd_class (a), sym_redo)) { pd_message (cast_pd (a), sym_redo, 0, NULL); }
        
        if (a->ua_next == NULL) { break; } else { a = a->ua_next; }
        
        if (undoaction_getType (a) == UNDO_SEPARATOR) { break; }
    }
    
    x->um_tail = a;
    
    instance_pendingEnd();
    //
    }
    
    instance_undoUnsetRecursive();
    
    if (dspSuspended) { dsp_resume (dspState); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undomanager *undomanager_new (t_glist *owner)
{
    t_undomanager *x = (t_undomanager *)PD_MEMORY_GET (sizeof (t_undomanager));
    
    x->um_owner = owner;
    x->um_clock = clock_new ((void *)x, (t_method)undomanager_task);

    undomanager_appendProceed (x, undoseparator_new());
    
    return x;
}

void undomanager_free (t_undomanager *x)
{
    if (x) {
    //
    clock_unset (x->um_clock);
    undoaction_releaseAllFrom (x->um_head, x);
    PD_ASSERT (x->um_count == 0);
    clock_free (x->um_clock);
    PD_MEMORY_FREE (x);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
