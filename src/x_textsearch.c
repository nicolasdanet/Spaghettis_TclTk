
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

static int textsearch_listIsMatch (t_textsearch *x, t_buffer *b, int start, int end, int argc, t_atom *argv)
{
    int field = x->x_keys[0].k_field;
    int type  = x->x_keys[0].k_type;
    int count = end - start;
    int j;
    
    for (j = 0; j < argc; ) {
    //
    if (field < count && atom_typesAreEqual (buffer_atomAtIndex (b, start + field), argv + j)) {
    //
    if (IS_FLOAT (argv + j)) {
        t_float f1 = GET_FLOAT (buffer_atomAtIndex (b, start + field));
        t_float f2 = GET_FLOAT (argv + j);
        switch (type) {
            case TEXTSEARCH_EQUAL           : if (f1 != f2) { return 0; } break;
            case TEXTSEARCH_GREATER         : if (f1 <= f2) { return 0; } break;
            case TEXTSEARCH_GREATER_EQUAL   : if (f1 < f2)  { return 0; } break;
            case TEXTSEARCH_LESS            : if (f1 >= f2) { return 0; } break;
            case TEXTSEARCH_LESS_EQUAL      : if (f1 > f2)  { return 0; } break;
            case TEXTSEARCH_NEAR            : break;
        }
        
    } else {
        if (type != TEXTSEARCH_EQUAL) { return 0; }
        if (atom_getSymbol (buffer_atomAtIndex (b, start + field)) != atom_getSymbol (argv + j)) {
            return 0; 
        }
    }
    
    if (++j < x->x_numberOfKeys) { field = x->x_keys[j].k_field; type = x->x_keys[j].k_type; }
    else {
        field++;
    }
    //
    } else {
        return 0;
    }
    //
    }

    return 1;
}

static void textsearch_list (t_textsearch *x, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) {
    //
    int numberOfLines = buffer_getNumberOfMessages (b);
    int bestLine = -1;
    int bestLineStart;
    int i, start, end;
        
    for (i = 0; i < numberOfLines; i++) {
    //
    buffer_getMessageAt (b, i, &start, &end);

    if (textsearch_listIsMatch (x, b, start, end, argc, argv)) {
    //
    if (bestLine >= 0)
    {
        t_atom *vec = buffer_atoms (b);
        int count = end - start;
        int j;
        int field = x->x_keys[0].k_field;
        int type = x->x_keys[0].k_type;
        
        for (j = 0; j < argc; )
        {
            if (field >= count
                || vec[start+field].a_type != argv[j].a_type) { PD_BUG; }
            if (argv[j].a_type == A_FLOAT)      /* arg is a float */
            {
                float thisv = vec[start+field].a_w.w_float, 
                    bestv = vec[bestLineStart+field].a_w.w_float;
                switch (type)
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
                    type = x->x_keys[j].k_type;
        }
        goto nomatch;   /* a tie - keep the old one */
replace:
        bestLine = i, bestLineStart = start;
    } else {
        bestLine = i, bestLineStart = start;
    }
    //
    }
nomatch:
    ;
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
