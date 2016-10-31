#include "hsm.h"
#include <stdint.h>
#include <string.h>

#define EVENT_NOT_FOUND 0xFFFF

// Local functions
static uint16_t findIndexForEvent(const transDef_t *table, uint16_t tableSize, uint16_t ev );
static uint16_t findKey(const transDef_t *table, uint16_t key, uint16_t min, uint16_t max);
static uint16_t findLca(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB);
static uint16_t findLineFromAToB(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB );
static HSM_Error_t traverseCompositeState(HSM_Handle_t *hsm, event_t ev );

HSM_Error_t HSM_createHSM( const stateDef_t *states,       // State information
                           const transDef_t *transitions,  // State transition information
                           uint16_t stateCount,            // Size of state informatino table
                           uint16_t transCount,            // Size of transition information table
                           initFunc_t initFunc,            // Function used for top-level initialization
                           exitFunc_t exitFunc,            // Function called after state machine exit
                           HSM_Handle_t *hsm)              // Pointer to handle
{

   uint16_t last, i, j;

   // Verify passed parameters....
   if( (NULL == states) || (NULL == transitions) || (NULL == hsm) || ( NULL_INIT_FUNC == initFunc) )
      return HSM_NULL_POINTER;

   // Make sure this state machine isn't already craeted...
   if(KEY_VAL == hsm->key)
      return HSM_ALREADY_CREATED;

   // Set to an uninitialized state first...
   memset(hsm, 0, sizeof( HSM_Handle_t ));

   /////////////////////////////
   // Validate state definitions
   /////////////////////////////

   // Make sure top most state has a null parent
   if( states[0].parent != stateCount )
      return HSM_TOP_PARENT_NOT_NULL;

   // Make sure top most state has an init function
   if(NULL_INIT_FUNC == states[0].initFunc )
      return HSM_MISSING_INIT_FUNC;

   //  Make sure every other state:
   //     (a) specifies a valid parent state
   //     (b) has a lineage that traces back to top state
   //     (c) defines (or not) an init state if it has children (or not)
   for(i = 1; i < stateCount; i++)
   {
      uint16_t tmp = i;
      uint16_t count = stateCount;

      // Move up until we find the top state
      while( (tmp != 0) && states[tmp].parent < stateCount && --count )
         tmp = states[tmp].parent;

      // If we didn't hit the top within stateCount steps, there
      //   must be a reference loop.
      if(0 == count )
         return HSM_CIRCULAR_HIERARCY;

      // If we bailed out for some other reason...
      if(tmp != 0)
      {
         if(states[tmp].parent > stateCount)
            return HSM_INVALID_STATE;

         if( states[tmp].parent == stateCount )
            return HSM_MISSING_PARENT_ON_SUBNODE;
      }

      // See if any other states name this one as parent
      for(j=1; j < stateCount; j++)
         if ( states[j].parent == i) break;

      // If so...
      if(j != stateCount)
      {
         // Make sure this one has an init function
         if ( NULL_INIT_FUNC == states[i].initFunc ) return HSM_MISSING_INIT_FUNC;
      }
      else
      {
         // No children should mean no init function
         if ( states[i].initFunc != NULL_INIT_FUNC) return HSM_UNEXPECTED_INIT_FUNC;
      }
   }

   ///////////////////////////////////
   // Validate transition definitions
   ///////////////////////////////////

   // Make sure transition list is sorted, all state references are valid, and that to/from are ancestorly related for
   //  any transitions marked as local
   //
   last = 0;
   for(i=0; i< transCount; i++)
   {
      // Make sure list is ordered
      if(transitions[i].ev < last)
         return HSM_EVENT_LIST_NOT_SORTED;

      // Make sure "to" state is valid.
      // NOTE: value of 'stateCount' is valid because it is used to denote internal transitions
      if(transitions[i].to > stateCount)
         return HSM_INVALID_STATE;

      // Make sure "from" state is valid
      if(transitions[i].from >= stateCount)
         return HSM_INVALID_STATE;

      // Internal transitions without an action function are useless....
      if(transitions[i].to == stateCount && NULL_ACTION_FUNC == transitions[i].action)
         return HSM_USELESS_TRANSITION;

      // If this transition is marked local, ensure the to/from states are ancestorly related
      if(true == transitions[i].local)
      {

         uint16_t walk;

         walk = transitions[i].from;

         // move up until we find "to" or the top of the tree
         while(walk != stateCount && walk != transitions[i].to)
            walk = states[walk].parent;

         // If we hit the top, check the opposite (i.e. if "from" is an ancestor of "to")
         if(walk == stateCount)
         {
            walk = transitions[i].to;

            // move up until we find "from" or the top of the tree
            while(walk != stateCount && walk != transitions[i].from)
               walk = states[walk].parent;

            // we hit the top again, so they are not ancestrally related..
            if(walk == stateCount)
               return HSM_LOCAL_TRANS_NOT_ANCESTRALLY_RELATED;
         }
      }
      last = transitions[i].ev;
   }

   // Everything looks good...

   // Initialize the handle
   hsm->states         = states;
   hsm->transitions    = transitions;
   hsm->transCount     = transCount;
   hsm->stateCount     = stateCount;
   hsm->currentState   = stateCount;  // This marks the state machine as created but not initialized.
   hsm->initFunc       = initFunc;
   hsm->exitFunc       = exitFunc;
   hsm->key            = KEY_VAL;

   return HSM_NO_ERROR;
}


