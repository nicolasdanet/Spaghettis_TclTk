
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
#include "s_system.h"
#include "g_canvas.h"
#include "s_utf8.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BOX_TAG_SIZE    50

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _boxtext {
    struct _boxtext     *box_next;
    t_object            *box_owner;
    t_glist             *box_view;
    char                *box_utf8;
    int                 box_utf8Size;
    int                 box_selectionStart; 
    int                 box_selectionEnd;
    int                 box_draggedFrom;
    int                 box_isActive;
    int                 box_width;
    int                 box_height;
    char                box_tag[BOX_TAG_SIZE];
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BOX_MARGIN_LEFT         2
#define BOX_MARGIN_RIGHT        2
#define BOX_MARGIN_TOP          2
#define BOX_MARGIN_BOTTOM       2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define BOX_CHECK               0
#define BOX_FIRST               1
#define BOX_UPDATE              2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_boxtext *rtext_new(t_glist *glist, t_object *who)
{
    t_boxtext *x = (t_boxtext *)PD_MEMORY_GET(sizeof *x);
    int w = 0, h = 0, indx;
    x->box_owner = who;
    x->box_view = glist;
    x->box_next = glist->gl_editor->e_text;
    x->box_selectionStart = x->box_selectionEnd = x->box_isActive =
        x->box_width = x->box_height = 0;
    buffer_toStringUnzeroed(who->te_buffer, &x->box_utf8, &x->box_utf8Size);
    glist->gl_editor->e_text = x;
    sprintf(x->box_tag, ".x%lx.t%lx", (t_int)canvas_getView(x->box_view),
        (t_int)x);
    return (x);
}

void rtext_free(t_boxtext *x)
{
    if (x->box_view->gl_editor->e_selectedText == x)
        x->box_view->gl_editor->e_selectedText = 0;
    if (x->box_view->gl_editor->e_text == x)
        x->box_view->gl_editor->e_text = x->box_next;
    else
    {
        t_boxtext *e2;
        for (e2 = x->box_view->gl_editor->e_text; e2; e2 = e2->box_next)
            if (e2->box_next == x)
        {
            e2->box_next = x->box_next;
            break;
        }
    }

    PD_MEMORY_FREE(x->box_utf8);
    PD_MEMORY_FREE(x);
}

char *rtext_gettag(t_boxtext *x)
{
    return (x->box_tag);
}

void rtext_gettext(t_boxtext *x, char **buf, int *bufsize)
{
    *buf = x->box_utf8;
    *bufsize = x->box_utf8Size;
}

void rtext_getseltext(t_boxtext *x, char **buf, int *bufsize)
{
    *buf = x->box_utf8 + x->box_selectionStart;
    *bufsize = x->box_selectionEnd - x->box_selectionStart;
}

/* convert t_object te_type symbol for use as a Tk tag */
static t_symbol *rtext_gettype(t_boxtext *x)
{
    switch (x->box_owner->te_type) 
    {
    case TYPE_TEXT: return sym_text;
    case TYPE_OBJECT: return sym_obj;
    case TYPE_MESSAGE: return sym_msg;
    case TYPE_ATOM: return sym_atom;
    }
    return (&s_);
}

/* LATER deal with tcl-significant characters */

/* firstone(), lastone()
 *  + returns byte offset of (first|last) occurrence of 'c' in 's[0..n-1]', or
 *    -1 if none was found
 *  + 's' is a raw byte string
 *  + 'c' is a byte value
 *  + 'n' is the length (in bytes) of the prefix of 's' to be searched.
 *  + we could make these functions work on logical characters in utf8 strings,
 *    but we don't really need to...
 */
static int firstone(char *s, int c, int n)
{
    char *s2 = s + n;
    int i = 0;
    while (s != s2)
    {
        if (*s == c) return (i);
        i++;
        s++;
    }
    return (-1);
}

static int lastone(char *s, int c, int n)
{
    char *s2 = s + n;
    while (s2 != s)
    {
        s2--;
        n--;
        if (*s2 == c) return (n);
    }
    return (-1);
}

    /* the following routine computes line breaks and carries out
    some action which could be:
        BOX_FIRST - draw the box  for the first time
        BOX_UPDATE - redraw the updated box
        otherwise - don't draw, just calculate.
    Called with *widthp and *heightp as coordinates of
    a test point, the routine reports the index of the character found
    there in *indexp.  *widthp and *heightp are set to the width and height
    of the entire text in pixels.
    */

   /*-- moo: 
    * + some variables from the original version have been renamed
    * + variables with a "_b" suffix are raw byte strings, lengths, or offsets
    * + variables with a "_c" suffix are logical character lengths or offsets
    *   (assuming valid UTF-8 encoded byte string in x->box_utf8)
    * + a fair amount of O(n) computations required to convert between raw byte
    *   offsets (needed by the C side) and logical character offsets (needed by
    *   the GUI)
    */

    /* LATER get this and sys_vGui to work together properly,
        breaking up messages as needed.  As of now, there's
        a limit of 1950 characters, imposed by sys_vGui(). */
#define UPBUFSIZE 4000
#define BOXWIDTH 60

static void rtext_senditup(t_boxtext *x, int action, int *widthp, int *heightp,
    int *indexp)
{
    t_float dispx, dispy;
    char smallbuf[200], *tempbuf;
    int outchars_b = 0, nlines = 0, ncolumns = 0,
        pixwide, pixhigh, font, fontwidth, fontheight, findx, findy;
    int reportedindex = 0;
    t_glist *canvas = canvas_getView(x->box_view);
    int widthspec_c = x->box_owner->te_width;
    int widthlimit_c = (widthspec_c ? widthspec_c : BOXWIDTH);
    int inindex_b = 0;
    int inindex_c = 0;
    int selstart_b = 0, selend_b = 0;
    int box_utf8Size_c = u8_charnum(x->box_utf8, x->box_utf8Size);
        /* if we're a GOP (the new, "goprect" style) borrow the font size
        from the inside to preserve the spacing */
    if (pd_class(&x->box_owner->te_g.g_pd) == canvas_class &&
        ((t_glist *)(x->box_owner))->gl_isGraphOnParent &&
        ((t_glist *)(x->box_owner))->gl_hasRectangle)
            font =  canvas_getFontSize((t_glist *)(x->box_owner));
    else font = canvas_getFontSize(x->box_view);
    fontwidth = font_getHostFontWidth(font);
    fontheight = font_getHostFontHeight(font);
    findx = (*widthp + (fontwidth/2)) / fontwidth;
    findy = *heightp / fontheight;
    if (x->box_utf8Size >= 100)
         tempbuf = (char *)PD_MEMORY_GET(2 * x->box_utf8Size + 1);
    else tempbuf = smallbuf;
    while (box_utf8Size_c - inindex_c > 0)
    {
        int inchars_b  = x->box_utf8Size - inindex_b;
        int inchars_c  = box_utf8Size_c  - inindex_c;
        int maxindex_c = (inchars_c > widthlimit_c ? widthlimit_c : inchars_c);
        int maxindex_b = u8_offset(x->box_utf8 + inindex_b, maxindex_c);
        int eatchar = 1;
        int foundit_b  = firstone(x->box_utf8 + inindex_b, '\n', maxindex_b);
        int foundit_c;
        if (foundit_b < 0)
        {
                /* too much text to fit in one line? */
            if (inchars_c > widthlimit_c)
            {
                    /* is there a space to break the line at?  OK if it's even
                    one byte past the end since in this context we know there's
                    more text */
                foundit_b = lastone(x->box_utf8 + inindex_b, ' ', maxindex_b + 1);
                if (foundit_b < 0)
                {
                    foundit_b = maxindex_b;
                    foundit_c = maxindex_c;
                    eatchar = 0;
                }
                else
                    foundit_c = u8_charnum(x->box_utf8 + inindex_b, foundit_b);
            }
            else
            {
                foundit_b = inchars_b;
                foundit_c = inchars_c;
                eatchar = 0;
            }
        }
        else
            foundit_c = u8_charnum(x->box_utf8 + inindex_b, foundit_b);

        if (nlines == findy)
        {
            int actualx = (findx < 0 ? 0 :
                (findx > foundit_c ? foundit_c : findx));
            *indexp = inindex_b + u8_offset(x->box_utf8 + inindex_b, actualx);
            reportedindex = 1;
        }
        strncpy(tempbuf+outchars_b, x->box_utf8 + inindex_b, foundit_b);
        if (x->box_selectionStart >= inindex_b &&
            x->box_selectionStart <= inindex_b + foundit_b + eatchar)
                selstart_b = x->box_selectionStart + outchars_b - inindex_b;
        if (x->box_selectionEnd >= inindex_b &&
            x->box_selectionEnd <= inindex_b + foundit_b + eatchar)
                selend_b = x->box_selectionEnd + outchars_b - inindex_b;
        outchars_b += foundit_b;
        inindex_b += (foundit_b + eatchar);
        inindex_c += (foundit_c + eatchar);
        if (inindex_b < x->box_utf8Size)
            tempbuf[outchars_b++] = '\n';
        if (foundit_c > ncolumns)
            ncolumns = foundit_c;
        nlines++;
    }
    if (!reportedindex)
        *indexp = outchars_b;
    dispx = text_xpix(x->box_owner, x->box_view);
    dispy = text_ypix(x->box_owner, x->box_view);
    if (nlines < 1) nlines = 1;
    if (!widthspec_c)
    {
        while (ncolumns < (x->box_owner->te_type == TYPE_TEXT ? 1 : 3))
        {
            tempbuf[outchars_b++] = ' ';
            ncolumns++;
        }
    }
    else ncolumns = widthspec_c;
    pixwide = ncolumns * fontwidth + (BOX_MARGIN_LEFT + BOX_MARGIN_RIGHT);
    pixhigh = nlines * fontheight + (BOX_MARGIN_TOP + BOX_MARGIN_BOTTOM);

    if (action && x->box_owner->te_width && x->box_owner->te_type != TYPE_ATOM)
    {
            /* if our width is specified but the "natural" width is the
            same as the specified width, set specified width to zero
            so future text editing will automatically change width.
            Except atoms whose content changes at runtime. */
        int widthwas = x->box_owner->te_width, newwidth = 0, newheight = 0,
            newindex = 0;
        x->box_owner->te_width = 0;
        rtext_senditup(x, 0, &newwidth, &newheight, &newindex);
        if (newwidth/fontwidth != widthwas)
            x->box_owner->te_width = widthwas;
        else x->box_owner->te_width = 0;
    }
    if (action == BOX_FIRST)
    {
        sys_vGui("::ui_object::newText .x%lx.c {%s %s text} %f %f {%.*s} %d %s\n",
            canvas, x->box_tag, rtext_gettype(x)->s_name,
            dispx + BOX_MARGIN_LEFT, dispy + BOX_MARGIN_TOP,
            outchars_b, tempbuf, font_getHostFontSize(font),
            (canvas_isObjectSelected(x->box_view,
                &x->box_view->gl_obj.te_g)? "blue" : "black"));
    }
    else if (action == BOX_UPDATE)
    {
        sys_vGui("::ui_object::setText .x%lx.c %s {%.*s}\n",
            canvas, x->box_tag, outchars_b, tempbuf);
        if (pixwide != x->box_width || pixhigh != x->box_height) 
            text_drawborder(x->box_owner, x->box_view, x->box_tag,
                pixwide, pixhigh, 0);
        if (x->box_isActive)
        {
            if (selend_b > selstart_b)
            {
                sys_vGui(".x%lx.c select from %s %d\n", canvas, 
                    x->box_tag, u8_charnum(x->box_utf8, selstart_b));
                sys_vGui(".x%lx.c select to %s %d\n", canvas, 
                    x->box_tag, u8_charnum(x->box_utf8, selend_b) - 1);
                sys_vGui(".x%lx.c focus \"\"\n", canvas);        
            }
            else
            {
                sys_vGui(".x%lx.c select clear\n", canvas);
                sys_vGui(".x%lx.c icursor %s %d\n", canvas, x->box_tag,
                    u8_charnum(x->box_utf8, selstart_b));
                sys_vGui(".x%lx.c focus %s\n", canvas, x->box_tag);        
            }
        }
    }
    x->box_width = pixwide;
    x->box_height = pixhigh;
    
    *widthp = pixwide;
    *heightp = pixhigh;
    if (tempbuf != smallbuf)
        PD_MEMORY_FREE(tempbuf);
}

void rtext_retext(t_boxtext *x)
{
    int w = 0, h = 0, indx;
    t_object *text = x->box_owner;
    PD_MEMORY_FREE(x->box_utf8);
    buffer_toStringUnzeroed(text->te_buffer, &x->box_utf8, &x->box_utf8Size);
        /* special case: for number boxes, try to pare the number down
        to the specified width of the box. */
    if (text->te_width > 0 && text->te_type == TYPE_ATOM &&
        x->box_utf8Size > text->te_width)
    {
        t_atom *atomp = buffer_atoms(text->te_buffer);
        int natom = buffer_size(text->te_buffer);
        int bufsize = x->box_utf8Size;
        if (natom == 1 && atomp->a_type == A_FLOAT)
        {
                /* try to reduce size by dropping decimal digits */
            int wantreduce = bufsize - text->te_width;
            char *decimal = 0, *nextchar, *ebuf = x->box_utf8 + bufsize,
                *s1, *s2;
            int ndecimals;
            for (decimal = x->box_utf8; decimal < ebuf; decimal++)
                if (*decimal == '.')
                    break;
            if (decimal >= ebuf)
                goto giveup;
            for (nextchar = decimal + 1; nextchar < ebuf; nextchar++)
                if (*nextchar < '0' || *nextchar > '9')
                    break;
            if (nextchar - decimal - 1 < wantreduce)
                goto giveup;
            for (s1 = nextchar - wantreduce, s2 = s1 + wantreduce;
                s2 < ebuf; s1++, s2++)
                    *s1 = *s2;
            x->box_utf8 = PD_MEMORY_RESIZE(x->box_utf8, bufsize, text->te_width);
            bufsize = text->te_width;
            goto done;
        giveup:
                /* give up and bash it to "+" or "-" */
            x->box_utf8[0] = (atomp->a_w.w_float < 0 ? '-' : '+');
            x->box_utf8 = PD_MEMORY_RESIZE(x->box_utf8, bufsize, 1);
            bufsize = 1;
        }
        else if (bufsize > text->te_width)
        {
            x->box_utf8[text->te_width - 1] = '>';
            x->box_utf8 = PD_MEMORY_RESIZE(x->box_utf8, bufsize, text->te_width);
            bufsize = text->te_width;
        }
    done:
        x->box_utf8Size = bufsize;
    }
    rtext_senditup(x, BOX_UPDATE, &w, &h, &indx);
}

/* find the rtext that goes with a text item */
t_boxtext *glist_findrtext(t_glist *gl, t_object *who)
{
    t_boxtext *x;
    if (!gl->gl_editor)
        canvas_createEditorIfNone(gl);
    for (x = gl->gl_editor->e_text; x && x->box_owner != who; x = x->box_next)
        ;
    if (!x) { PD_BUG; }
    return (x);
}

int rtext_width(t_boxtext *x)
{
    int w = 0, h = 0, indx;
    rtext_senditup(x, BOX_CHECK, &w, &h, &indx);
    return (w);
}

int rtext_height(t_boxtext *x)
{
    int w = 0, h = 0, indx;
    rtext_senditup(x, BOX_CHECK, &w, &h, &indx);
    return (h);
}

void rtext_draw(t_boxtext *x)
{
    int w = 0, h = 0, indx;
    rtext_senditup(x, BOX_FIRST, &w, &h, &indx);
}

void rtext_erase(t_boxtext *x)
{
    sys_vGui(".x%lx.c delete %s\n", canvas_getView(x->box_view), x->box_tag);
}

void rtext_displace(t_boxtext *x, int dx, int dy)
{
    sys_vGui(".x%lx.c move %s %d %d\n", canvas_getView(x->box_view), 
        x->box_tag, dx, dy);
}

void rtext_select(t_boxtext *x, int state)
{
    t_glist *glist = x->box_view;
    t_glist *canvas = canvas_getView(glist);
    sys_vGui(".x%lx.c itemconfigure %s -fill %s\n", canvas, 
        x->box_tag, (state? "blue" : "black"));
}

void rtext_activate(t_boxtext *x, int state)
{
    int w = 0, h = 0, indx;
    t_glist *glist = x->box_view;
    t_glist *canvas = canvas_getView(glist);
    if (state)
    {
        sys_vGui("::ui_object::setEditing .x%lx %s 1\n", canvas, x->box_tag);
        glist->gl_editor->e_selectedText = x;
        glist->gl_editor->e_isTextDirty = 0;
        x->box_draggedFrom = x->box_selectionStart = 0;
        x->box_selectionEnd = x->box_utf8Size;
        x->box_isActive = 1;
    }
    else
    {
        sys_vGui("::ui_object::setEditing .x%lx {} 0\n", canvas);
        if (glist->gl_editor->e_selectedText == x)
            glist->gl_editor->e_selectedText = 0;
        x->box_isActive = 0;
    }
    rtext_senditup(x, BOX_UPDATE, &w, &h, &indx);
}

void rtext_key(t_boxtext *x, int keynum, t_symbol *keysym)
{
    int w = 0, h = 0, indx, i, newsize, ndel;
    char *s1, *s2;
    if (keynum)
    {
        int n = keynum;
        if (n == '\r') n = '\n';
        if (n == '\b')  /* backspace */
        {
                    /* LATER delete the box if all text is selected...
                    this causes reentrancy problems now. */
            /* if ((!x->box_selectionStart) && (x->box_selectionEnd == x->box_utf8Size))
            {
                ....
            } */
            if (x->box_selectionStart && (x->box_selectionStart == x->box_selectionEnd))
                u8_dec(x->box_utf8, &x->box_selectionStart);
        }
        else if (n == 127)      /* delete */
        {
            if (x->box_selectionEnd < x->box_utf8Size && (x->box_selectionStart == x->box_selectionEnd))
                u8_inc(x->box_utf8, &x->box_selectionEnd);
        }
        
        ndel = x->box_selectionEnd - x->box_selectionStart;
        for (i = x->box_selectionEnd; i < x->box_utf8Size; i++)
            x->box_utf8[i- ndel] = x->box_utf8[i];
        newsize = x->box_utf8Size - ndel;
        x->box_utf8 = PD_MEMORY_RESIZE(x->box_utf8, x->box_utf8Size, newsize);
        x->box_utf8Size = newsize;

/* at Guenter's suggestion, use 'n>31' to test wither a character might
be printable in whatever 8-bit character set we find ourselves. */

/*-- moo:
  ... but test with "<" rather than "!=" in order to accomodate unicode
  codepoints for n (which we get since Tk is sending the "%A" substitution
  for bind <Key>), effectively reducing the coverage of this clause to 7
  bits.  Case n>127 is covered by the next clause.
*/
        if (n == '\n' || (n > 31 && n < 127))
        {
            newsize = x->box_utf8Size+1;
            x->box_utf8 = PD_MEMORY_RESIZE(x->box_utf8, x->box_utf8Size, newsize);
            for (i = x->box_utf8Size; i > x->box_selectionStart; i--)
                x->box_utf8[i] = x->box_utf8[i-1];
            x->box_utf8[x->box_selectionStart] = n;
            x->box_utf8Size = newsize;
            x->box_selectionStart = x->box_selectionStart + 1;
        }
        /*--moo: check for unicode codepoints beyond 7-bit ASCII --*/
        else if (n > 127)
        {
            int ch_nbytes = u8_wc_nbytes(n);
            newsize = x->box_utf8Size + ch_nbytes;
            x->box_utf8 = PD_MEMORY_RESIZE(x->box_utf8, x->box_utf8Size, newsize);
            for (i = newsize-1; i > x->box_selectionStart; i--)
                x->box_utf8[i] = x->box_utf8[i-ch_nbytes];
            x->box_utf8Size = newsize;
            /*-- moo: assume canvas_key() has encoded keysym as UTF-8 */
            strncpy(x->box_utf8+x->box_selectionStart, keysym->s_name, ch_nbytes);
            x->box_selectionStart = x->box_selectionStart + ch_nbytes;
        }
        x->box_selectionEnd = x->box_selectionStart;
        x->box_view->gl_editor->e_isTextDirty = 1;
    }
    else if (!strcmp(keysym->s_name, "Right"))
    {
        if (x->box_selectionEnd == x->box_selectionStart && x->box_selectionStart < x->box_utf8Size)
        {
            u8_inc(x->box_utf8, &x->box_selectionStart);
            x->box_selectionEnd = x->box_selectionStart;
        }
        else
            x->box_selectionStart = x->box_selectionEnd;
    }
    else if (!strcmp(keysym->s_name, "Left"))
    {
        if (x->box_selectionEnd == x->box_selectionStart && x->box_selectionStart > 0)
        {
            u8_dec(x->box_utf8, &x->box_selectionStart);
            x->box_selectionEnd = x->box_selectionStart;
        }
        else
            x->box_selectionEnd = x->box_selectionStart;
    }
        /* this should be improved...  life's too short */
    else if (!strcmp(keysym->s_name, "Up"))
    {
        if (x->box_selectionStart)
            u8_dec(x->box_utf8, &x->box_selectionStart);
        while (x->box_selectionStart > 0 && x->box_utf8[x->box_selectionStart] != '\n')
            u8_dec(x->box_utf8, &x->box_selectionStart);
        x->box_selectionEnd = x->box_selectionStart;
    }
    else if (!strcmp(keysym->s_name, "Down"))
    {
        while (x->box_selectionEnd < x->box_utf8Size &&
            x->box_utf8[x->box_selectionEnd] != '\n')
            u8_inc(x->box_utf8, &x->box_selectionEnd);
        if (x->box_selectionEnd < x->box_utf8Size)
            u8_inc(x->box_utf8, &x->box_selectionEnd);
        x->box_selectionStart = x->box_selectionEnd;
    }
    rtext_senditup(x, BOX_UPDATE, &w, &h, &indx);
}

void rtext_mouse(t_boxtext *x, int xval, int yval, int flag)
{
    int w = xval, h = yval, indx;
    rtext_senditup(x, BOX_CHECK, &w, &h, &indx);
    if (flag == BOX_TEXT_DOWN)
    {
        x->box_draggedFrom = x->box_selectionStart = x->box_selectionEnd = indx;
    }
    else if (flag == BOX_TEXT_DOUBLE)
    {
        int whereseparator, newseparator;
        x->box_draggedFrom = -1;
        whereseparator = 0;
        if ((newseparator = lastone(x->box_utf8, ' ', indx)) > whereseparator)
            whereseparator = newseparator+1;
        if ((newseparator = lastone(x->box_utf8, '\n', indx)) > whereseparator)
            whereseparator = newseparator+1;
        if ((newseparator = lastone(x->box_utf8, ';', indx)) > whereseparator)
            whereseparator = newseparator+1;
        if ((newseparator = lastone(x->box_utf8, ',', indx)) > whereseparator)
            whereseparator = newseparator+1;
        x->box_selectionStart = whereseparator;
        
        whereseparator = x->box_utf8Size - indx;
        if ((newseparator =
            firstone(x->box_utf8+indx, ' ', x->box_utf8Size - indx)) >= 0 &&
                newseparator < whereseparator)
                    whereseparator = newseparator;
        if ((newseparator =
            firstone(x->box_utf8+indx, '\n', x->box_utf8Size - indx)) >= 0 &&
                newseparator < whereseparator)
                    whereseparator = newseparator;
        if ((newseparator =
            firstone(x->box_utf8+indx, ';', x->box_utf8Size - indx)) >= 0 &&
                newseparator < whereseparator)
                    whereseparator = newseparator;
        if ((newseparator =
            firstone(x->box_utf8+indx, ',', x->box_utf8Size - indx)) >= 0 &&
                newseparator < whereseparator)
                    whereseparator = newseparator;
        x->box_selectionEnd = indx + whereseparator;
    }
    else if (flag == BOX_TEXT_SHIFT)
    {
        if (indx * 2 > x->box_selectionStart + x->box_selectionEnd)
            x->box_draggedFrom = x->box_selectionStart, x->box_selectionEnd = indx;
        else
            x->box_draggedFrom = x->box_selectionEnd, x->box_selectionStart = indx;
    }
    else if (flag == BOX_TEXT_DRAG)
    {
        if (x->box_draggedFrom < 0)
            return;
        x->box_selectionStart = (x->box_draggedFrom < indx ? x->box_draggedFrom : indx);
        x->box_selectionEnd = (x->box_draggedFrom > indx ? x->box_draggedFrom : indx);
    }
    rtext_senditup(x, BOX_UPDATE, &w, &h, &indx);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
