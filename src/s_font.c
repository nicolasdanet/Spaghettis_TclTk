
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_fontinfo sys_fontlist[] = {
    {8, 6, 10, 1, 1, 1}, {10, 7, 13, 1, 1, 1}, {12, 9, 16, 1, 1, 1},
    {16, 10, 21, 1, 1, 1}, {24, 15, 25, 1, 1, 1}, {36, 25, 45, 1, 1, 1}};
#define NFONT (sizeof(sys_fontlist)/sizeof(*sys_fontlist))

/* here are the actual font size structs on msp's systems:
MSW:
font 8 5 9 8 5 11
font 10 7 13 10 6 13
font 12 9 16 14 8 16
font 16 10 20 16 10 18
font 24 15 25 16 10 18
font 36 25 42 36 22 41

linux:
font 8 5 9 8 5 9
font 10 7 13 12 7 13
font 12 9 16 14 9 15
font 16 10 20 16 10 19
font 24 15 25 24 15 24
font 36 25 42 36 22 41
*/

static t_fontinfo *sys_findfont(int fontsize)
{
    unsigned int i;
    t_fontinfo *fi;
    for (i = 0, fi = sys_fontlist; i < (NFONT-1); i++, fi++)
        if (fontsize < fi[1].fi_fontsize) return (fi);
    return (sys_fontlist + (NFONT-1));
}

int sys_nearestfontsize(int fontsize)
{
    return (sys_findfont(fontsize)->fi_fontsize);
}

int sys_hostfontsize(int fontsize)
{
    return (sys_findfont(fontsize)->fi_hostfontsize);
}

int sys_fontwidth(int fontsize)
{
    return (sys_findfont(fontsize)->fi_width);
}

int sys_fontheight(int fontsize)
{
    return (sys_findfont(fontsize)->fi_height);
}

int sys_defaultfont;    /* Shared. */

static void openit(const char *dirname, const char *filename)
{
    char dirbuf[PD_STRING], *nameptr;
    int fd = open_via_path(dirname, filename, "", dirbuf, &nameptr,
        PD_STRING, 0);
    if (fd >= 0)
    {
        close (fd);
        buffer_openFile(0, gensym(nameptr), gensym(dirbuf));
    }
    else
        post_error ("%s: can't open", filename);
}

/* this is called from the gui process.  The first argument is the cwd, and
succeeding args give the widths and heights of known fonts.  We wait until 
these are known to open files and send messages specified on the command line.
We ask the GUI to specify the "cwd" in case we don't have a local OS to get it
from; for instance we could be some kind of RT embedded system.  However, to
really make this make sense we would have to implement
open(), read(), etc, calls to be served somehow from the GUI too. */

void global_gui(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    char *cwd = atom_getSymbolAtIndex(0, argc, argv)->s_name;
    t_pathlist *nl;
    unsigned int i;
    int j;
    int nhostfont = (argc-1)/3;
    /* */
    if (argc != 1 + 3 * nhostfont) { PD_BUG; }
    for (i = 0; i < NFONT; i++)
    {
        int best = 0;
        int wantheight = sys_fontlist[i].fi_maxheight;
        int wantwidth = sys_fontlist[i].fi_maxwidth;
        for (j = 0; j < nhostfont; j++)
        {
            if ((t_int)atom_getFloatAtIndex(3 * j + 3, argc, argv) <= wantheight &&
                (t_int)atom_getFloatAtIndex(3 * j + 2, argc, argv) <= wantwidth)
                    best = j;
        }
            /* best is now the host font index for the desired font index i. */
        sys_fontlist[i].fi_hostfontsize =
            (t_int)atom_getFloatAtIndex(3 * best + 1, argc, argv);
        sys_fontlist[i].fi_width = (t_int)atom_getFloatAtIndex(3 * best + 2, argc, argv);
        sys_fontlist[i].fi_height = (t_int)atom_getFloatAtIndex(3 * best + 3, argc, argv);
    }
#if 0
    for (i = 0; i < 6; i++)
        fprintf(stderr, "font (%d %d %d) -> (%d %d %d)\n",
            sys_fontlist[i].fi_fontsize,
            sys_fontlist[i].fi_maxwidth,
            sys_fontlist[i].fi_maxheight,
            sys_fontlist[i].fi_hostfontsize,
            sys_fontlist[i].fi_width,
            sys_fontlist[i].fi_height);
#endif
        /* load dynamic libraries specified with "-lib" args */
    /*for  (nl = sys_externlist; nl; nl = nl->nl_next)
        if (!sys_load_lib(0, nl->nl_string))
            post("%s: can't load library", nl->nl_string);*/
        /* open patches specifies with "-open" args */
    /*for  (nl = main_openList; nl; nl = nl->nl_next)
        openit(cwd, nl->nl_string);
    pathlist_free(main_openList);
    main_openList = 0;*/

    /*
    for  (nl = main_messageList; nl; nl = nl->nl_next)
    {
        t_buffer *b = buffer_new();
        buffer_withStringUnzeroed(b, nl->nl_string, strlen(nl->nl_string));
        buffer_eval(b, 0, 0, 0);
        buffer_free(b);
    }
    pathlist_free(main_messageList);
    main_messageList = 0;*/
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