HSM_Error_t HSM_init( HSM_Handle_t *hsm )
{
   event_t nullEvent = {0};                                                                         

   uint16_t target;

   if(NULL == hsm )
      return HSM_NULL_POINTER;

   if(hsm->key != KEY_VAL)
      return HSM_NOT_CREATED;

   // Only proceed if current state is still set to stateCount (indicates create was run, but init not run yet)
   if(hsm->currentState != hsm->stateCount)
      return HSM_ALREADY_INITIALIZED;

   // Call the state machine init function.  It will return the desired start state...
   target = hsm->initFunc(nullEvent);

   // Verify valid state returned
   if(target >= hsm->stateCount)
      return HSM_INIT_FUNC_RETURNED_INVALID_STATE;

   // Traverse downward from top to leaf node
   //   running init functions (and entry functios) on the way down...
   do
   {
      // For each init function, drill down
      do
      {
         // Find the child node that moves us towards the target
         uint16_t nextStep = findLineFromAToB(hsm, hsm->currentState, target);

         // If target not a decendent of our current state, bail out...
         if(nextStep >= hsm->stateCount)
            return HSM_INIT_FUNC_RETURNED_INVALID_STATE;

         // Move state downward,
         hsm->currentState = nextStep;

         // Call entry function if it exists...
         if(hsm->states[hsm->currentState].entryFunc != NULL_ENTRY_FUNC)
            hsm->states[hsm->currentState].entryFunc(nullEvent);

      // Repeat until we reach the target state
      } while (hsm->currentState != target);

      // If we have arrived at a state without an init function, we are done
      if(NULL_INIT_FUNC == hsm->states[hsm->currentState].initFunc)
         break;

      // Set the new target, and keep going...
      target = hsm->states[hsm->currentState].initFunc(nullEvent);

   } while(1);

   return HSM_NO_ERROR;
}

// Destroy a state machine
HSM_Error_t HSM_destroy( HSM_Handle_t *hsm)
{

   if(NULL == hsm)
      return HSM_NULL_POINTER;

   if(hsm->key != KEY_VAL)
      return HSM_NOT_CREATED;

   // Set to an uninitialized state
   memset(hsm, 0, sizeof( HSM_Handle_t ));

   return HSM_NO_ERROR;
}

