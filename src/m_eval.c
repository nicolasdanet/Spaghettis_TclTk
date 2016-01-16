
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"
#include "m_alloca.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd pd_objectMaker;
extern t_pd pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define BUFFER_WRITE_SIZE   4096

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_symbol *buffer_getObject (t_atom *v, int argc, t_atom *argv)
{   
    if (IS_DOLLARSYMBOL (v)) { return dollar_substituteDollarSymbol (GET_DOLLARSYMBOL (v), argc, argv); }
    else if (IS_DOLLAR  (v)) {
        t_symbol *s = atom_getSymbolAtIndex (GET_DOLLAR (v) - 1, argc, argv); 
        return (s == &s_ ? NULL : s);
    }

    return atom_getSymbol (v);
}

static int buffer_getMessage (t_atom *v, t_pd *object, t_pd **next, t_atom *m, int argc, t_atom *argv)
{
    t_symbol *s = NULL;
    int end = 0;
    
    switch (v->a_type) {
    //
    case A_SEMICOLON    :   if (object == &pd_objectMaker) { SET_SYMBOL (m, gensym (";")); }
                            else { 
                                *next = NULL; end = 1; 
                            }
                            break;
    case A_COMMA        :   if (object == &pd_objectMaker) { SET_SYMBOL (m, gensym (",")); }
                            else { 
                                end = 1; 
                            }
                            break;
    case A_FLOAT        :   *m = *v; break;
    case A_SYMBOL       :   *m = *v; break;
    case A_DOLLAR       :   dollar_substituteDollarNumber (v, m, argc, argv); break;
    case A_DOLLARSYMBOL :   s = dollar_substituteDollarSymbol (GET_DOLLARSYMBOL (v), argc, argv);
                            if (s) { SET_SYMBOL (m, s); }
                            else {
                                SET_SYMBOL (m, GET_DOLLARSYMBOL (v));
                            }
                            break;
    default             :   end = 1; PD_BUG; 
    //
    }
    
    return end;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_eval (t_buffer *x, t_pd *object, int argc, t_atom *argv)
{
    int size = x->b_size;
    t_atom *v = x->b_vector;
    t_atom *message = NULL;
    t_atom *m = NULL;
    t_pd *next = NULL;
    int args = 0;
    
    ATOMS_ALLOCA (message, x->b_size);
    
    while (1) {
    //
    while (!object) {  
         
        t_symbol *s = NULL;
        
        while (size && (IS_SEMICOLON (v) || IS_COMMA (v))) { size--; v++; }
        
        if (size) { s = buffer_getObject (v, argc, argv); }
        else {
            break;
        }
        
        if (s == NULL || !(object = s->s_thing)) {
            if (s) { post_error (PD_TRANSLATE ("%s: no such object"), s->s_name); }     // --
            else {
                PD_BUG;
            }
        } else {
            size--; v++; break;
        }
    }
    
    m    = message; 
    args = 0;
    next = object;
        
    while (1) {
        if (!size || buffer_getMessage (v, object, &next, m, argc, argv)) { break; }
        else {
            m++; args++; size--; v++;
        }
    }
    
    if (args) {
        if (IS_SYMBOL (message)) { pd_message (object, GET_SYMBOL (message), args - 1, message + 1); }
        else if (IS_FLOAT (message)) {
            if (args == 1) { pd_float (object, GET_FLOAT (message)); }
            else { 
                pd_list (object, args, message); 
            }
        }
    }
    
    if (!size) { break; }
    
    object = next;
    size--;
    v++;
    //
    }
    
    ATOMS_FREEA (message, x->b_size);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error buffer_withFile (t_buffer *x, char *name, char *directory)
{
    t_error err = PD_ERROR;
    
    char filename[PD_STRING] = { 0 };

    if (!(err = path_withNameAndDirectory (filename, PD_STRING, name, directory))) {
    //
    int f = sys_open (filename, 0);
    
    err = (f < 0);
    
    if (err) { PD_BUG; }
    else {
    //
    off_t length;
    
    err |= ((length = lseek (f, 0, SEEK_END)) < 0);
    err |= (lseek (f, 0, SEEK_SET) < 0); 
    
    if (err) { PD_BUG; }
    else {
        char *t = PD_MEMORY_GET ((size_t)length + 1);
        err = (read (f, t, length) != length);
        PD_ASSERT (t[length] == 0);
        if (err) { PD_BUG; } else { buffer_withString (x, t, length); }
        PD_MEMORY_FREE (t, length + 1);
    }
    
    close (f);
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error buffer_read (t_buffer *x, char *name, t_canvas *canvas)
{
    t_error err = PD_ERROR;
    
    char *filename = NULL;
    char directory[PD_STRING] = { 0 };
    
    int f = canvas_open (canvas, name, "", directory, &filename, PD_STRING, 0);
    
    err = (f < 0);
    
    if (err) { post_error (PD_TRANSLATE ("%s: can't open"), name); }
    else {
        close (f);
        err = buffer_withFile (x, filename, directory);
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error buffer_write (t_buffer *x, char *name, char *directory)
{
    t_error err = PD_ERROR;

    char filename[PD_STRING] = { 0 };

    if (!(err = path_withNameAndDirectory (filename, PD_STRING, name, directory))) {
    //
    FILE *f = 0;

    err = !(f = sys_fopen (filename, "w"));
    
    if (!err) {
    //
    char buf[BUFFER_WRITE_SIZE] = { 0 };
    char *ptr = buf;
    char *end = buf + BUFFER_WRITE_SIZE;
    int i;
        
    for (i = 0; i < x->b_size; i++) {
    //
    t_atom *a = x->b_vector + i;
    
    if (end - ptr < (BUFFER_WRITE_SIZE / 2)) { err |= (fwrite (buf, ptr - buf, 1, f) < 1); ptr = buf; }
    
    if ((IS_SEMICOLON (a) || IS_COMMA (a)) && (ptr > buf) && (*(ptr - 1) == ' ')) { ptr--; }
    
    err |= atom_toString (a, ptr, end - ptr - 2);
    ptr += strlen (ptr);
    
    if (IS_SEMICOLON (a)) { *ptr++ = '\n'; }
    else {
        *ptr++ = ' ';
    }
    //
    }
    
    err |= (fwrite (buf, ptr - buf, 1, f) < 1);
    err |= (fflush (f) != 0) ;

    PD_ASSERT (!err);
    
    fclose (f);
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* LATER make this evaluate the file on-the-fly. */
/* LATER figure out how to log errors */
void binbuf_evalfile(t_symbol *name, t_symbol *dir)
{
    t_buffer *b = buffer_new();
    /*int import = !strcmp(name->s_name + strlen(name->s_name) - 4, ".pat") ||
        !strcmp(name->s_name + strlen(name->s_name) - 4, ".mxt");*/
    int dspstate = canvas_suspend_dsp();
        /* set filename so that new canvases can pick them up */
    glob_setfilename(0, name, dir);
    if (buffer_withFile(b, name->s_name, dir->s_name))
        post_error ("%s: read failed; %s", name->s_name, strerror(errno));
    else
    {
            /* save bindings of symbols #N, #A (and restore afterward) */
        t_pd *bounda = gensym("#A")->s_thing, *boundn = s__N.s_thing;
        gensym("#A")->s_thing = 0;
        s__N.s_thing = &pd_canvasMaker;
        /*if (import)
        {
            t_buffer *newb = binbuf_convert(b, 1);
            buffer_free(b);
            b = newb;
        }*/
        buffer_eval(b, 0, 0, 0);
        gensym("#A")->s_thing = bounda;
        s__N.s_thing = boundn;
    }
    glob_setfilename(0, &s_, &s_);
    buffer_free(b);
    canvas_resume_dsp(dspstate);
}

void global_open(void *dummy, t_symbol *name, t_symbol *dir)
{
    t_pd *x = 0;
        /* even though binbuf_evalfile appears to take care of dspstate,
        we have to do it again here, because canvas_startdsp() assumes
        that all toplevel canvases are visible.  LATER check if this
        is still necessary -- probably not. */

    int dspstate = canvas_suspend_dsp();
    t_pd *boundx = s__X.s_thing;
        s__X.s_thing = 0;       /* don't save #X; we'll need to leave it bound
                                for the caller to grab it. */
    binbuf_evalfile(name, dir);
    while ((x != s__X.s_thing) && s__X.s_thing) 
    {
        x = s__X.s_thing;
        pd_vMessage(x, gensym("pop"), "i", 1);
    }
    pd_performLoadbang();
    canvas_resume_dsp(dspstate);
    s__X.s_thing = boundx;
}

    /* save a text object to a binbuf for a file or copy buf */
void binbuf_savetext(t_buffer *bfrom, t_buffer *bto)
{
    int k, n = buffer_size(bfrom);
    t_atom *ap = buffer_atoms(bfrom), at;
    for (k = 0; k < n; k++)
    {
        if (ap[k].a_type == A_FLOAT ||
            ap[k].a_type == A_SYMBOL &&
                !strchr(ap[k].a_w.w_symbol->s_name, ';') &&
                !strchr(ap[k].a_w.w_symbol->s_name, ',') &&
                !strchr(ap[k].a_w.w_symbol->s_name, '$'))
                    buffer_append(bto, 1, &ap[k]);
        else
        {
            char buf[PD_STRING+1];
            atom_toString(&ap[k], buf, PD_STRING);
            SET_SYMBOL(&at, gensym(buf));
            buffer_append(bto, 1, &at);
        }
    }
    buffer_appendSemicolon(bto);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
