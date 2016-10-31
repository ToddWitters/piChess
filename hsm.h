#ifndef HSM_H
#define HSM_H

/*
 *  HSM State machine engine
 *
 *  Provides an engine that will process events based upon a user-provided statemachine definition.
 *
 */
#include "stdint.h"
#include "stdbool.h"
#include "hsmUserDataType.h"

#define KEY_VAL 0xCCAA5533
// Event type
//
// The data argument can be altered to contain any payload desired.  The hsm engine does nothing with the data other
// than pass the structure through to the entry/exit/init/guard/action functions.  It should be noted that the entire
// structure is passed by value to these function, so the payload should be kept relatively small...
typedef struct event_e
{
   uint16_t       ev;
   hsmUserData_t  data;     // Data attached to the event.
}event_t;

// Typedefs for the various functions that are called

// Called upon entry to a state.  Triggering event is passed in.  If this
//   is is due to the init function traversing to a leaf node, pass a zero
typedef void   (*entryFunc_t)( event_t ev );

// Called upon exit from a state.  Triggering event is passed in.  If this is
// due to a an exit condition, pass zero
typedef void   (*exitFunc_t)( event_t ev );

// Allows a guard condition on a transition.  Triggering event is passed in, return
// value allows (or disallows) transition from happeneing.
typedef bool   (*guardFunc_t)( event_t ev );

// Executes an action associated with an internal event, or one which causes
//   a transition.  Trigger event is passed in
typedef void   (*actionFunc_t)( event_t ev );

// Called when the target of a transition is a container state.
// Returns a substate to navigate to.  Original
// triggering event is passed in
// Also used to initialize the first transition into the state machine , in which case, passed event is EV_NULL
typedef uint16_t (*initFunc_t)(event_t ev);

// Null definition for the various pointers
#define NULL_ENTRY_FUNC   (entryFunc_t)(0)
#define NULL_EXIT_FUNC     (exitFunc_t)(0)
#define NULL_GUARD_FUNC   (guardFunc_t)(0)
#define NULL_ACTION_FUNC (actionFunc_t)(0)
#define NULL_INIT_FUNC     (initFunc_t)(0)

// User definition of a state. This structures provides information related to an individual state
typedef struct stateDef_s
{
   uint16_t             parent;        // The parent state.  Use ST_COUNT for top state (which has no parent)
   initFunc_t           initFunc;      // Called to choose a substate when the state conatins others.  Use NULL_INIT_FUNC otherwise
   entryFunc_t          entryFunc;     // function to run on entry
   exitFunc_t           exitFunc;      // function to run on exit
}stateDef_t;

// Transition definitions. Sorted by event.  May have multiple matches
typedef struct transDef_s
 {
   uint16_t       ev;       // The event that triggers this transition
   uint16_t       from;     // The state to which this trigger applies
   guardFunc_t    guard;    // The guard condition. May be NULL_GUARD_FUNC.  If non-NULL, must return true to make transition
   actionFunc_t   action;   // The action, if any, to perform prior to the transition (i.e. prior to running the exit/entry functions)
   uint16_t       to;       // The state to transition to (NULL_STATE if handled internally and no transition needed)
   bool           local;    // When to/from are direct ancestors/decendents, choose type of transition
                            // local transitions prevent entry/exit functions of upper most state from running.
                            // Setting this to true without the states being ancestrally related is an error
}transDef_t;

// The state machine handle.
typedef struct HSM_Handle_s
{
   uint32_t            key;           // holds known pattern if handle is "created"
   const stateDef_t   *states;        // Pointer to the state definitions
   const transDef_t   *transitions;   // Pointer to the transitions definitions
   uint16_t            currentState;  // The current state
   uint16_t            transCount;    // The number of elements in the transition table
   uint16_t            stateCount;    // The number of elements in the state table
   initFunc_t          initFunc;      // The init function of the state machine
   exitFunc_t          exitFunc;      // The function that will be called on exit

}HSM_Handle_t;

typedef enum HSM_Error_e
{
   HSM_NO_ERROR,                                // SUCCESS
   HSM_NULL_POINTER,                            // Null pointer passed in one or more arguments
   HSM_EV_NOT_IN_TABLE,                         // Event doesn't appear in transition table
   HSM_TOP_PARENT_NOT_NULL,                     // top-most state has a parent (but shouldn't)
   HSM_MISSING_PARENT_ON_SUBNODE,               // some other state is missing a parent (but should have one)
   HSM_MISSING_INIT_FUNC,                       // a state with substates lacks an init function
   HSM_UNEXPECTED_INIT_FUNC,                    // a state without substates specifies and init function
   HSM_INVALID_STATE,                           // an invalid state reference was found
   HSM_EVENT_LIST_NOT_SORTED,                   // the transition list was not sorted by event
   HSM_NO_EV_HANDLER_FOUND,                     // the event has no handler in the current state
   HSM_NOT_INITIALIZED,                         // action cannot be performed because the init function has not yet been called
   HSM_NOT_CREATED,                             // action cannot be performed because the create function has not yet been called
   HSM_ALREADY_INITIALIZED,                     // init has been called with a state machine that has been init'd (and not exited)
   HSM_ALREADY_CREATED,                         // create has been called with a state machine that has already been created
   HSM_USELESS_TRANSITION,                      // transition table entry has no "to" state and no action function
   HSM_CIRCULAR_HIERARCY,                       // circular reference found in state hierarcy
   HSM_INIT_FUNC_RETURNED_INVALID_STATE,        // one of the state's init function returned a state that is out of range or not a descendent
   HSM_LOCAL_TRANS_NOT_ANCESTRALLY_RELATED,     // local flag was set on a transition between states not related ancestrally
}HSM_Error_t;

