
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __g_unique_h_
#define __g_unique_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     glist_objectRemoveByUnique              (t_id u);
t_error     glist_objectDisplaceByUnique            (t_id u, int deltaX, int deltaY);
t_error     glist_objectMoveAtFirstByUnique         (t_id u);
t_error     glist_objectMoveAtLastByUnique          (t_id u);
t_error     glist_objectMoveAtByUnique              (t_id u, int n);
t_error     glist_objectGetIndexOfByUnique          (t_id u, int *n);

t_error     glist_lineConnectByUnique               (t_id u, int m, t_id v, int n);
t_error     glist_lineDisconnectByUnique            (t_id u, int m, t_id v, int n);

t_error     glist_resizeBoxByUnique                 (t_id u, int n);
t_error     glist_resizeGraphByUnique               (t_id u, int deltaX, int deltaY);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_unique_h_
