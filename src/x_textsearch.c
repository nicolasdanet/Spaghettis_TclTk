
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

static t_class *textsearch_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define TEXTSEARCH_EQUAL                0
#define TEXTSEARCH_GREATER              1
#define TEXTSEARCH_GREATER_EQUAL        2
#define TEXTSEARCH_LESS                 3
#define TEXTSEARCH_LESS_EQUAL           4
#define TEXTSEARCH_NEAR                 5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _key {
    int k_field;
    int k_type;
    } t_key;

typedef struct _textsearch {
    t_textclient    x_textclient;
    int             x_numberOfKeys;
    t_key           *x_keys;
    t_outlet        *x_outlet;
    } t_textsearch;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textsearch_list (t_textsearch *x, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) {
    //
    int i, bestLine  = -1;
    int currentLine  = 0;
    int currentStart = 0;

    for (i = 0; i < buffer_size (b); i++) {
    //
    int j;
    int beststart;
    int bestn;
    int thisn;
    int failed = 0;

    t_atom *vec = buffer_atoms (b);
    
    if (IS_SEMICOLON_OR_COMMA (buffer_atomAtIndex (b, i)) || i == (buffer_size (b) - 1)) {
    //
    int thisn = i - currentStart, j, field = x->x_keys[0].k_field,
        binop = x->x_keys[0].k_type;
        /* do we match? */
    for (j = 0; j < argc; )
    {
        if (field >= thisn ||
            vec[currentStart+field].a_type != argv[j].a_type)
                goto nomatch;
        if (argv[j].a_type == A_FLOAT)      /* arg is a float */
        {
            switch (binop)
            {
                case TEXTSEARCH_EQUAL:
                    if (vec[currentStart+field].a_w.w_float !=
                        argv[j].a_w.w_float)
                            goto nomatch;
                break;
                case TEXTSEARCH_GREATER:
                    if (vec[currentStart+field].a_w.w_float <=
                        argv[j].a_w.w_float)
                            goto nomatch;
                break;
                case TEXTSEARCH_GREATER_EQUAL:
                    if (vec[currentStart+field].a_w.w_float <
                        argv[j].a_w.w_float)
                            goto nomatch;
                case TEXTSEARCH_LESS:
                    if (vec[currentStart+field].a_w.w_float >=
                        argv[j].a_w.w_float)
                            goto nomatch;
                break;
                case TEXTSEARCH_LESS_EQUAL:
                    if (vec[currentStart+field].a_w.w_float >
                        argv[j].a_w.w_float)
                            goto nomatch;
                break;
                    /* the other possibility ('near') never fails */
            }
        }
        else                                /* arg is a symbol */
        {
            if (binop != TEXTSEARCH_EQUAL)
            {
                if (!failed)
                {
                    post_error ("text search (%s): only exact matches allowed for symbols",
                        argv[j].a_w.w_symbol->s_name);
                    failed = 1;
                }
                goto nomatch;
            }
            if (vec[currentStart+field].a_w.w_symbol !=
                argv[j].a_w.w_symbol)
                    goto nomatch;
        }
        if (++j >= x->x_numberOfKeys)    /* if at last key just increment field */
            field++;
        else field = x->x_keys[j].k_field,    /* else next key */
                binop = x->x_keys[j].k_type;
    }
        /* the line matches.  Now, if there is a previous match, are
        we better than it? */
    if (bestLine >= 0)
    {
        field = x->x_keys[0].k_field;
        binop = x->x_keys[0].k_type;
        for (j = 0; j < argc; )
        {
            if (field >= thisn
                || vec[currentStart+field].a_type != argv[j].a_type) { PD_BUG; }
            if (argv[j].a_type == A_FLOAT)      /* arg is a float */
            {
                float thisv = vec[currentStart+field].a_w.w_float, 
                    bestv = vec[beststart+field].a_w.w_float;
                switch (binop)
                {
                    case TEXTSEARCH_GREATER:
                    case TEXTSEARCH_GREATER_EQUAL:
                        if (thisv < bestv)
                            goto replace;
                        else if (thisv > bestv)
                            goto nomatch;
                    break;
                    case TEXTSEARCH_LESS:
                    case TEXTSEARCH_LESS_EQUAL:
                        if (thisv > bestv)
                            goto replace;
                        else if (thisv < bestv)
                            goto nomatch;
                    case TEXTSEARCH_NEAR:
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
            if (++j >= x->x_numberOfKeys)    /* last key - increment field */
                field++;
            else field = x->x_keys[j].k_field,    /* else next key */
                    binop = x->x_keys[j].k_type;
        }
        goto nomatch;   /* a tie - keep the old one */
    replace:
        bestLine = currentLine, beststart = currentStart, bestn = thisn;
    }
        /* no previous match so we're best */
    else bestLine = currentLine, beststart = currentStart, bestn = thisn;
nomatch:
    currentLine++;
    currentStart = i+1;
    //
    }
    //
    }
    
    outlet_float (x->x_outlet, bestLine);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *textsearch_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textsearch *x = (t_textsearch *)pd_new (textsearch_class);
    
    int i, numberOfKeys = 0;
    
    textclient_init (&x->x_textclient, &argc, &argv);           /* Note that it may consume arguments. */
    
    for (i = 0; i < argc; i++) { if (IS_FLOAT (argv + i)) { numberOfKeys++; } }
    
    x->x_numberOfKeys   = PD_MAX (1, numberOfKeys);
    x->x_keys           = (t_key *)PD_MEMORY_GET (x->x_numberOfKeys * sizeof (t_key));
    x->x_outlet         = outlet_new (cast_object (x), &s_list);
     
    if (!numberOfKeys) { x->x_keys[0].k_field = 0; x->x_keys[0].k_type = TEXTSEARCH_EQUAL; }
    else {
    //
    int key = 0;
    int operator = -1;
    
    for (i = 0; i < argc; i++) {
    //
    if (IS_FLOAT (argv + i)) {
        x->x_keys[key].k_field = PD_MAX (0.0, (int)GET_FLOAT (argv + i));
        x->x_keys[key].k_type  = PD_MAX (TEXTSEARCH_EQUAL, operator);
        operator = -1;
        key++;
        
    } else {
        t_symbol *t = atom_getSymbolAtIndex (i, argc, argv);
        
        if (operator < 0) {
            if (t == sym___greater__)                { operator = TEXTSEARCH_GREATER;       }
            else if (t == sym___greater____equals__) { operator = TEXTSEARCH_GREATER_EQUAL; }
            else if (t == sym___less__)              { operator = TEXTSEARCH_LESS;          }
            else if (t == sym___less____equals__)    { operator = TEXTSEARCH_LESS_EQUAL;    }
            else if (t == sym_near)                  { operator = TEXTSEARCH_NEAR;          }
        }
    }
    //
    }
    //
    }
    
    if (TEXTCLIENT_ASPOINTER (&x->x_textclient)) { 
        inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
    } else {
        inlet_newSymbol(cast_object (x), TEXTCLIENT_GETNAME (&x->x_textclient));
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textsearch_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__search,
            (t_newmethod)textsearch_new,
            (t_method)textclient_free,
            sizeof (t_textsearch),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, textsearch_list);
    class_setHelpName (c, sym_text);
    
    textsearch_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
