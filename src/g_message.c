
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _messageresponder {
    t_pd                mr_pd;                          /* MUST be the first. */
    t_outlet            *mr_outlet;
    } t_messageresponder;

struct _message {
    t_object            m_obj;                          /* MUST be the first. */
    t_messageresponder  m_responder;
    t_glist             *m_owner;
    t_clock             *m_clock;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *message_class;                          /* Shared. */
static t_class *messageresponder_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void messageresponder_bang (t_messageresponder *x)
{
    outlet_bang (x->mr_outlet);
}

static void messageresponder_float (t_messageresponder *x, t_float f)
{
    outlet_float (x->mr_outlet, f);
}

static void messageresponder_symbol (t_messageresponder *x, t_symbol *s)
{
    outlet_symbol (x->mr_outlet, s);
}

static void messageresponder_list (t_messageresponder *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list (x->mr_outlet, argc, argv);
}

static void messageresponder_anything (t_messageresponder *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (x->mr_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void message_bang (t_message *x)
{
    buffer_eval (object_getBuffer (cast_object (x)), cast_pd (&x->m_responder), 0, NULL);
}

static void message_float (t_message *x, t_float f)
{
    t_atom a; SET_FLOAT (&a, f);
    
    buffer_eval (object_getBuffer (cast_object (x)), cast_pd (&x->m_responder), 1, &a);
}

static void message_symbol (t_message *x, t_symbol *s)
{
    t_atom a; SET_SYMBOL (&a, s);
    
    buffer_eval (object_getBuffer (cast_object (x)), cast_pd (&x->m_responder), 1, &a);
}

static void message_list (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_eval (object_getBuffer (cast_object (x)), cast_pd (&x->m_responder), argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void message_click (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    if (glist_hasWindow (x->m_owner)) {                     /* Not shown in GOP. */
    //
    if (glist_isOnScreen (x->m_owner)) {
    //
    t_box *text = box_fetch (x->m_owner, cast_object (x));
              
    sys_vGui ("%s.c itemconfigure %sBORDER -width 5\n", 
                    glist_getTagAsString (x->m_owner), 
                    box_getTag (text));
    
    sys_guiFlush(); clock_delay (x->m_clock, 120.0);        /* Force the GUI to update. */
    //
    }
    //
    }
    
    message_float (x, (t_float)0.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void message_set (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_reset (object_getBuffer (cast_object (x)));
    buffer_append (object_getBuffer (cast_object (x)), argc, argv);
    box_retext (box_fetch (x->m_owner, cast_object (x)));
}

static void message_add (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append (object_getBuffer (cast_object (x)), argc, argv);
    buffer_appendSemicolon (object_getBuffer (cast_object (x)));
    box_retext (box_fetch (x->m_owner, &x->m_obj));
}

static void message_append (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append (object_getBuffer (cast_object (x)), argc, argv);
    box_retext (box_fetch (x->m_owner, cast_object (x)));
}

static void message_addComma (t_message *x)
{
    t_atom a; SET_COMMA (&a);
    
    buffer_appendAtom (object_getBuffer (cast_object (x)), &a);
    box_retext (box_fetch (x->m_owner, cast_object (x)));
}

static void message_addSemicolon (t_message *x)
{
    message_add (x, NULL, 0, NULL);
}

static void message_addDollar (t_message *x, t_float f)
{
    int n = PD_MAX (0, (int)f);
    t_atom a; SET_DOLLAR (&a, n);
    
    buffer_appendAtom (object_getBuffer (cast_object (x)), &a);
    box_retext (box_fetch (x->m_owner, cast_object (x)));
}

static void message_addDollarSymbol (t_message *x, t_symbol *s)
{
    t_atom a;
    
    char t[PD_STRING] = { 0 };
    string_sprintf (t, PD_STRING, "$%s", s->s_name);
    SET_DOLLARSYMBOL (&a, gensym (t));

    buffer_appendAtom (object_getBuffer (cast_object (x)), &a);
    box_retext (box_fetch (x->m_owner, cast_object (x)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void message_taskTick (t_message *x)
{
    if (glist_hasWindow (x->m_owner)) {                     /* Not shown in GOP. */
    //
    if (glist_isOnScreen (x->m_owner)) {
    //
    t_box *text = box_fetch (x->m_owner, cast_object (x));
    
    sys_vGui ("%s.c itemconfigure %sBORDER -width 1\n",
                    glist_getTagAsString (x->m_owner),
                    box_getTag (text));
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void message_makeObjectFile (t_message *x, int argc, t_atom *argv)
{
    t_buffer *t = buffer_new(); if (argc > 2) { buffer_deserialize (t, argc - 2, argv + 2); }
    
    object_setBuffer (cast_object (x), t);
    object_setX (cast_object (x), atom_getFloatAtIndex (0, argc, argv));
    object_setY (cast_object (x), atom_getFloatAtIndex (1, argc, argv));
    object_setWidth (cast_object (x), 0);
    object_setType (cast_object (x), TYPE_MESSAGE);
    
    glist_objectAdd (x->m_owner, cast_gobj (x));
}

static void message_makeObjectMenu (t_message *x, int argc, t_atom *argv)
{
    if (glist_isOnScreen (x->m_owner)) {
    //
    glist_deselectAll (x->m_owner);

    object_setBuffer (cast_object (x), buffer_new());
    object_setX (cast_object (x), instance_getDefaultX (x->m_owner));
    object_setY (cast_object (x), instance_getDefaultY (x->m_owner));
    object_setWidth (cast_object (x), 0);
    object_setType (cast_object (x), TYPE_MESSAGE);
    
    glist_objectAdd (x->m_owner, cast_gobj (x));
    glist_objectSelect (x->m_owner, cast_gobj (x));
    gobj_activated (cast_gobj (x), x->m_owner, 1);
    //
    }
}

void message_makeObject (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_message *x = (t_message *)pd_new (message_class);
    
    x->m_responder.mr_pd     = messageresponder_class;
    x->m_responder.mr_outlet = outlet_new (cast_object (x), &s_anything);
    x->m_owner               = glist;
    x->m_clock               = clock_new ((void *)x, (t_method)message_taskTick);
    
    if (argc > 1) { message_makeObjectFile (x, argc, argv); }
    else {
        message_makeObjectMenu (x, argc, argv);
    }
}

static void message_free (t_message *x)
{
    clock_free (x->m_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void message_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_message,
            NULL,
            (t_method)message_free,
            sizeof (t_message),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addBang (c, (t_method)message_bang);
    class_addFloat (c, (t_method)message_float);
    class_addSymbol (c, (t_method)message_symbol);
    class_addList (c, (t_method)message_list);
    class_addAnything (c, (t_method)message_list);

    class_addClick (c, (t_method)message_click);
        
    class_addMethod (c, (t_method)message_set,              sym_set,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_add,              sym_add,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_append,           sym_append,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_addComma,         sym_addcomma,           A_NULL);
    class_addMethod (c, (t_method)message_addSemicolon,     sym_addsemicolon,       A_NULL);
    class_addMethod (c, (t_method)message_addDollar,        sym_adddollar,          A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)message_addDollarSymbol,  sym_adddollarsymbol,    A_SYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)message_append,           sym_add2,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)message_addSemicolon,     sym_addsemi,            A_NULL);
    class_addMethod (c, (t_method)message_addDollarSymbol,  sym_adddollsym,         A_SYMBOL, A_NULL);
        
    #endif
    
    message_class = c;
    
    c = class_new (sym_messageresponder,
            NULL,
            NULL,
            0,
            CLASS_ABSTRACT,
            A_NULL);
            
    class_addBang (c, (t_method)messageresponder_bang);
    class_addFloat (c, (t_method)messageresponder_float);
    class_addSymbol (c, (t_method)messageresponder_symbol);
    class_addList (c, (t_method)messageresponder_list);
    class_addAnything (c, (t_method)messageresponder_anything);
    
    messageresponder_class = c;
}

void message_destroy (void)
{
    CLASS_FREE (message_class);
    CLASS_FREE (messageresponder_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
