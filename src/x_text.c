
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
#include "m_alloca.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     *pd_newest;
extern t_pd     pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class         *textdefine_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

    /* random helper function */
int text_nthline(int n, t_atom *vec, int line, int *startp, int *endp)
{
    int i, cnt = 0;
    for (i = 0; i < n; i++)
    {
        if (cnt == line)
        {
            int j = i, outc, k;
            while (j < n && vec[j].a_type != A_SEMICOLON &&
                vec[j].a_type != A_COMMA)
                    j++;
            *startp = i;
            *endp = j;
            return (1);
        }
        else if (vec[i].a_type == A_SEMICOLON || vec[i].a_type == A_COMMA)
            cnt++;
    }
    return (0);
}

/* text_define object - text buffer, accessible by other accessor objects */

typedef struct _text_define
{
    t_textbuf x_textbuf;
    t_outlet *x_out;
    t_symbol *x_bindsym;
    t_scalar *x_scalar;     /* faux scalar (struct text-scalar) to point to */
    t_gpointer x_gp;        /* pointer to it */
    t_glist *x_canvas;     /* owning canvas whose stub we use for x_gp */
    unsigned char x_keep;   /* whether to embed contents in patch on save */
} t_text_define;

#define x_ob x_textbuf.b_ob
#define x_binbuf x_textbuf.b_binbuf
#define x_canvas x_textbuf.b_canvas

static void *text_define_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_define *x = (t_text_define *)pd_new(textdefine_class);
    t_symbol *asym = sym___hash__A;
    x->x_keep = 0;
    x->x_bindsym = &s_;
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
        pd_bind(&x->x_ob.te_g.g_pd, argv->a_w.w_symbol);
        x->x_bindsym = argv->a_w.w_symbol;
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: text define ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    textbuf_init(&x->x_textbuf);
        /* set up a scalar and a pointer to it that we can output */
    x->x_scalar = scalar_new(canvas_getCurrent(), sym___TEMPLATE__text);
    buffer_free(x->x_scalar->sc_vector[2].w_buffer);                        /* Encaspulate ASAP. */
    x->x_scalar->sc_vector[2].w_buffer = x->x_binbuf;                       /* Encaspulate ASAP. */
    x->x_out = outlet_new(&x->x_ob, &s_pointer);
    gpointer_init(&x->x_gp);
    x->x_canvas = canvas_getCurrent();
           /* bashily unbind #A -- this would create garbage if #A were
           multiply bound but we believe in this context it's at most
           bound to whichever text_define or array was created most recently */
    asym->s_thing = 0;
        /* and now bind #A to us to receive following messages in the
        saved file or copy buffer */
    pd_bind(&x->x_ob.te_g.g_pd, asym); 
    return (x);
}

