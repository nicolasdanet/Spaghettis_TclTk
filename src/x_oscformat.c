
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
#include "g_graphics.h"

#define ROUNDUPTO4(x) (((x) + 3) & (~3))

#define READINT(x)  ((((int)(((x)  )->a_w.w_float)) & 0xff) << 24) | \
                    ((((int)(((x)+1)->a_w.w_float)) & 0xff) << 16) | \
                    ((((int)(((x)+2)->a_w.w_float)) & 0xff) << 8) | \
                    ((((int)(((x)+3)->a_w.w_float)) & 0xff) << 0)

static t_class *oscformat_class;

typedef struct _oscformat
{
    t_object x_obj;
    char *x_pathbuf;
    int x_pathsize;
    t_symbol *x_format;
} t_oscformat;

static void oscformat_set(t_oscformat *x, t_symbol *s, int argc, t_atom *argv)
{
    char buf[PD_STRING];
    int i, newsize;
    *x->x_pathbuf = 0;
    buf[0] = '/';
    for (i = 0; i < argc; i++)
    {
        char *where = (argv[i].a_type == A_SYMBOL &&
            *argv[i].a_w.w_symbol->s_name == '/' ? buf : buf+1);
        atom_toString(&argv[i], where, PD_STRING-1);
        if ((newsize = strlen(buf) + strlen(x->x_pathbuf) + 1) > x->x_pathsize)
        {
            x->x_pathbuf = PD_MEMORY_RESIZE(x->x_pathbuf, x->x_pathsize, newsize);
            x->x_pathsize = newsize;
        }
        strcat(x->x_pathbuf, buf);
    }
}

static void oscformat_format(t_oscformat *x, t_symbol *s)
{
    char *sp;
    for (sp = s->s_name; *sp; sp++)
    {
        if (*sp != 'f' && *sp != 'i' && *sp != 's' && *sp != 'b')
        {
            post_error ("oscformat '%s' may only contain 'f', 'i'. 's', and/or 'b'",
                    sp);
            return;
        }
    }
    x->x_format = s;
}

#define WRITEINT(msg, i)    SET_FLOAT((msg),   (((i) >> 24) & 0xff)); \
                            SET_FLOAT((msg)+1, (((i) >> 16) & 0xff)); \
                            SET_FLOAT((msg)+2, (((i) >>  8) & 0xff)); \
                            SET_FLOAT((msg)+3, (((i)      ) & 0xff))

static void putstring(t_atom *msg, int *ip, const char *s)
{
    const char *sp = s;
    do
    {
        SET_FLOAT(&msg[*ip], *sp & 0xff);
        (*ip)++;
    }
    while (*sp++);
    while (*ip & 3)
    {
        SET_FLOAT(&msg[*ip], 0);
        (*ip)++;
    }
}

