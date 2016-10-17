
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
#include "s_system.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *oscparse_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _oscparse {
    t_object    x_obj;                  /* Must be the first. */
    t_outlet    *x_outlet;
    } t_oscparse;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_symbol *grabstring(int argc, t_atom *argv, int *ip, int slash)
{
    char buf[PD_STRING];
    int first, nchar;
    if (slash)
        while (*ip < argc && argv[*ip].a_w.w_float == '/')
            (*ip)++;
    for (nchar = 0; nchar < PD_STRING-1 && *ip < argc; nchar++, (*ip)++)
    {
        char c = argv[*ip].a_w.w_float;
        if (c == 0 || (slash && c == '/'))
            break;
        buf[nchar] = c;
    }
    buf[nchar] = 0;
    if (!slash)
        *ip = OSC_ROUNDUP(*ip+1);
    if (*ip > argc)
        *ip = argc;
    return (gensym (buf));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void oscparse_list(t_oscparse *x, t_symbol *s, int argc, t_atom *argv)
{
    int i, j, j2, k, outc = 1, blob = 0, typeonset, dataonset, nfield;
    t_atom *outv;
    if (!argc)
        return;
    for (i = 0; i < argc; i++)
        if (argv[i].a_type != A_FLOAT)
    {
        post_error ("oscparse: takes numbers only");
        return;
    }
    if (argv[0].a_w.w_float == '#') /* it's a bundle */
    {
        if (argv[1].a_w.w_float != 'b' || argc < 16)
        {
            post_error ("oscparse: malformed bundle");
            return;
        }
            /* we ignore the timetag since there's no correct way to
            convert it to Pd logical time that I can think of.  LATER
            consider at least outputting timetag differentially converted
            into Pd time units. */
        for (i = 16; i < argc-4; )
        {
            int msize = OSC_READ(argv+i);
            if (msize <= 0 || msize & 3)
            {
                post_error ("oscparse: bad bundle element size");
                return;
            }
            oscparse_list(x, 0, msize, argv+i+4);
            i += msize+4;
        }
        return;
    }
    else if (argv[0].a_w.w_float != '/')
    {
        post_error ("oscparse: not an OSC message (no leading slash)");
        return;
    }
    for (i = 1; i < argc && argv[i].a_w.w_float != 0; i++)
        if (argv[i].a_w.w_float == '/')
            outc++;
    i = OSC_ROUNDUP(i+1);
    if (argv[i].a_w.w_float != ',' || (i+1) >= argc)
    {
        post_error ("oscparse: malformed type string (char %d, index %d)",
            (int)(argv[i].a_w.w_float), i);
        return;
    }
    typeonset = ++i;
    for (; i < argc && argv[i].a_w.w_float != 0; i++)
        if (argv[i].a_w.w_float == 'b')
            blob = 1;
    nfield = i - typeonset;
    if (blob)
        outc += argc - typeonset;
    else outc += nfield;
    outv = (t_atom *)alloca(outc * sizeof(t_atom));
    dataonset = OSC_ROUNDUP(i + 1);
    /* post("outc %d, typeonset %d, dataonset %d, nfield %d", outc, typeonset, 
        dataonset, nfield); */
    for (i = j = 0; i < typeonset-1 && argv[i].a_w.w_float != 0 &&
        j < outc; j++)
            SET_SYMBOL(outv+j, grabstring(argc, argv, &i, 1));
    for (i = typeonset, k = dataonset; i < typeonset + nfield; i++)
    {
        union
        {
            float z_f;
            uint32_t z_i;
        } z;
        float f;
        int blobsize;
        switch ((int)(argv[i].a_w.w_float))
        {
        case 'f':
            if (k > argc - 4)
                goto tooshort;
            z.z_i = OSC_READ(argv+k);
            f = z.z_f;
            if (PD_DENORMAL_OR_ZERO(f))
                f = 0;
            if (j >= outc)
            {
                PD_BUG;
                return;
            }
            SET_FLOAT(outv+j, f);
            j++; k += 4;
            break;
        case 'i':
            if (k > argc - 4)
                goto tooshort;
            if (j >= outc)
            {
                PD_BUG;
                return;
            }
            SET_FLOAT(outv+j, OSC_READ(argv+k));
            j++; k += 4;
            break;
        case 's':
            if (j >= outc)
            {
                PD_BUG;
                return;
            }
            SET_SYMBOL(outv+j, grabstring(argc, argv, &k, 0));
            j++;
            break;
        case 'b':
            if (k > argc - 4)
                goto tooshort;
            blobsize = OSC_READ(argv+k);
            k += 4;
            if (blobsize < 0 || blobsize > argc - k)
                goto tooshort;
            if (j + blobsize + 1 > outc)
            {
                PD_BUG;
                return;
            }
            if (k + blobsize > argc)
                goto tooshort;
            SET_FLOAT(outv+j, blobsize);
            j++;
            for (j2 = 0; j2 < blobsize; j++, j2++, k++)
                SET_FLOAT(outv+j, argv[k].a_w.w_float);
            k = OSC_ROUNDUP(k);
            break;
        default:
            post_error ("oscparse: unknown tag '%c' (%d)", 
                (int)(argv[i].a_w.w_float), (int)(argv[i].a_w.w_float));
        } 
    }
    outlet_list(x->x_obj.te_outlet, j, outv);
    return;
tooshort:
    post_error ("oscparse: OSC message ended prematurely");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_oscparse *oscparse_new (t_symbol *s, int argc, t_atom *argv)
{
    t_oscparse *x = (t_oscparse *)pd_new (oscparse_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_list);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void oscparse_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_oscparse,
            (t_newmethod)oscparse_new,
            NULL,
            sizeof (t_oscparse),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, oscparse_list);
    
    oscparse_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
