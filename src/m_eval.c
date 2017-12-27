
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error buffer_fromFile (t_buffer *, const char *, const char *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_symbol *eval_bufferGetObject (t_atom *v, int argc, t_atom *argv)
{   
    if (IS_DOLLARSYMBOL (v))   {
        return dollar_expandSymbolWithArguments (GET_SYMBOL (v), NULL, argc, argv);
    } else if (IS_DOLLAR  (v)) {
        t_symbol *s = atom_getSymbolAtIndex (GET_DOLLAR (v) - 1, argc, argv); 
        return (s == &s_ ? NULL : s);
    }

    return atom_getSymbol (v);
}

static int eval_bufferGetMessage (t_atom *v, t_pd *object, t_pd **next, t_atom *m, int argc, t_atom *argv)
{
    t_symbol *s = NULL;
    int end = 0;
    
    switch (atom_getType (v)) {
    //
    case A_SEMICOLON    :   if (instance_isMakerObject (object)) { SET_SYMBOL (m, sym___semicolon__); }
                            else { 
                                *next = NULL; end = 1; 
                            }
                            break;
    case A_COMMA        :   if (instance_isMakerObject (object)) { SET_SYMBOL (m, sym___comma__); }
                            else { 
                                end = 1; 
                            }
                            break;
    case A_FLOAT        :   *m = *v; break;
    case A_SYMBOL       :   *m = *v; break;
    case A_POINTER      :   *m = *v; break;
    case A_DOLLAR       :   dollar_expandWithArguments (v, m, NULL, argc, argv); break;
    case A_DOLLARSYMBOL :   s = dollar_expandSymbolWithArguments (GET_SYMBOL (v), NULL, argc, argv);
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

void eval_buffer (t_buffer *x, t_pd *object, int argc, t_atom *argv)
{
    int size = x->b_size;
    t_atom *v = x->b_vector;
    t_atom *message = NULL;
    t_atom *m = NULL;
    t_pd *next = NULL;
    int args = 0;
    
    instance_setBoundA (NULL);
    
    PD_ATOMS_ALLOCA (message, x->b_size);
    
    while (1) {
    //
    while (!object) {  
         
        t_symbol *s = NULL;
        
        while (size && IS_SEMICOLON_OR_COMMA (v)) { size--; v++; }
        
        if (size) { s = eval_bufferGetObject (v, argc, argv); }
        else {
            break;
        }
        
        if (s == NULL || !(object = symbol_getThing (s))) {
            if (!s) { error_invalid (&s_, sym_expansion); }
            else { 
                symbol_hasThing (s); 
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
        if (!size || eval_bufferGetMessage (v, object, &next, m, argc, argv)) { break; }
        else {
            m++; args++; size--; v++;
        }
    }
    
    if (args) {
        if (IS_SYMBOL (message))     { pd_message (object, GET_SYMBOL (message), args - 1, message + 1); }
        else if (IS_FLOAT (message)) {
            if (args == 1) { pd_float (object, GET_FLOAT (message)); }
            else { 
                pd_list (object, args, message); 
            }
        } else if (IS_POINTER (message)) {
            if (args == 1) { pd_pointer (object, GET_POINTER (message)); }
            else { 
                pd_list (object, args, message); 
            }
        } else {
            PD_BUG;
        }
    }
    
    if (!size) { break; }
    
    object = next;
    size--;
    v++;
    //
    }
    
    PD_ATOMS_FREEA (message, x->b_size);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error eval_fileProceed (t_symbol *name, t_symbol *directory, const char *s)
{
    t_error err = PD_ERROR_NONE;
    
    int state = dsp_suspend();
    
    t_buffer *t = buffer_new();
        
        instance_environmentSetFile (name, directory);
        
        if (s == NULL) { err = buffer_fromFile (t, name->s_name, directory->s_name); }
        else {
            buffer_withStringUnzeroed (t, s, (int)strlen (s));
        }
        
        if (err) { error_failsToRead (name); } else { eval_buffer (t, NULL, 0, NULL); }
        
        instance_environmentResetFile();
    
    buffer_free (t);
    
    dsp_resume (state);
    
    return err;
}

t_error eval_fileByString (t_symbol *name, t_symbol *directory, const char *s)
{
    return eval_fileProceed (name, directory, s);
}

t_error eval_file (t_symbol *name, t_symbol *directory)
{
    return eval_fileProceed (name, directory, NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
