
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error__error1 (char *s)
{
    error__error2 ("", s);
}

void error__error2 (char *s1, char *s2)
{
    post_error ("%s: %s", s1, s2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error__post (int argc, t_atom *argv)
{
    char *s = atom_atomsToString (argc, argv); post_error ("[ %s ]", s); PD_MEMORY_FREE (s);
}

int error__options (t_symbol *s, int argc, t_atom *argv)
{
    int i, k = 0;
    
    for (i = 0; i < argc; i++) {
    //
    if (IS_SYMBOL (argv + i)) {
        t_symbol *t = GET_SYMBOL (argv + i);
        if (t != sym___dash__ && string_containsAtStart (t->s_name, sym___dash__->s_name)) { 
            warning_unusedOption (s, t); 
            k = 1;
        }
    }
    //
    }
    
    return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_stackOverflow (void)
{
    post_error (PD_TRANSLATE (": stack overflow"));
}

void error_ioStuck (void)
{
    post_error (PD_TRANSLATE ("audio: I/O stuck"));
}

void error_stubNotFound (void)
{
    post_error (PD_TRANSLATE ("loader: stub not found"));
}

void error_tooManyCharacters (void) 
{ 
    post_error (PD_TRANSLATE ("console: too many characters"));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_recursiveInstantiation (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: recursive instantiation"), s->s_name);
}

void error_sendReceiveLoop (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: send/receive loop"), s->s_name);
}

void error_canNotSetMultipleFields (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: can't set multiple fields"), s->s_name);
}

void error_alreadyExists (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: already exists"), s->s_name);
}

void error_canNotOpen (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: can't open"), s->s_name);
}

void error_canNotCreate (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: can't create"), s->s_name);
}

void error_failsToRead (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: fails to read"), s->s_name);
}

void error_failsToWrite (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: fails to write"), s->s_name);
}

void error_ignored (t_symbol *s)
{
    post_error (PD_TRANSLATE ("%s: ignored"), s->s_name);
}

void error_failed (t_symbol *s)
{   
    post_error (PD_TRANSLATE ("%s: failed"), s->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_noSuch (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: no such %s"), s1->s_name, s2->s_name);
}

void error_canNotFind (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: can't find %s"), s1->s_name, s2->s_name);
}

void error_unknownMethod (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: unknown method %s"), s1->s_name, s2->s_name);
}

void error_missingField (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: missing field %s"), s1->s_name, s2->s_name);
}

void error_unexpected (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: unexpected %s"), s1->s_name, s2->s_name);
}

void error_invalid (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: invalid %s"), s1->s_name, s2->s_name);
}

void error_mismatch (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: mismatch %s"), s1->s_name, s2->s_name);
}

void error_unspecified (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: unspecified %s"), s1->s_name, s2->s_name);
}

void error_undefined (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: undefined %s"), s1->s_name, s2->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_canNotMake (int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
    
    post_error (PD_TRANSLATE (": can't make [ %s ]"), t);
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_invalidArguments (t_symbol *s, int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
    
    post_error (PD_TRANSLATE ("%s: invalid argument(s) [ %s ]"), s->s_name, t);
    
    PD_MEMORY_FREE (t);
}

void error_invalidArgumentsForMethod (t_symbol *s1, t_symbol *s2, int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
        
    post_error (PD_TRANSLATE ("%s: invalid argument(s) for method %s [ %s ]"), s1->s_name, s2->s_name, t);
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void warning_badName (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: bad name %s"), s1->s_name, s2->s_name);
}

void warning_badType (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: bad type %s"), s1->s_name, s2->s_name);
}

void warning_unusedOption (t_symbol *s1, t_symbol *s2)
{
    post_error (PD_TRANSLATE ("%s: unused option %s"), s1->s_name, s2->s_name);
}

void warning_unusedArguments (t_symbol *s, int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
    
    post_error (PD_TRANSLATE ("%s: unused argument(s) [ %s ]"), s->s_name, t);
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
