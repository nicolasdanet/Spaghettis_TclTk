
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "m_alloca.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd pd_objectMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error buffer_fromFile (t_buffer *x, char *name, char *directory)
{
    t_error err = PD_ERROR;
    
    char filepath[PD_STRING] = { 0 };

    if (!(err = path_withDirectoryAndName (filepath, PD_STRING, directory, name, 0))) {
    //
    int f = file_openRaw (filepath, O_RDONLY);
    
    err = (f < 0);
    
    if (err) { PD_BUG; }
    else {
    //
    off_t length;
    
    err |= ((length = lseek (f, 0, SEEK_END)) < 0);
    err |= (lseek (f, 0, SEEK_SET) < 0); 
    
    if (err) { PD_BUG; }
    else {
        char *t = (char *)PD_MEMORY_GET ((size_t)length);
        err = (read (f, t, (size_t)length) != length);
        if (err) { PD_BUG; } else { buffer_withStringUnzeroed (x, t, (int)length); }
        PD_MEMORY_FREE (t);
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

static t_symbol *buffer_evalGetObject (t_atom *v, int argc, t_atom *argv)
{   
    if (IS_DOLLARSYMBOL (v)) { return dollar_expandDollarSymbol (GET_SYMBOL (v), argc, argv, NULL); }
    else if (IS_DOLLAR  (v)) {
        t_symbol *s = atom_getSymbolAtIndex (GET_DOLLAR (v) - 1, argc, argv); 
        return (s == &s_ ? NULL : s);
    }

    return atom_getSymbol (v);
}

static int buffer_evalGetMessage (t_atom *v, t_pd *object, t_pd **next, t_atom *m, int argc, t_atom *argv)
{
    t_symbol *s = NULL;
    int end = 0;
    
    switch (atom_getType (v)) {
    //
    case A_SEMICOLON    :   if (object == &pd_objectMaker) { SET_SYMBOL (m, sym___semicolon__); }
                            else { 
                                *next = NULL; end = 1; 
                            }
                            break;
    case A_COMMA        :   if (object == &pd_objectMaker) { SET_SYMBOL (m, sym___comma__); }
                            else { 
                                end = 1; 
                            }
                            break;
    case A_FLOAT        :   *m = *v; break;
    case A_SYMBOL       :   *m = *v; break;
    case A_DOLLAR       :   dollar_expandDollarNumber (v, m, argc, argv, NULL); break;
    case A_DOLLARSYMBOL :   s = dollar_expandDollarSymbol (GET_SYMBOL (v), argc, argv, NULL);
                            if (s) { SET_SYMBOL (m, s); }
                            else {
                                SET_SYMBOL (m, GET_SYMBOL (v));
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
    
    pd_setBoundA (NULL);
    
    ATOMS_ALLOCA (message, x->b_size);
    
    while (1) {
    //
    while (!object) {  
         
        t_symbol *s = NULL;
        
        while (size && IS_SEMICOLON_OR_COMMA (v)) { size--; v++; }
        
        if (size) { s = buffer_evalGetObject (v, argc, argv); }
        else {
            break;
        }
        
        if (s == NULL || !(object = pd_getThing (s))) {
            if (!s) { error_invalid (&s_, sym_expansion); }
            else { 
                pd_isThing (s); 
            }
            do { size--; v++; } while (size && !IS_SEMICOLON (v));
            
        } else {
            size--; v++; break;
        }
    }
    
    PD_ASSERT ((object != NULL) || (size == 0));
    
    m    = message; 
    args = 0;
    next = object;
        
    while (1) {
        if (!size || buffer_evalGetMessage (v, object, &next, m, argc, argv)) { break; }
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

t_error buffer_read (t_buffer *x, t_symbol *name, t_glist *glist)
{
    t_error err = PD_ERROR;
    
    char *filepath = NULL;
    char directory[PD_STRING] = { 0 };
    
    int f = canvas_openFile (glist, name->s_name, "", directory, &filepath, PD_STRING);
    
    err = (f < 0);
    
    if (err) { error_canNotOpen (name); }
    else {
        close (f);
        err = buffer_fromFile (x, filepath, directory);
    }
    
    return err;
}

t_error buffer_write (t_buffer *x, char *name, char *directory)
{
    t_error err = PD_ERROR;

    char filepath[PD_STRING] = { 0 };

    if (!(err = path_withDirectoryAndName (filepath, PD_STRING, directory, name, 0))) {
    //
    FILE *f = 0;

    err = !(f = file_openWrite (filepath));
    
    if (!err) {
    //
    char *s = NULL;
    int size = 0;
    
    buffer_toStringUnzeroed (x, &s, &size);

    err |= (fwrite (s, size, 1, f) < 1);
    err |= (fflush (f) != 0);

    PD_ASSERT (!err);
    PD_MEMORY_FREE (s);
        
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

t_error buffer_fileEval (t_symbol *name, t_symbol *directory)
{
    t_error err = PD_ERROR;
    
    int state = dsp_suspend();
    t_buffer *t = buffer_new();
        
    canvas_setActiveFileNameAndDirectory (name, directory);
    
    err = buffer_fromFile (t, name->s_name, directory->s_name);
    
    if (err) { error_failsToRead (name); }
    else {
    //
    buffer_eval (t, NULL, 0, NULL);
    //
    }
    
    canvas_setActiveFileNameAndDirectory (&s_, &s_);
    
    buffer_free (t);
    dsp_resume (state);
    
    return err;
}

void buffer_fileOpen (void *dummy, t_symbol *name, t_symbol *directory)
{
    t_pd *x = NULL;
    t_pd *boundX = pd_getBoundX(); int state = dsp_suspend();
    
    pd_setBoundX (NULL);
    
    buffer_fileEval (name, directory);
    
    while ((x != pd_getBoundX()) && pd_getBoundX()) { x = pd_getBoundX(); pd_vMessage (x, sym__pop, "i", 1); }
    
    stack_performLoadbang();
    
    dsp_resume (state); pd_setBoundX (boundX);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