// Create a state machine, verify passed structure
//
// Arguments
//
//   states - pointer to an array of stateDef_t objects that define the states in the system.  The first state must be
//            the top-most state.  state hierarcy is checked to ensure that (a) the first state has ST_NONE specified
//            as the parent, (b) all remaining states have a parent state that is < ST_NONE, (c) traversing any node
//            upward will ultimately arrive at the top node and (d) no loops occur in the hierarchy
//   transitions - pointer to an array defining all the transitions (including internal transitions).  The list must be
//            sorted by event.  Within transitions that have the same event that originate from the same state, the
//            transitions are evaluated in the order they appear, i.e. the guard functions (if any) are evaluted in
//            sequence.
//   stateCount - the number of elements in the states array
//   transCount - the number of elements in the transitions array
//   initFunc   - the upper-level initialization function that runs when the state machine is intiailized.  It returns the desired
//            start state, which then triggers a series of entry functions until the state is reached.   If the target state is a
//            composite, the process repeats until a leaf node is reached.
//   exitFunc   - the upper-level exit function called when the state machine terminates.  May be NULL if no action needed.
//   hsm      A pointer to a pointer to an hsm handle.
//
//  Return codes are as follows:
//     HSM_NO_ERROR
//     HSM_NULL_POINTER
//     HSM_ALREADY_CREATED
//     HSM_TOP_PARENT_NOT_NULL
//     HSM_MISSING_PARENT_ON_SUBNODE
//     HSM_MISSING_INIT_FUNC
//     HSM_UNEXPECTED_INIT_FUNC
//     HSM_INVALID_STATE
//     HSM_EVENT_LIST_NOT_SORTED
//     HSM_CIRCULAR_HIERARCY
//     HSM_USELESS_TRANSITION
//     HSM_LOCAL_TRANS_NOT_ANCESTRALLY_RELATED
//
HSM_Error_t HSM_createHSM( const stateDef_t *states,       // State information
                           const transDef_t *transitions,  // State transition information
                           uint16_t stateCount,            // Size of state information table
                           uint16_t transCount,            // Size of transition information table
                           initFunc_t initFunc,            // Function used to pick initial state machine state
                           exitFunc_t exitFunc,            // Function to run when state machine is exited.
                           HSM_Handle_t *hsm);             // Pointer to state machine

// Initialize a created hsm
//
// Argument
//
//     hsm - Pointer to the state machine handle
//
//  Calls the initFunc provided by the earlier HSM_createHSM() call.  This function returns a state that should be navigated to by
//    calling all the entry functions of the containing states until it is reached.  It that state itself is a container state,
//    the process is repeated until a leaf node is reached.
//
//  Return codes are as follows:
//
//  HSM_NO_ERROR
//  HSM_NULL_POINTER
//  HSM_NOT_CREATED
//  HSM_ALREADY_INITIALIZED
//  HSM_INIT_FUNC_RETURNED_INVALID_STATE
//
HSM_Error_t HSM_init( HSM_Handle_t *hsm );

// Process a new event
//
// Uses the current state, the transitions table, and the passed event to determine a course of action...
//
// (1) Staring with the current state, see if an entry exists in the transition table which meets all of the following
//   (a) from = the state we are looking for
//   (b) ev   = the passed event
//   (c) guard() returns true OR is undefined
// (2) if a suitable transition is found...
//   (a) determine the lowest common ancestor (lca) between the state we are looking for and the ".to" state
//   (b) call the action function associated with the transistion (if it exists)
//   (c) navigate upwards towards the lca state, calling exit functions on the way up
//   (d) navigate downwards towards the target state calling entry functions on the way down
// (3) if (1) did not find a suitable transition, move to parent of current state and try looking there.
// (4) repeat (3) until top reached.
//
// Exception:  If 'local' was set, do not execute entry/exit functions of outer-most containing state.
//
// Return codes are as follows
//
//  HSM_NO_ERROR
//  HSM_NULL_POINTER
//  HSM_NOT_INITIALIZED
//  HSM_NOT_CREATED
//  HSM_INVALID_STATE
//  HSM_EV_NOT_IN_TABLE
//  HSM_NO_EV_HANDLER_FOUND
//  HSM_INIT_FUNC_RETURNED_INVALID_STATE
//
HSM_Error_t HSM_processEvent( HSM_Handle_t *hsm, event_t ev);


// Call the exit functions from the current state, walking up to the top state and calling all those as well...
// WARNING!!! Do not call this function from within any entry/exit/init/guard/action functions!!!
//
// Return codes are as follows
//
//  HSM_NO_ERROR
//  HSM_NULL_POINTER
//  HSM_NOT_INITIALIZED
//  HSM_NOT_CREATED
//  HSM_INVALID_STATE
HSM_Error_t HSM_exit( HSM_Handle_t *hsm );

// Destroys a statemachine (resets the handle to an 'uncreated' state)
// Return codes are as follows
// NOTE:  This skips the normal exit sequence.
//
//  HSM_NO_ERROR
//  HSM_NULL_POINTER
//  HSM_NOT_CREATED
HSM_Error_t HSM_destroy( HSM_Handle_t *hsm);


// Returns a printable string representing the error
char* HSM_getErrorString( HSM_Error_t err );

#endif
