
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_symbol s_pointer  = { "pointer"   , NULL, NULL };         /* Shared. */
t_symbol s_float    = { "float"     , NULL, NULL };         /* Shared. */
t_symbol s_symbol   = { "symbol"    , NULL, NULL };         /* Shared. */
t_symbol s_bang     = { "bang"      , NULL, NULL };         /* Shared. */
t_symbol s_list     = { "list"      , NULL, NULL };         /* Shared. */
t_symbol s_anything = { "anything"  , NULL, NULL };         /* Shared. */
t_symbol s_signal   = { "signal"    , NULL, NULL };         /* Shared. */
t_symbol s__N       = { "#N"        , NULL, NULL };         /* Shared. */
t_symbol s__X       = { "#X"        , NULL, NULL };         /* Shared. */
t_symbol s__A       = { "#A"        , NULL, NULL };         /* Shared. */
t_symbol s_         = { ""          , NULL, NULL };         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