HSM_Error_t HSM_processEvent( HSM_Handle_t *hsm, event_t ev)
{
   uint16_t baseState;
   uint16_t firstMatch;
   uint16_t transTo = hsm->stateCount;
   bool found, localTrans;
   HSM_Error_t err;

   if(NULL == hsm)
      return HSM_NULL_POINTER;

   if(hsm->key != KEY_VAL)
      return HSM_NOT_CREATED;

   // Bail if this hsm is not initialized
   if(hsm->currentState == hsm->stateCount)
      return HSM_NOT_INITIALIZED;

   if(hsm->currentState > hsm->stateCount)
      return HSM_INVALID_STATE;

   // Find the first index where this event appears in the transition table
   firstMatch = findIndexForEvent(hsm->transitions, hsm->transCount, ev.ev);

   // Bail out if the event doesn't exist in the table
   if(EVENT_NOT_FOUND == firstMatch )
      return HSM_EV_NOT_IN_TABLE;

   // Keep a temp state for navigation purposes...
   baseState = hsm->currentState;

   // Move upward until we run out of states...
   while(baseState < hsm->stateCount)
   {
      // temporary offset into event list...
      uint16_t scanOffset = firstMatch;

      // Id of lowest common ancestor
      uint16_t lca;

      found = false;

      do // Scan through event table for this state
      {
         // If we found a transition for the specified event coming from this state
         if( hsm->transitions[scanOffset].from == baseState )
         {
            // If the guard function is absent OR it returns true...
            if ( (NULL_GUARD_FUNC == hsm->transitions[scanOffset].guard ) ||
                 (true            == hsm->transitions[scanOffset].guard(ev)) )
            {
               // Mark the state we wish to move to...
               transTo = hsm->transitions[scanOffset].to;

               // Keep track of whether this is a local transition
               localTrans = hsm->transitions[scanOffset].local;

               // Note that we found something
               found = true;

               // Exit the do/while loop
               break;
            }
         }
      }while(hsm->transitions[++scanOffset].ev == ev.ev);

      // Did we find a match?
      if(true == found)
      {
         // Execute action function associated with the transition we are making..
         if(hsm->transitions[scanOffset].action != NULL_ACTION_FUNC)
            hsm->transitions[scanOffset].action(ev);

         // If this is not an internal transition...
         if(transTo != hsm->stateCount)
         {

            // Find the lowest common ancestor of the target state and the state at which a transition was found...
            lca = findLca(hsm, baseState, transTo);

            // move upward towards lca ancestor
            while(hsm->currentState != lca)
            {
               // If we aren't at the top yet, or we are and the localTrans settings if false...
               if(hsm->states[hsm->currentState].parent != lca || false == localTrans)
                 // Run the exit funtion of our current state
                 if(hsm->states[hsm->currentState].exitFunc != NULL_EXIT_FUNC)
                     hsm->states[hsm->currentState].exitFunc(ev);

               // Move up...
               hsm->currentState = hsm->states[hsm->currentState].parent;

               // Validity check..
               if(hsm->currentState > hsm->stateCount)
                  return HSM_INVALID_STATE;
            }

            // move downward to target.
            do
            {
               // Move down towards target...
               hsm->currentState = findLineFromAToB(hsm, hsm->currentState, transTo);

               // Validity check..
               if(hsm->currentState >= hsm->stateCount)
                  return HSM_INVALID_STATE;

               // Make sure we don't call entry if we are at top and localTrans is true..
               if(hsm->states[hsm->currentState].parent != lca || false == localTrans)
                  // Run the entry function if it exists...
                  if(hsm->states[hsm->currentState].entryFunc != NULL_ENTRY_FUNC)
                     hsm->states[hsm->currentState].entryFunc(ev);

            } while(hsm->currentState != transTo);
         }

         // Drill down to leaf node if not already there...
         err = traverseCompositeState(hsm, ev);

         if(err != HSM_NO_ERROR)
            return err;

         break; // We don't need to look any  further
      }

      baseState = hsm->states[baseState].parent;
   }

   if(false == found )
      return HSM_NO_EV_HANDLER_FOUND;

   return HSM_NO_ERROR;
}


// WARNING:  Do not call this from within any called function (entry, exit, init, guard or action)
HSM_Error_t HSM_exit(  HSM_Handle_t *hsm  )
{
   event_t nullEvent = {0};

   if(NULL == hsm)
      return HSM_NULL_POINTER;

   if(hsm->key != KEY_VAL)
      return HSM_NOT_CREATED;

   // Bail if this hsm is not initialized
   if(hsm->currentState == hsm->stateCount)
      return HSM_NOT_INITIALIZED;

   if(hsm->currentState > hsm->stateCount)
      return HSM_INVALID_STATE;

   while(hsm->currentState != hsm->stateCount)
   {
      if(hsm->states[hsm->currentState].exitFunc != NULL_EXIT_FUNC)
         hsm->states[hsm->currentState].exitFunc(nullEvent);

      hsm->currentState = hsm->states[hsm->currentState].parent;
   }

   // Call the state machine exit function
   if(hsm->exitFunc != NULL_EXIT_FUNC) hsm->exitFunc(nullEvent);

   return HSM_NO_ERROR;
}

