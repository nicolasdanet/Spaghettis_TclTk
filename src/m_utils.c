
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error utils_version (char *dest, size_t size)
{
    t_error err = string_sprintf (dest, size, "%s %s / %s / %s", 
                    PD_NAME, 
                    PD_VERSION, 
                    midi_nameNative(), 
                    audio_nameNative());
    
    #if PD_32BIT
        err |= string_add (dest, size, " / 32-bit");
    #endif
    
    #if PD_64BIT
        err |= string_add (dest, size, " / 64-bit");
    #endif
    
    #if PD_WITH_DEBUG
        err |= string_add (dest, size, " / DEBUG");
    #endif

    return err;
}

t_unique utils_unique (void)
{
    static t_unique unique = 10000;     /* Global. */
    
    unique++;
    
    return (unique == 0 ? (++unique) : unique);        
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void utils_anythingToList (t_pd *x, t_listmethod fn, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *t = NULL;
    PD_ATOMS_ALLOCA (t, argc + 1);
    atom_copyAtoms (argv, argc, t + 1, argc);
    SET_SYMBOL (t, s);
    (*fn) (x, &s_anything, argc + 1, t);
    PD_ATOMS_FREEA (t, argc + 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *utils_getUnusedBindName (t_class *c, t_symbol *prefix)
{
    int i = 1;
    char t[PD_STRING] = { 0 };
    
    PD_ASSERT (prefix);
    
    while (1) {
        t_error err = string_sprintf (t, PD_STRING, "%s%d", prefix->s_name, i);
        PD_ABORT (err != PD_ERROR_NONE);
        t_symbol *name = gensym (t);
        if (!pd_getThingByClass (name, c)) { return name; }
        i++;
        PD_ABORT (i < 0);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol *utils_getFirstAtomOfObject (t_object *x)
{
    return utils_getFirstAtomOfBuffer (object_getBuffer (x));
}

t_symbol *utils_getFirstAtomOfBuffer (t_buffer *x)
{
    if (x != NULL) {
    //
    int argc = buffer_getSize (x);
    t_atom *argv = buffer_getAtoms (x);
    if (argc && IS_SYMBOL (argv)) { return GET_SYMBOL (argv); }
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int utils_isNameAllowedForWindow (t_symbol *s)
{
    if (s == sym_Array)         { return 0; }
    else if (s == sym_Patch)    { return 0; }
    else if (s == sym_Scalar)   { return 0; }
    else if (s == sym_Text)     { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
