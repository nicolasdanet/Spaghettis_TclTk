
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     *pd_newest;
extern t_pd     pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class         *textdefine_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textdefine {
    t_textbuffer    x_textbuffer;                   /* Must be the first. */
    t_gpointer      x_gpointer;
    int             x_keep;
    t_symbol        *x_name;
    t_scalar        *x_scalar;
    t_outlet        *x_out;
    } t_textdefine;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *text_define_new(t_symbol *s, int argc, t_atom *argv)
{
    t_textdefine *x = (t_textdefine *)pd_new(textdefine_class);
    t_symbol *asym = sym___hash__A;
    x->x_keep = 0;
    x->x_name = &s_;
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-k"))
            x->x_keep = 1;
        else
        {
            post_error ("text define: unknown flag ...");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        pd_bind(cast_pd (x), argv->a_w.w_symbol);
        x->x_name = argv->a_w.w_symbol;
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: text define ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    textbuffer_init (&x->x_textbuffer);
        /* set up a scalar and a pointer to it that we can output */
    x->x_scalar = scalar_new(canvas_getCurrent(), sym___TEMPLATE__text);
    buffer_free(x->x_scalar->sc_vector[2].w_buffer);                                /* Encaspulate ASAP. */
    x->x_scalar->sc_vector[2].w_buffer = textbuffer_getBuffer (&x->x_textbuffer);   /* Encaspulate ASAP. */
    x->x_out = outlet_new(cast_object (x), &s_pointer);
    gpointer_init(&x->x_gpointer);
           /* bashily unbind #A -- this would create garbage if #A were
           multiply bound but we believe in this context it's at most
           bound to whichever text_define or array was created most recently */
    asym->s_thing = 0;
        /* and now bind #A to us to receive following messages in the
        saved file or copy buffer */
    pd_bind(cast_pd (x), asym); 
    return (x);
}

static void text_define_clear(t_textdefine *x)
{
    buffer_reset(textbuffer_getBuffer (&x->x_textbuffer));
    textbuffer_update(&x->x_textbuffer);
}

/*********  random utility function to find a binbuf in a datum */

t_buffer *pointertobinbuf(t_pd *x, t_gpointer *gp, t_symbol *s,
    const char *fname)
{
    t_symbol *templatesym = gpointer_getTemplateIdentifier(gp), *arraytype;
    t_template *template;
    int onset, type;
    t_buffer *b;
    t_word *vec;
    if (!templatesym)
    {
        post_error ("%s: bad pointer", fname);
        return (0);
    }
    if (!(template = template_findByIdentifier(templatesym)))
    {
        post_error ("%s: couldn't find template %s", fname,
            templatesym->s_name);
        return (0);
    }
    /* Remove template_findField ASAP !!! */
    if (!template_findField(template, s, &onset, &type, &arraytype))    
    {
        post_error ("%s: %s.%s: no such field", fname,
            templatesym->s_name, s->s_name);
        return (0);
    }
    if (type != DATA_TEXT)
    {
        post_error ("%s: %s.%s: not a list", fname,
            templatesym->s_name, s->s_name);
        return (0);
    }
    vec = gpointer_getData (gp);
    return (vec[onset].w_buffer);
}


    /* these are unused; they copy text from this object to and from a text
        field in a scalar. */
static void text_define_frompointer(t_textdefine *x, t_gpointer *gp,
    t_symbol *s)
{
    t_buffer *b = pointertobinbuf(cast_pd (x),
        gp, s, "text_frompointer");
    if (b)
    {
        buffer_reset(textbuffer_getBuffer (&x->x_textbuffer));
        buffer_appendBuffer(textbuffer_getBuffer (&x->x_textbuffer), b);
    } 
}

static void text_define_topointer(t_textdefine *x, t_gpointer *gp, t_symbol *s)
{
    t_buffer *b = pointertobinbuf(cast_pd (x), gp, s, "text_topointer");
    if (b)
    {
        buffer_reset(b);
        buffer_appendBuffer (b, textbuffer_getBuffer (&x->x_textbuffer));
        gpointer_redraw (gp);
    } 
}

    /* bang: output a pointer to a struct containing this text */
void text_define_bang(t_textdefine *x)
{
    gpointer_setAsScalar(&x->x_gpointer, textbuffer_getView (&x->x_textbuffer), x->x_scalar);
    outlet_pointer(x->x_out, &x->x_gpointer);
}

    /* set from a list */
void text_define_set(t_textdefine *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_deserialize(textbuffer_getBuffer (&x->x_textbuffer), argc, argv);
    textbuffer_update(&x->x_textbuffer);
}


static void text_define_save(t_gobj *z, t_buffer *bb)
{
    t_textdefine *x = (t_textdefine *)z;
    buffer_vAppend(bb, "ssff", sym___hash__X, sym_obj,
        (float)cast_object (x)->te_xCoordinate, (float)cast_object (x)->te_yCoordinate);
    buffer_serialize(bb, cast_object (x)->te_buffer);
    buffer_appendSemicolon(bb);
    if (x->x_keep)
    {
        buffer_vAppend(bb, "ss", sym___hash__A, sym_set);
        buffer_serialize(bb, textbuffer_getBuffer (&x->x_textbuffer));
        buffer_appendSemicolon(bb);
    }
    object_saveWidth(cast_object (x), bb);
}

static void text_define_free(t_textdefine *x)
{
    textbuffer_free (&x->x_textbuffer);
    if (x->x_name != &s_)
        pd_unbind(cast_pd (x), x->x_name);
    gpointer_unset(&x->x_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* --- overall creator for "text" objects: dispatch to "text define" etc --- */
static void *text_new(t_symbol *s, int argc, t_atom *argv)
{
    if (!argc || argv[0].a_type != A_SYMBOL)
        pd_newest = text_define_new(s, argc, argv);
    else
    {
        char *str = argv[0].a_w.w_symbol->s_name;
        if (!strcmp(str, "d") || !strcmp(str, "define"))
            pd_newest = text_define_new(s, argc-1, argv+1);
        else if (!strcmp(str, "get"))
            pd_newest = textget_new(s, argc-1, argv+1);
        else if (!strcmp(str, "set"))
            pd_newest = textset_new(s, argc-1, argv+1);
        else if (!strcmp(str, "size"))
            pd_newest = textsize_new(s, argc-1, argv+1);
        else if (!strcmp(str, "tolist"))
            pd_newest = texttolist_new(s, argc-1, argv+1);
        else if (!strcmp(str, "fromlist"))
            pd_newest = textfromlist_new(s, argc-1, argv+1);
        else if (!strcmp(str, "search"))
            pd_newest = textsearch_new(s, argc-1, argv+1);
        else if (!strcmp(str, "sequence"))
            pd_newest = textsequence_new(s, argc-1, argv+1);
        else 
        {
            post_error ("list %s: unknown function", str);
            pd_newest = 0;
        }
    }
    return (pd_newest);
}

static t_pd *text_templatecanvas;
static char text_templatefile[] = "\
canvas 0 0 458 153 10;\n\
#X obj 43 31 struct text float x float y text t;\n\
";

/* create invisible, built-in canvas to supply template containing one text
field named 't'.  I don't know how to make this not break
pre-0.45 patches using templates named 'text'... perhaps this is a minor
enough incompatibility that I'll just get away with it. */

static void text_template_init( void)
{
    t_buffer *b;
    if (text_templatecanvas)
        return;
    b = buffer_new();
    
    canvas_setActiveFileNameAndDirectory (sym__texttemplate, sym___dot__);
    buffer_withStringUnzeroed(b, text_templatefile, strlen (text_templatefile));
    buffer_eval(b, &pd_canvasMaker, 0, 0);
    pd_vMessage(s__X.s_thing, sym__pop, "i", 0);
    
    canvas_setActiveFileNameAndDirectory (&s_, &s_);
    buffer_free(b);  
}

void textdefine_setup (void)
{
    text_template_init();
    textdefine_class = class_new(sym_text__space__define,
        (t_newmethod)text_define_new,
        (t_method)text_define_free, sizeof(t_textdefine), 0, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)textbuffer_open,
        sym_click, 0);
    class_addMethod(textdefine_class, (t_method)textbuffer_close,
        sym_close, 0);
    class_addMethod(textdefine_class, (t_method)textbuffer_add, 
        sym__addline, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)text_define_set,
        sym_set, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)text_define_clear,
        sym_clear, 0);
    class_addMethod(textdefine_class, (t_method)textbuffer_write,
        sym_write, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)textbuffer_read,
        sym_read, A_GIMME, 0);
    class_setSaveFunction(textdefine_class, text_define_save);
    class_addBang(textdefine_class, text_define_bang);
    class_setHelpName(textdefine_class, sym_text);

    class_addCreator((t_newmethod)text_new, sym_text, A_GIMME, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
