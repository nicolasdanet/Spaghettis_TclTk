
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_error_h_
#define __m_error_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error__error1  (char *);
void error__error2  (char *, char *);
void error__post    (int argc, t_atom *argv);
void error__options (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_stackOverflow                    (void);
void error_ioStuck                          (void);
void error_stubNotFound                     (void);
void error_tooManyCharacters                (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_recursiveInstantiation           (t_symbol *);
void error_sendReceiveLoop                  (t_symbol *);
void error_canNotSetMultipleFields          (t_symbol *);
void error_alreadyExists                    (t_symbol *);
void error_canNotOpen                       (t_symbol *);
void error_canNotCreate                     (t_symbol *);
void error_failsToRead                      (t_symbol *);
void error_failsToWrite                     (t_symbol *);
void error_ignored                          (t_symbol *);
void error_failed                           (t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_noSuch                           (t_symbol *, t_symbol *);
void error_canNotFind                       (t_symbol *, t_symbol *);
void error_unknownMethod                    (t_symbol *, t_symbol *);
void error_missingField                     (t_symbol *, t_symbol *);
void error_unexpected                       (t_symbol *, t_symbol *);
void error_invalid                          (t_symbol *, t_symbol *);
void error_mismatch                         (t_symbol *, t_symbol *);
void error_unspecified                      (t_symbol *, t_symbol *);
void error_undefined                        (t_symbol *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_canNotMake                       (int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void error_invalidArguments                 (t_symbol *, int argc, t_atom *argv);
void error_invalidArgumentsForMethod        (t_symbol *, t_symbol *, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void warning_badName                        (t_symbol *, t_symbol *);
void warning_unusedOption                   (t_symbol *, t_symbol *);
void warning_unusedArguments                (t_symbol *, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_error_h_
