
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
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
    buffer_eval (cast_object (x)->te_buffer, cast_pd (&x->m_responder), 0, NULL);
}

static void message_float (t_message *x, t_float f)
{
    t_atom a; SET_FLOAT (&a, f);
    
    buffer_eval (cast_object (x)->te_buffer, cast_pd (&x->m_responder), 1, &a);
}

static void message_symbol (t_message *x, t_symbol *s)
{
    t_atom a; SET_SYMBOL (&a, s);
    
    buffer_eval (cast_object (x)->te_buffer, cast_pd (&x->m_responder), 1, &a);
}

static void message_list (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_eval (cast_object (x)->te_buffer, cast_pd (&x->m_responder), argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void message_click (t_message *x, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    message_float (x, 0);
    
    if (canvas_isMapped (x->m_owner)) {
    //
    t_boxtext *text = boxtext_fetch (x->m_owner, cast_object (x));

    sys_vGui (".x%lx.c itemconfigure %sBORDER -width 5\n", 
                    canvas_getView (x->m_owner), 
                    boxtext_getTag (text));
                    
    clock_delay (x->m_clock, 120.0);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void message_set (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_reset (cast_object (x)->te_buffer);
    buffer_append (cast_object (x)->te_buffer, argc, argv);
    
    boxtext_retext (x->m_owner, cast_object (x));
}

static void message_add (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append (cast_object (x)->te_buffer, argc, argv);
    buffer_appendSemicolon (cast_object (x)->te_buffer);
    
    boxtext_retext (x->m_owner, &x->m_obj);
}

static void message_append (t_message *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append (cast_object (x)->te_buffer, argc, argv);
    
    boxtext_retext (x->m_owner, cast_object (x));
}

static void message_addComma (t_message *x)
{
    t_atom a; SET_COMMA (&a);
    
    buffer_appendAtom (cast_object (x)->te_buffer, &a);
    
    boxtext_retext (x->m_owner, cast_object (x));
}

static void message_addSemicolon (t_message *x)
{
    message_add (x, NULL, 0, NULL);
}

static void message_addDollar (t_message *x, t_float f)
{
    int n = PD_MAX (0, (int)f);
    t_atom a; SET_DOLLAR (&a, n);
    
    buffer_appendAtom (cast_object (x)->te_buffer, &a);
    
    boxtext_retext (x->m_owner, cast_object (x));
}

static void message_addDollarSymbol (t_message *x, t_symbol *s)
{
    t_atom a;
    
    char t[PD_STRING] = { 0 };
    string_sprintf (t, PD_STRING, "$%s", s->s_name);
    SET_DOLLARSYMBOL (&a, gensym (t));

    buffer_appendAtom (cast_object (x)->te_buffer, &a);
    
    boxtext_retext (x->m_owner, cast_object (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void message_taskTick (t_message *x)
{
    if (canvas_isMapped (x->m_owner)) {
    //
    t_boxtext *text = boxtext_fetch (x->m_owner, cast_object (x));
    
    sys_vGui (".x%lx.c itemconfigure %sBORDER -width 1\n",
                    canvas_getView (x->m_owner),
                    boxtext_getTag (text));
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void message_makeObject (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_message *x = (t_message *)pd_new (message_class);
    
    cast_object (x)->te_buffer  = buffer_new();
    cast_object (x)->te_width   = 0;
    cast_object (x)->te_type    = TYPE_MESSAGE;
    x->m_responder.mr_pd        = messageresponder_class;
    x->m_responder.mr_outlet    = outlet_new (cast_object (x), &s_anything);
    x->m_owner                  = glist;
    x->m_clock                  = clock_new ((void *)x, (t_method)message_taskTick);
    
    if (argc > 1) {                                                             /* File creation. */
    
        cast_object (x)->te_xCoordinate = atom_getFloatAtIndex (0, argc, argv);
        cast_object (x)->te_yCoordinate = atom_getFloatAtIndex (1, argc, argv);
        
        if (argc > 2) {
            buffer_deserialize (cast_object (x)->te_buffer, argc - 2, argv + 2);
        }
        
        canvas_addObject (glist, cast_gobj (x));
        
    } else if (canvas_isMapped (glist)) {                                       /* Interactive creation. */

        int positionX = 0;
        int positionY = 0;
        
        canvas_getLastMotionCoordinates (glist, &positionX, &positionY);
        canvas_deselectAll (glist);
    
        cast_object (x)->te_xCoordinate = positionX;
        cast_object (x)->te_yCoordinate = positionY;
        
        canvas_addObject (glist, cast_gobj (x));
        canvas_selectObject (glist, cast_gobj (x));
        gobj_activated (cast_gobj (x), glist, 1);
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
    
    class_addBang (c, message_bang);
    class_addFloat (c, message_float);
    class_addSymbol (c, message_symbol);
    class_addList (c, message_list);
    class_addAnything (c, message_list);

    class_addClick (c, message_click);
        
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
            
    class_addBang (c, messageresponder_bang);
    class_addFloat (c, messageresponder_float);
    class_addSymbol (c, messageresponder_symbol);
    class_addList (c, messageresponder_list);
    class_addAnything (c, messageresponder_anything);
    
    messageresponder_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