char* HSM_getErrorString( HSM_Error_t err )
{
   switch (err)
   {
      case HSM_NO_ERROR:                             return "No error";
      case HSM_NULL_POINTER:                         return "Null Pointer";
      case HSM_EV_NOT_IN_TABLE:                      return "Event not in table";
      case HSM_TOP_PARENT_NOT_NULL:                  return "Top parent not null";
      case HSM_MISSING_PARENT_ON_SUBNODE:            return "Parent can't be null";
      case HSM_MISSING_INIT_FUNC:                    return "Missing init function";
      case HSM_UNEXPECTED_INIT_FUNC:                 return "Unexpected init function";
      case HSM_INVALID_STATE:                        return "Invalid state";
      case HSM_EVENT_LIST_NOT_SORTED:                return "Transition table not sorted";
      case HSM_NO_EV_HANDLER_FOUND:                  return "Event not handled";
      case HSM_NOT_INITIALIZED:                      return "State machine not initialized";
      case HSM_NOT_CREATED:                          return "State machine not created";
      case HSM_ALREADY_INITIALIZED:                  return "State machine already initialized";
      case HSM_ALREADY_CREATED:                      return "State machine alread created";
      case HSM_USELESS_TRANSITION:                   return "Useless transition";
      case HSM_CIRCULAR_HIERARCY:                    return "Circular Hierarcy";
      case HSM_INIT_FUNC_RETURNED_INVALID_STATE:     return "Init function returned invalid state";
      case HSM_LOCAL_TRANS_NOT_ANCESTRALLY_RELATED:  return "Local transition must be ancestrally related";
      default:                                       return "Unknown error code";
   }
}

//////////////////
// LOCAL FUNCTIONS
//////////////////

// Find the first line in the transition table that matches the given event
//   Uses a binary search for quick access.
static uint16_t findIndexForEvent(const transDef_t *table, uint16_t size, uint16_t ev )
{
   return (findKey(table, ev, 0, size - 1));
}

// Recursive helper functions for binary search
static uint16_t findKey(const transDef_t *table, uint16_t key, uint16_t min, uint16_t max)
{
   uint16_t mid = (min + max)/2;
   uint16_t thisKey = table[mid].ev;

   if(thisKey == key)
   {
      // back up to first entry in this group...
      while(mid--) if(table[mid].ev != key) return mid + 1;

      // We arrive here if we back up all the way to the first position, in which case, just return it.
      return 0;
   }
   else if (thisKey > key)
   {
      // can't narrow any further - no match available.
      if(min == mid) return EVENT_NOT_FOUND;

      // Recurse, moving in closer
      else return findKey( table, key, min, mid-1 );
   }
   else
   {
      // can't narrow any further - no match available.
      if(max == mid) return EVENT_NOT_FOUND;

      // Recurse, moving in closer
      else return findKey( table, key, mid+1, max );

   }
}

// Find the lowest common ancestor for the two states.
static uint16_t findLca(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB)
{
   uint16_t a, b;

   a = stateA;
   do
   {
      b = stateB;
      do
      {
         if(hsm->states[a].parent == hsm->states[b].parent)
            return hsm->states[a].parent;

         b = hsm->states[b].parent;
      } while(b != hsm->stateCount);

      a = hsm->states[a].parent;
   } while(a != hsm->stateCount);

   // Ideally, since every state has been checked in the init function for
   //   validity and an upward path to the top state, we should never be here.
   return hsm->stateCount;
}

// Find the child of A through which B descends (or is actually B)
static uint16_t findLineFromAToB(const HSM_Handle_t *hsm, uint16_t a, uint16_t b )
{
   while(hsm->states[b].parent != a && hsm->states[b].parent < hsm->stateCount)
      b = hsm->states[b].parent;

   return hsm->states[b].parent == a ? b : hsm->stateCount;
}

// Call the init funciton for this state and move to that substate, recursively calling
//  init functions
static HSM_Error_t traverseCompositeState(HSM_Handle_t *hsm, event_t ev )
{
   // As long as an init function is defined...
   while(hsm->states[hsm->currentState].initFunc != NULL_INIT_FUNC)
   {
      uint16_t target = hsm->states[hsm->currentState].initFunc(ev);

      // Verify valid state returned
      if(target >= hsm->stateCount)
         return HSM_INIT_FUNC_RETURNED_INVALID_STATE;

      do
      {
         uint16_t nextStep = findLineFromAToB(hsm, hsm->currentState, target);

         if(nextStep >= hsm->stateCount)
            return HSM_INIT_FUNC_RETURNED_INVALID_STATE;

         hsm->currentState = nextStep;

         if(hsm->states[hsm->currentState].entryFunc != NULL)
            hsm->states[hsm->currentState].entryFunc(ev);

      }while (hsm->currentState != target);

   }
   return HSM_NO_ERROR;
}
