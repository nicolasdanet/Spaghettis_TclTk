
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_undosnippet *undosnippet_newProceed (t_gobj *gobj, t_glist *owner)
{
    t_undosnippet *x = (t_undosnippet *)PD_MEMORY_GET (sizeof (t_undosnippet));
    
    x->us_object = gobj_getUnique (gobj);
    x->us_owner  = owner ? gobj_getUnique (cast_gobj (owner)) : 0;
    x->us_z      = owner ? glist_objectMoveGetPosition (owner, gobj) : 0;
    x->us_buffer = buffer_new();
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undosnippet *undosnippet_newCopy (t_gobj *gobj, t_glist *owner)
{
    t_undosnippet *x = undosnippet_newProceed (gobj, owner);

    if (gobj_isScalar (gobj)) { PD_BUG; }
    else {
        buffer_appendBuffer (x->us_buffer, object_getBuffer (cast_object (gobj)));
    }
    
    return x;
}

t_undosnippet *undosnippet_newSave (t_gobj *gobj, t_glist *owner)
{
    t_undosnippet *x = undosnippet_newProceed (gobj, owner);
    
    gobj_save (gobj, x->us_buffer, SAVE_UNDO);
    
    return x;
}

t_undosnippet *undosnippet_newProperties (t_gobj *gobj, t_glist *owner)
{
    t_undosnippet *x = undosnippet_newProceed (gobj, owner);
    
    if (class_hasUndoFunction (pd_class (gobj))) {
    //
    (*class_getUndoFunction (pd_class (gobj))) (gobj, x->us_buffer);
    //
    }
    
    return x;
}

t_undosnippet *undosnippet_new (t_gobj *gobj, t_glist *owner)
{
    return undosnippet_newProceed (gobj, owner);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undosnippet_free (t_undosnippet *x)
{
    buffer_free (x->us_buffer);
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* MUST have been created with undosnippet_newCopy. */

void undosnippet_paste (t_undosnippet *x)
{
    t_gobj *t = instance_registerGetObject (x->us_owner);
    t_gobj *y = instance_registerGetObject (x->us_object);
    
    if (t && y && gobj_isCanvas (t) && !gobj_isScalar (y) && buffer_getSize (x->us_buffer)) {
    //
    t_object *o = cast_object (y);
    
    if (object_isMessage (o) || object_isComment (o)) {
    //
    t_buffer *b = buffer_newCopy (x->us_buffer);
    
    object_setBuffer (o, b);    /* Takes ownership of the buffer. */
    
    box_retext (box_fetch (cast_glist (t), o)); glist_updateLinesForObject (cast_glist (t), o);
    
    if (object_isMessage (o)) { message_dirty ((t_message *)o); }
    //
    }
    //
    }
}

/* MUST have been created with undosnippet_newSave. */

void undosnippet_load (t_undosnippet *x)
{
    if (instance_registerContains (x->us_object) == 0) {
    //
    t_gobj *t = instance_registerGetObject (x->us_owner);
    
    if (t && gobj_isCanvas (t)) {
    //
    if (buffer_getSize (x->us_buffer)) { instance_loadSnippet (cast_glist (t), x->us_buffer); }
    //
    }
    //
    }
}

void undosnippet_update (t_undosnippet *x)
{
    t_gobj *t = instance_registerGetObject (x->us_object);
    
    if (t) { buffer_clear (x->us_buffer); gobj_save (t, x->us_buffer, SAVE_UNDO | SAVE_UPDATE); }
}

/* MUST have been created with undosnippet_newProperties. */

void undosnippet_message (t_undosnippet *x)
{
    int n = buffer_getSize (x->us_buffer);
    
    if (n) {
    //
    t_symbol *s = atom_getSymbol (buffer_getAtomAtIndex (x->us_buffer, 0));
    
    pd_messageByUnique (x->us_object, s, n - 1, (n > 1) ? (buffer_getAtoms (x->us_buffer) + 1) : NULL);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undosnippet_z (t_undosnippet *x)
{
    if (instance_registerContains (x->us_object) == 1) {
    //
    t_gobj *t = instance_registerGetObject (x->us_owner);
    
    if (t && gobj_isCanvas (t)) {
    //
    glist_objectMoveAtByUnique (x->us_object, x->us_z);
    //
    }
    //
    }
}

void undosnippet_front (t_undosnippet *x)
{
    if (instance_registerContains (x->us_object) == 1) {
    //
    t_gobj *t = instance_registerGetObject (x->us_owner);
    
    if (t && gobj_isCanvas (t)) {
    //
    glist_objectMoveAtLastByUnique (x->us_object);
    //
    }
    //
    }
}

void undosnippet_back (t_undosnippet *x)
{
    if (instance_registerContains (x->us_object) == 1) {
    //
    t_gobj *t = instance_registerGetObject (x->us_owner);
    
    if (t && gobj_isCanvas (t)) {
    //
    glist_objectMoveAtFirstByUnique (x->us_object);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
