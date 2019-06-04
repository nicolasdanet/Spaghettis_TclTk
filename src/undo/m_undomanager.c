
/* Copyright (c) 1997-2019 Miller Puckette and others. */

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

int  undodisconnect_match (t_undoaction *, t_id, t_items *, t_items *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void undomanager_task (t_undomanager *x)
{
    undomanager_appendSeparator (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_undoaction *undomanager_getUndoAction (t_undomanager *x)
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

static t_undoaction *undomanager_getRedoAction (t_undomanager *x)
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

static int undomanager_undoContainsDelete (t_undomanager *x, t_items *i, t_items *o)
{
    int k = 0;
    
    t_undoaction *a = x->um_tail;

    while (a && a->ua_previous) {
    
        if (undoaction_getType (a) == UNDO_DELETE) { k |= undoaction_getInletsAndOutlets (a, i, o); }
        
        a = a->ua_previous;
        
        if (undoaction_getType (a) == UNDO_SEPARATOR) { break; }
    }
    
    return k;
}

int undomanager_undoNeedToTriggerParent (t_undomanager *x, t_items *i, t_items *o)
{
    t_undoaction *a = undomanager_getUndoAction (x);
    
    if (a && undoaction_getType (a) == UNDO_REMOVE) { return undomanager_undoContainsDelete (x, i, o); }
    
    return 0;
}

static int undomanager_redoContainsCreate (t_undomanager *x, t_items *i, t_items *o)
{
    int k = 0;
    
    t_undoaction *a = x->um_tail;
    
    while (a) {
    
        if (undoaction_getType (a) == UNDO_CREATE) { k |= undoaction_getInletsAndOutlets (a, i, o); }
        
        if (a->ua_next == NULL) { break; } else { a = a->ua_next; }
        
        if (undoaction_getType (a) == UNDO_SEPARATOR) { break; }
    }
    
    return k;
}

int undomanager_redoNeedToTriggerParent (t_undomanager *x, t_items *i, t_items *o)
{
    t_undoaction *a = undomanager_getRedoAction (x);
    
    if (a && undoaction_getType (a) == UNDO_ADD) { return undomanager_redoContainsCreate (x, i, o); }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int undomanager_undoContainsDisconnect (t_undomanager *x, t_glist *glist, t_items *i, t_items *o)
{
    t_id u = gobj_getUnique (cast_gobj (glist));
    
    t_undoaction *a = x->um_tail;

    while (a && a->ua_previous) {
    
        if (undodisconnect_match (a, u, i, o)) { } else if (undoaction_getType (a) != UNDO_SEPARATOR) {
            return 0;
        }
        
        a = a->ua_previous;
        
        if (undoaction_getType (a) == UNDO_SEPARATOR) { break; }
    }
    
    return 1;
}

int undomanager_triggerParentIsPossible (t_glist *glist, t_items *i, t_items *o)
{
    if (glist_hasParent (glist)) {
    //
    t_glist *parent = glist_getParent (glist);
    
    if (undomanager_undoContainsDisconnect (glist_getUndoManager (parent), glist, i, o)) {
        return 1;
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undomanager_undo (t_undomanager *x)
{
    clock_unset (x->um_clock);
    
    if (!instance_undoIsRecursive()) {
    //
    int state = dsp_suspend();
    
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
    
    dsp_resume (state);
    //
    }
}

void undomanager_redo (t_undomanager *x)
{
    clock_unset (x->um_clock);
    
    if (!instance_undoIsRecursive()) {
    //
    int state = dsp_suspend();
    
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
    
    dsp_resume (state);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undomanager *undomanager_new (void)
{
    t_undomanager *x = (t_undomanager *)PD_MEMORY_GET (sizeof (t_undomanager));
    
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