static void text_define_clear(t_text_define *x)
{
    buffer_reset(x->x_binbuf);
    textbuf_senditup(&x->x_textbuf);
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
static void text_define_frompointer(t_text_define *x, t_gpointer *gp,
    t_symbol *s)
{
    t_buffer *b = pointertobinbuf(&x->x_textbuf.b_ob.te_g.g_pd,
        gp, s, "text_frompointer");
    if (b)
    {
        buffer_reset(x->x_textbuf.b_binbuf);
        buffer_append(x->x_textbuf.b_binbuf, buffer_size(b), buffer_atoms(b));
    } 
}

static void text_define_topointer(t_text_define *x, t_gpointer *gp, t_symbol *s)
{
    t_buffer *b = pointertobinbuf(&x->x_textbuf.b_ob.te_g.g_pd, gp, s, "text_topointer");
    if (b)
    {
        buffer_reset(b);
        buffer_append(b, buffer_size(x->x_textbuf.b_binbuf),
            buffer_atoms(x->x_textbuf.b_binbuf));
        gpointer_redraw (gp);
    } 
}

    /* bang: output a pointer to a struct containing this text */
void text_define_bang(t_text_define *x)
{
    gpointer_setAsScalar(&x->x_gp, x->x_canvas, x->x_scalar);
    outlet_pointer(x->x_out, &x->x_gp);
}

    /* set from a list */
void text_define_set(t_text_define *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_deserialize(x->x_binbuf, argc, argv);
    textbuf_senditup(&x->x_textbuf);
}


static void text_define_save(t_gobj *z, t_buffer *bb)
{
    t_text_define *x = (t_text_define *)z;
    buffer_vAppend(bb, "ssff", sym___hash__X, sym_obj,
        (float)x->x_ob.te_xCoordinate, (float)x->x_ob.te_yCoordinate);
    buffer_serialize(bb, x->x_ob.te_buffer);
    buffer_appendSemicolon(bb);
    if (x->x_keep)
    {
        buffer_vAppend(bb, "ss", sym___hash__A, sym_set);
        buffer_serialize(bb, x->x_binbuf);
        buffer_appendSemicolon(bb);
    }
    object_saveWidth(&x->x_ob, bb);
}

static void text_define_free(t_text_define *x)
{
    textbuf_free(&x->x_textbuf);
    if (x->x_bindsym != &s_)
        pd_unbind(&x->x_ob.te_g.g_pd, x->x_bindsym);
    gpointer_unset(&x->x_gp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ------- text_get object - output all or part of nth lines ----------- */
t_class *text_get_class;

typedef struct _text_get
{
    t_text_client x_tc;
    t_outlet *x_out1;       /* list */
    t_outlet *x_out2;       /* 1 if comma terminated, 0 if semi, 2 if none */
    t_float x_f1;           /* field number, -1 for whole line */
    t_float x_f2;           /* number of fields */
} t_text_get;

#define x_obj x_tc.tc_obj
#define x_sym x_tc.tc_sym
#define x_gp x_tc.tc_gp
#define x_struct x_tc.tc_struct
#define x_field x_tc.tc_field

static void *text_get_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_get *x = (t_text_get *)pd_new(text_get_class);
    x->x_out1 = outlet_new(&x->x_obj, &s_list);
    x->x_out2 = outlet_new(&x->x_obj, &s_float);
    inlet_newFloat(&x->x_obj, &x->x_f1);
    inlet_newFloat(&x->x_obj, &x->x_f2);
    x->x_f1 = -1;
    x->x_f2 = 1;
    text_client_argparse(&x->x_tc, &argc, &argv, "text get");
    if (argc)
    {
        if (argv->a_type == A_FLOAT)
            x->x_f1 = argv->a_w.w_float;
        else
        {
            post("text get: can't understand field number");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc)
    {
        if (argv->a_type == A_FLOAT)
            x->x_f2 = argv->a_w.w_float;
        else
        {
            post("text get: can't understand field count");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: text get ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    if (x->x_struct)
        inlet_newPointer(&x->x_obj, &x->x_gp);
    else inlet_newSymbol(&x->x_obj, &x->x_tc.tc_sym);
    return (x);
}

static void text_get_float(t_text_get *x, t_float f)
{
    t_buffer *b = text_client_getbuf(&x->x_tc);
    int start, end, n, startfield, nfield;
    t_atom *vec;
    if (!b)
       return;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    startfield = x->x_f1;
    nfield = x->x_f2;
    if (text_nthline(n, vec, f, &start, &end))
    {
        int outc = end - start, k;
        t_atom *outv;
        if (x->x_f1 < 0)    /* negative start field for whole line */
        {
                /* tell us what terminated the line (semi or comma) */
            outlet_float(x->x_out2, (end < n && vec[end].a_type == A_COMMA));
            ATOMS_ALLOCA(outv, outc);
            for (k = 0; k < outc; k++)
                outv[k] = vec[start+k];
            outlet_list(x->x_out1, 0, outc, outv);
            ATOMS_FREEA(outv, outc);
        }
        else if (startfield + nfield > outc)
            post_error ("text get: field request (%d %d) out of range",
                startfield, nfield); 
        else
        {
            ATOMS_ALLOCA(outv, nfield);
            for (k = 0; k < nfield; k++)
                outv[k] = vec[(start+startfield)+k];
            outlet_list(x->x_out1, 0, nfield, outv);
            ATOMS_FREEA(outv, nfield);
        }
    }
    else if (x->x_f1 < 0)   /* whole line but out of range: empty list and 2 */
    {
        outlet_float(x->x_out2, 2);         /* 2 for out of range */
        outlet_list(x->x_out1, 0, 0, 0);    /* ... and empty list */
    }
}


typedef struct _text_set
{
    t_text_client x_tc;
    t_float x_f1;           /* line number */
    t_float x_f2;           /* field number, -1 for whole line */
} t_text_set;

t_class *text_set_class;

static void *text_set_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_set *x = (t_text_set *)pd_new(text_set_class);
    inlet_newFloat(&x->x_obj, &x->x_f1);
    inlet_newFloat(&x->x_obj, &x->x_f2);
    x->x_f1 = 0;
    x->x_f2 = -1;
    text_client_argparse(&x->x_tc, &argc, &argv, "text get");
    if (argc)
    {
        if (argv->a_type == A_FLOAT)
            x->x_f1 = argv->a_w.w_float;
        else
        {
            post("text get: can't understand field number");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc)
    {
        if (argv->a_type == A_FLOAT)
            x->x_f2 = argv->a_w.w_float;
        else
        {
            post("text get: can't understand field count");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: text set ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    if (x->x_struct)
        inlet_newPointer(&x->x_obj, &x->x_gp);
    else inlet_newSymbol(&x->x_obj, &x->x_tc.tc_sym);
    return (x);
}

static void text_set_list(t_text_set *x,
    t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = text_client_getbuf(&x->x_tc);
    int start, end, n, lineno = x->x_f1, fieldno = x->x_f2, i;
    t_atom *vec;
    if (!b)
       return;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    if (lineno < 0)
    {
        post_error ("text set: line number (%d) < 0", lineno);
        return;
    }
    if (text_nthline(n, vec, lineno, &start, &end))
    {
        if (fieldno < 0)
        {
            if (end - start != argc)  /* grow or shrink */
            {
                int oldn = n;
                n = n + (argc - (end-start));
                if (n > oldn)
                    buffer_resize(b, n);
                vec = buffer_atoms(b);
                memmove(&vec[start + argc], &vec[end],
                    sizeof(*vec) * (oldn - end));
                if (n < oldn) {
                    buffer_resize(b, n);
                    vec = buffer_atoms(b);
                }
            }
        }
        else
        {
            if (fieldno >= end - start)
            {
                post_error ("text set: field number (%d) past end of line",
                    fieldno);
                return;
            }
            if (fieldno + argc > end - start)
                argc = (end - start) - fieldno;
            start += fieldno;
        }
    }
    else if (fieldno < 0)  /* if line number too high just append to end */
    {
        int addsemi = (n && vec[n-1].a_type != A_SEMICOLON &&
            vec[n-1].a_type != A_COMMA), newsize = n + addsemi + argc + 1;
        buffer_resize(b, newsize);
        vec = buffer_atoms(b);
        if (addsemi)
            SET_SEMICOLON(&vec[n]);
        SET_SEMICOLON(&vec[newsize-1]);
        start = n+addsemi;
    }
    else
    {
        post("text set: %d: line number out of range", lineno);
        return;
    }
    for (i = 0; i < argc; i++)
    {
        if (argv[i].a_type == A_POINTER)
            SET_SYMBOL(&vec[start+i], sym___parenthesis__pointer__parenthesis__);
        else vec[start+i] = argv[i];
    }
    text_client_senditup(&x->x_tc);
}

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
            pd_newest = text_get_new(s, argc-1, argv+1);
        else if (!strcmp(str, "set"))
            pd_newest = text_set_new(s, argc-1, argv+1);
        else if (!strcmp(str, "size"))
            pd_newest = text_size_new(s, argc-1, argv+1);
        else if (!strcmp(str, "tolist"))
            pd_newest = text_tolist_new(s, argc-1, argv+1);
        else if (!strcmp(str, "fromlist"))
            pd_newest = text_fromlist_new(s, argc-1, argv+1);
        else if (!strcmp(str, "search"))
            pd_newest = text_search_new(s, argc-1, argv+1);
        else if (!strcmp(str, "sequence"))
            pd_newest = text_sequence_new(s, argc-1, argv+1);
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
        (t_method)text_define_free, sizeof(t_text_define), 0, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)textbuf_open,
        sym_click, 0);
    class_addMethod(textdefine_class, (t_method)textbuf_close,
        sym_close, 0);
    class_addMethod(textdefine_class, (t_method)textbuf_addline, 
        sym_addline, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)text_define_set,
        sym_set, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)text_define_clear,
        sym_clear, 0);
    class_addMethod(textdefine_class, (t_method)textbuf_write,
        sym_write, A_GIMME, 0);
    class_addMethod(textdefine_class, (t_method)textbuf_read,
        sym_read, A_GIMME, 0);
    class_setSaveFunction(textdefine_class, text_define_save);
    class_addBang(textdefine_class, text_define_bang);
    class_setHelpName(textdefine_class, sym_text);

    class_addCreator((t_newmethod)text_new, sym_text, A_GIMME, 0);

    text_get_class = class_new(sym_text__space__get,
        (t_newmethod)text_get_new, (t_method)text_client_free,
            sizeof(t_text_get), 0, A_GIMME, 0);
    class_addFloat(text_get_class, text_get_float);
    class_setHelpName(text_get_class, sym_text);
    
    text_set_class = class_new(sym_text__space__set,
        (t_newmethod)text_set_new, (t_method)text_client_free,
            sizeof(t_text_set), 0, A_GIMME, 0);
    class_addList(text_set_class, text_set_list);
    class_setHelpName(text_set_class, sym_text);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
