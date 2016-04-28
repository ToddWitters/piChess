#include "stdint.h"
#include "types.h"

#ifndef HSM_H
#define HSM_H

// Event type
typedef struct event_e
{
   uint16_t     ev;
   int          data;
}event_t;

// Typedefs for the various functions that are called

// Called upon entry to a state.  Triggering event is passed in.  If this
//   is a entry due to a default state of a container state, pass 0
typedef void   (*entryFunc_t)( event_t ev );

// Called upon exit from a state.  Triggering event is passed in.
typedef void   (*exitFunc_t)( event_t ev );

// Allows guard condition on a transition.  Triggering event is passed in
typedef bool_t (*guardFunc_t)( event_t ev );

// Executes an action associated for an internal event, or one which causes
//   a transition.  Trigger event is passed in
typedef void   (*actionFunc_t)( event_t ev );

// Called to pick a substate when the target of a transition is a container state
typedef uint16_t (*substatePickerFunc_t)(event_t ev);

// Null definition for the various pointers
#define NULL_ENTRY_FUNC           (entryFunc_t)(0)
#define NULL_EXIT_FUNC            (exitFunc_t)(0)
#define NULL_GUARD_FUNC           (guardFunc_t)(0)
#define NULL_ACTION_FUNC          (actionFunc_t)(0)
#define NULL_SUBSTATE_PICKER_FUNC (substatePickerFunc_t)(0)

// User definition of a state. These structures are used to define the tree.
// Sorted by state enum
typedef struct stateDef_s
{
   char                *displayName;   // Optional name for debugging
   uint16_t             parent;        // The parent state.  Use TOTAL_STATES for topmost state only
   substatePickerFunc_t pickerFunc;    // Called to choose a substate when transition target is a container state
   entryFunc_t          entryFunc;     // run on entry
   exitFunc_t           exitFunc;      // run on exit
}stateDef_t;

// User definition of an event handler.
// Sorted by from state.  May have multiple matches
typedef struct transDef_s
 {
   uint16_t       ev;       // The event trigger
   uint16_t       from;     // The originating state
   uint16_t       to;       // The target state (NULL_STATE_ID if handled internally)
   guardFunc_t    guard;    // The guard condition.  If non-NULL, must return true to make transition
   actionFunc_t   action;   // The action, if any, to perform
   bool_t         local;    // When to/from are direct ancestors/decendents, choose type of transition
}transDef_t;

// Overall disposition of an HSM
typedef enum HSM_disposition_e
{
   HSM_CREATED,      // Created but not yet initialized
   HSM_INITIALIZED,  // Created and initialized (or re-initialized after exit)
   HSM_EXITED        // Exit state was reached.
}HSM_disposition_t;

typedef struct HSM_Handle_s
{
   const stateDef_t   *states;
   const transDef_t   *transitions;
   uint16_t            currentState;
   uint16_t            transCount;
   uint16_t            stateCount;
   HSM_disposition_t   hsmDisposition;

}HSM_Handle_t;

typedef enum HSM_Error_e
{
   HSM_NO_ERROR,
   HSM_NULL_POINTER,
   HSM_NULL_EVENT,
   HSM_EV_NOT_IN_TABLE,
   HSM_TOP_PARENT_NOT_NULL,
   HSM_MISSING_PARENT_ON_SUBNODE,
   HSM_MISSING_PICKER_FUNC,
   HSM_INVALID_STATE,
   HSM_EVENT_LIST_NOT_SORTED,
   HSM_NO_EV_HANDLER_FOUND,
   HSM_NOT_HANDLED,
   HSM_NOT_INITIALIZED,
   HSM_ALREADY_INITIALIZED,
   HSM_CIRCULAR_HIERARCY,
   HSM_PICKER_RETURNED_INVALID_STATE,
   HSM_PICKER_RETURNED_NON_CHILD,
   HSM_NO_COMMON_ANCESTOR_FOUND,
   HSM_OUT_OF_MEMORY,
}HSM_Error_t;

// public interface

// Create a state machine, verify passed structure
HSM_Error_t HSM_createHSM( const stateDef_t *states,
                       const transDef_t *transitions,
                       uint16_t stateCount,
                       uint16_t transCount,
                       HSM_Handle_t **hsm);

// initialize hsm (enter top state and )
HSM_Error_t HSM_init( HSM_Handle_t *hsm );

// Destroy a state machine
HSM_Error_t HSM_destroy( HSM_Handle_t *hsm);

// Process a new event and return the resulting state
HSM_Error_t HSM_processEvent( HSM_Handle_t *hsm, event_t ev);



#endif
