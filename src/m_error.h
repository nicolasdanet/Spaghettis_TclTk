
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_error_h_
#define __m_error_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error__error1                          (const char *);
void error__error2                          (const char *, const char *);
int  error__options                         (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error_dspLoop                          (void);
void error_stackOverflow                    (void);
void error_ioStuck                          (void);
void error_stubNotFound                     (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error_recursiveInstantiation           (t_symbol *);
void error_badClassName                     (t_symbol *);
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
// MARK: -

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
// MARK: -

void error_canNotMake                       (int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void error_invalidArguments                 (t_symbol *, int argc, t_atom *argv);
void error_invalidArgumentsForMethod        (t_symbol *, t_symbol *, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void warning_invalid                        (t_symbol *, t_symbol *);
void warning_badName                        (t_symbol *, t_symbol *);
void warning_badType                        (t_symbol *, t_symbol *);
void warning_unusedOption                   (t_symbol *, t_symbol *);
void warning_unusedArguments                (t_symbol *, int argc, t_atom *argv);
void warning_tooManyCharacters              (t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_error_h_
