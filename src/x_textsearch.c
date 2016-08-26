
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

#define x_obj x_tc.tc_obj
#define x_sym x_tc.tc_name
#define x_gp x_tc.tc_gpointer
#define x_struct x_tc.tc_templateIdentifier

/* ---- text_search object - output index of line(s) matching criteria ---- */

t_class *text_search_class;

    /* relations we can test when searching: */
#define KB_EQ 0     /*  equal */
#define KB_GT 1     /* > (etc..) */
#define KB_GE 2
#define KB_LT 3
#define KB_LE 4
#define KB_NEAR 5   /* anything matches but closer is better */

typedef struct _key
{
    int k_field;
    int k_binop;
} t_key;

typedef struct _text_search
{
    t_textclient x_tc;
    t_outlet *x_out1;       /* line indices */
    int x_nkeys;
    t_key *x_keyvec;
} t_text_search;

void *textsearch_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_search *x = (t_text_search *)pd_new(text_search_class);
    int i, key, nkey, nextop;
    x->x_out1 = outlet_new(&x->x_obj, &s_list);
    textclient_init(&x->x_tc, &argc, &argv);
    for (i = nkey = 0; i < argc; i++)
        if (argv[i].a_type == A_FLOAT)
            nkey++;
    if (nkey == 0)
        nkey = 1;
    x->x_nkeys = nkey;
    x->x_keyvec = (t_key *)PD_MEMORY_GET(nkey * sizeof(*x->x_keyvec));
    if (!argc)
        x->x_keyvec[0].k_field = 0, x->x_keyvec[0].k_binop = KB_EQ; 
    else for (i = key = 0, nextop = -1; i < argc; i++)
    {
        if (argv[i].a_type == A_FLOAT)
        {
            x->x_keyvec[key].k_field =
                (argv[i].a_w.w_float > 0 ? argv[i].a_w.w_float : 0);
            x->x_keyvec[key].k_binop = (nextop >= 0 ? nextop : KB_EQ);
            nextop = -1;
            key++;
        }
        else
        {
            char *s = argv[i].a_w.w_symbol->s_name;
            if (nextop >= 0)
                post_error ("text search: extra operation argument ignored: %s", s);
            else if (!strcmp(argv[i].a_w.w_symbol->s_name, ">"))
                nextop = KB_GT;
            else if (!strcmp(argv[i].a_w.w_symbol->s_name, ">="))
                nextop = KB_GE;
            else if (!strcmp(argv[i].a_w.w_symbol->s_name, "<"))
                nextop = KB_LT;
            else if (!strcmp(argv[i].a_w.w_symbol->s_name, "<="))
                nextop = KB_LE;
            else if (!strcmp(argv[i].a_w.w_symbol->s_name, "near"))
                nextop = KB_NEAR;
            else post_error ("text search: unknown operation argument: %s", s);
        }
    }
    if (x->x_struct)
        inlet_newPointer(&x->x_obj, &x->x_gp);
    else inlet_newSymbol(&x->x_obj, &x->x_sym);
    return (x);
}