static void oscformat_list(t_oscformat *x, t_symbol *s, int argc, t_atom *argv)
{
    int typeindex = 0, j, msgindex, msgsize, datastart, ndata;
    t_atom *msg;
    char *sp, *formatp = x->x_format->s_name, typecode;
        /* pass 1: go through args to find overall message size */
    for (j = ndata = 0, sp = formatp, msgindex = 0; j < argc;)
    {
        if (*sp)
            typecode = *sp++;
        else if (argv[j].a_type == A_SYMBOL)
            typecode = 's';
        else typecode = 'f';
        if (typecode == 's')
            msgindex += ROUNDUPTO4(strlen(argv[j].a_w.w_symbol->s_name) + 1);
        else if (typecode == 'b')
        {
            int blobsize = PD_INT_MAX, blobindex;
                /* check if we have a nonnegative size field */ 
            if (argv[j].a_type == A_FLOAT &&
                (int)(argv[j].a_w.w_float) >= 0)
                    blobsize = (int)(argv[j].a_w.w_float);
            if (blobsize > argc - j - 1)
                blobsize = argc - j - 1;    /* if no or bad size, eat it all */ 
            msgindex += 4 + ROUNDUPTO4(blobsize);
            j += blobsize;
        }
        else msgindex += 4;
        j++;
        ndata++;
    }
    datastart = ROUNDUPTO4(strlen(x->x_pathbuf)+1) + ROUNDUPTO4(ndata + 2);
    msgsize = datastart + msgindex;
    msg = (t_atom *)alloca(msgsize * sizeof(t_atom));
    putstring(msg, &typeindex, x->x_pathbuf);
    SET_FLOAT(&msg[typeindex], ',');
    typeindex++;
        /* pass 2: fill in types and data portion of packet */
    for (j = 0, sp = formatp, msgindex = datastart; j < argc;)
    {
        if (*sp)
            typecode = *sp++;
        else if (argv[j].a_type == A_SYMBOL)
            typecode = 's';
        else typecode = 'f';
        SET_FLOAT(&msg[typeindex], typecode & 0xff);
        typeindex++;
        if (typecode == 'f')
        {
            union
            {
                float z_f;
                uint32_t z_i;
            } z;
            z.z_f = atom_getFloat(&argv[j]);
            WRITEINT(msg+msgindex, z.z_i);
            msgindex += 4;
        }
        else if (typecode == 'i')
        {
            int dat = atom_getFloat(&argv[j]);
            WRITEINT(msg+msgindex, dat);
            msgindex += 4;
        }
        else if (typecode == 's')
            putstring(msg, &msgindex, argv[j].a_w.w_symbol->s_name);
        else if (typecode == 'b')
        {
            int blobsize = PD_INT_MAX, blobindex;
            if (argv[j].a_type == A_FLOAT &&
                (int)(argv[j].a_w.w_float) >= 0)
                    blobsize = (int)(argv[j].a_w.w_float);
            if (blobsize > argc - j - 1)
                blobsize = argc - j - 1;
            WRITEINT(msg+msgindex, blobsize);
            msgindex += 4;
            for (blobindex = 0; blobindex < blobsize; blobindex++)
                SET_FLOAT(msg+msgindex+blobindex,
                    (argv[j+1+blobindex].a_type == A_FLOAT ?
                        argv[j+1+blobindex].a_w.w_float :
                        (argv[j+1+blobindex].a_type == A_SYMBOL ?
                            argv[j+1+blobindex].a_w.w_symbol->s_name[0] & 0xff :
                            0)));
            j += blobsize;
            while (blobsize & 3)
                SET_FLOAT(msg+msgindex+blobsize, 0), blobsize++;
            msgindex += blobsize;
        }
        j++;
    }
    SET_FLOAT(&msg[typeindex], 0);
    typeindex++;
    while (typeindex & 3)
        SET_FLOAT(&msg[typeindex], 0), typeindex++;
    if (typeindex != datastart || msgindex != msgsize) { PD_BUG; }
    /* else post("datastart %d, msgsize %d", datastart, msgsize); */
    outlet_list(x->x_obj.te_outlet, msgsize, msg);
}

static void oscformat_free(t_oscformat *x)
{
    PD_MEMORY_FREE(x->x_pathbuf);
}

static void *oscformat_new(t_symbol *s, int argc, t_atom *argv)
{
    t_oscformat *x = (t_oscformat *)pd_new(oscformat_class);
    outlet_new(&x->x_obj, &s_list);
    x->x_pathbuf = PD_MEMORY_GET(1);
    x->x_pathsize = 1;
    *x->x_pathbuf = 0;
    x->x_format = &s_;
    if (argc > 1 && argv[0].a_type == A_SYMBOL &&
        argv[1].a_type == A_SYMBOL &&
            !strcmp(argv[0].a_w.w_symbol->s_name, "-f"))
    {
        oscformat_format(x, argv[1].a_w.w_symbol);
        argc -= 2;
        argv += 2;
    }
    oscformat_set(x, 0, argc, argv);
    return (x);
}

void oscformat_setup(void)
{
    oscformat_class = class_new(sym_oscformat, (t_newmethod)oscformat_new,
        (t_method)oscformat_free, sizeof(t_oscformat), 0, A_GIMME, 0);
    class_addMethod(oscformat_class, (t_method)oscformat_set,
        sym_set, A_GIMME, 0);
    class_addMethod(oscformat_class, (t_method)oscformat_format,
        sym_format, A_DEFSYMBOL, 0);
    class_addList(oscformat_class, oscformat_list);
}