static void text_search_list(t_text_search *x,
    t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_tc);
    int i, j, n, lineno, bestline = -1, beststart, bestn, thisstart, thisn,
        nkeys = x->x_nkeys, failed = 0;
    t_atom *vec;
    t_key *kp = x->x_keyvec;
    if (!b)
       return;
    if (argc < nkeys)
    {
        post_error ("need %d keys, only got %d in list",
            nkeys, argc);
    }
    vec = buffer_atoms(b);
    n = buffer_size(b);
    if (nkeys < 1) { PD_BUG; }
    for (i = lineno = thisstart = 0; i < n; i++)
    {
        if (vec[i].a_type == A_SEMICOLON || vec[i].a_type == A_COMMA || i == n-1)
        {
            int thisn = i - thisstart, j, field = x->x_keyvec[0].k_field,
                binop = x->x_keyvec[0].k_binop;
                /* do we match? */
            for (j = 0; j < argc; )
            {
                if (field >= thisn ||
                    vec[thisstart+field].a_type != argv[j].a_type)
                        goto nomatch;
                if (argv[j].a_type == A_FLOAT)      /* arg is a float */
                {
                    switch (binop)
                    {
                        case KB_EQ:
                            if (vec[thisstart+field].a_w.w_float !=
                                argv[j].a_w.w_float)
                                    goto nomatch;
                        break;
                        case KB_GT:
                            if (vec[thisstart+field].a_w.w_float <=
                                argv[j].a_w.w_float)
                                    goto nomatch;
                        break;
                        case KB_GE:
                            if (vec[thisstart+field].a_w.w_float <
                                argv[j].a_w.w_float)
                                    goto nomatch;
                        case KB_LT:
                            if (vec[thisstart+field].a_w.w_float >=
                                argv[j].a_w.w_float)
                                    goto nomatch;
                        break;
                        case KB_LE:
                            if (vec[thisstart+field].a_w.w_float >
                                argv[j].a_w.w_float)
                                    goto nomatch;
                        break;
                            /* the other possibility ('near') never fails */
                    }
                }
                else                                /* arg is a symbol */
                {
                    if (binop != KB_EQ)
                    {
                        if (!failed)
                        {
                            post_error ("text search (%s): only exact matches allowed for symbols",
                                argv[j].a_w.w_symbol->s_name);
                            failed = 1;
                        }
                        goto nomatch;
                    }
                    if (vec[thisstart+field].a_w.w_symbol !=
                        argv[j].a_w.w_symbol)
                            goto nomatch;
                }
                if (++j >= nkeys)    /* if at last key just increment field */
                    field++;
                else field = x->x_keyvec[j].k_field,    /* else next key */
                        binop = x->x_keyvec[j].k_binop;
            }
                /* the line matches.  Now, if there is a previous match, are
                we better than it? */
            if (bestline >= 0)
            {
                field = x->x_keyvec[0].k_field;
                binop = x->x_keyvec[0].k_binop;
                for (j = 0; j < argc; )
                {
                    if (field >= thisn
                        || vec[thisstart+field].a_type != argv[j].a_type) { PD_BUG; }
                    if (argv[j].a_type == A_FLOAT)      /* arg is a float */
                    {
                        float thisv = vec[thisstart+field].a_w.w_float, 
                            bestv = vec[beststart+field].a_w.w_float;
                        switch (binop)
                        {
                            case KB_GT:
                            case KB_GE:
                                if (thisv < bestv)
                                    goto replace;
                                else if (thisv > bestv)
                                    goto nomatch;
                            break;
                            case KB_LT:
                            case KB_LE:
                                if (thisv > bestv)
                                    goto replace;
                                else if (thisv < bestv)
                                    goto nomatch;
                            case KB_NEAR:
                                if (thisv >= argv[j].a_w.w_float &&
                                    bestv >= argv[j].a_w.w_float)
                                {
                                    if (thisv < bestv)
                                        goto replace;
                                    else if (thisv > bestv)
                                        goto nomatch;
                                }
                                else if (thisv <= argv[j].a_w.w_float &&
                                    bestv <= argv[j].a_w.w_float)
                                {
                                    if (thisv > bestv)
                                        goto replace;
                                    else if (thisv < bestv)
                                        goto nomatch;
                                }
                                else
                                {
                                    float d1 = thisv - argv[j].a_w.w_float,
                                        d2 = bestv - argv[j].a_w.w_float;
                                    if (d1 < 0)
                                        d1 = -d1;
                                    if (d2 < 0)
                                        d2 = -d2;
                                        
                                    if (d1 < d2)
                                        goto replace;
                                    else if (d1 > d2)
                                        goto nomatch;
                                }   
                            break;
                                /* the other possibility ('=') never decides */
                        }
                    }
                    if (++j >= nkeys)    /* last key - increment field */
                        field++;
                    else field = x->x_keyvec[j].k_field,    /* else next key */
                            binop = x->x_keyvec[j].k_binop;
                }
                goto nomatch;   /* a tie - keep the old one */
            replace:
                bestline = lineno, beststart = thisstart, bestn = thisn;
            }
                /* no previous match so we're best */
            else bestline = lineno, beststart = thisstart, bestn = thisn;
        nomatch:
            lineno++;
            thisstart = i+1;
        }
    }
    outlet_float(x->x_out1, bestline);
}

void textsearch_setup (void)
{
    text_search_class = class_new(sym_text__space__search,
        (t_newmethod)textsearch_new, (t_method)textclient_free,
            sizeof(t_text_search), 0, A_GIMME, 0);
    class_addList(text_search_class, text_search_list);
    class_setHelpName(text_search_class, sym_text);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
