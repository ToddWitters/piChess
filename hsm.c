#include "hsm.h"

// FOR DEBUGGING ONLY
#include "diag.h"
#include "hsmDefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Local functions
static uint16_t findIndexForEvent(const transDef_t *table, uint16_t tableSize, uint16_t ev );
static uint16_t findKey(const transDef_t *table, uint16_t key, uint16_t min, uint16_t max);
static uint16_t findLca(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB);
static uint16_t findLineFromAToB(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB );
static HSM_Error_t traverseCompositeState(HSM_Handle_t *hsm );


#define HSM_DPRINT(...)    DPRINT(__VA_ARGS__)

extern const char *eventName[];


// Create a state machine, verify passed structures
HSM_Error_t HSM_createHSM( const stateDef_t *states,
                       const transDef_t *transitions,
                       uint16_t stateCount,
                       uint16_t transCount,
                       HSM_Handle_t **hsm)
{

   uint16_t last;
   uint16_t *childCountArray;
   uint16_t *boolArray;

   int i,j;

   // Verify passed parameters....
   if( (NULL == states) || (NULL == transitions) )
   {
      HSM_DPRINT("ERROR: One or more passed pointers are NULL\n");
      return HSM_NULL_POINTER;
   }

   // Validate passed tables...

   // Make sure all state references are within range...
   //  Make sure the only state with a NULL parent is top most
   //  Make sure every substate specifies its parent state
   for(i = 0; i < stateCount; i++)
   {
      if(0 == i)
      {
         if((states[i].parent != stateCount))
         {
            HSM_DPRINT("ERROR: Topmost state ""%s"" has a non-NULL parent\n", states[i].displayName);
            return HSM_TOP_PARENT_NOT_NULL;
         }
      }
      else
      {
         if((states[i].parent == stateCount))
         {
            HSM_DPRINT("ERROR: State ""%s"" has no parent defined\n", states[i].displayName);
            return HSM_MISSING_PARENT_ON_SUBNODE;
         }
      }

      if( states[i].parent     > stateCount )
      {
         HSM_DPRINT("ERROR: State ""%s"" has an invalid parent state definition\n", states[i].displayName);
         return HSM_INVALID_STATE;
      }

   }
   // For each node, work towards top, ensuring we don't revisit any
   //   nodes (circular reference) and we eventually reach the top;

   // NOTE: Ignore top, start with next state...

   boolArray = malloc(sizeof(bool_t) * stateCount);

   if(boolArray == NULL)
   {
      HSM_DPRINT("ERROR: Out of memory\n");
      return HSM_OUT_OF_MEMORY;
   }

   for(i = 1; i < stateCount; i++)
   {
      uint16_t walkingState = i;

      // Start with bool array set to false.  This will track which nodes
      //   we have already visited.
      memset(boolArray, 0x00, stateCount * sizeof(bool_t));

      // Work up the chain...
      //   bail out on any of...
      //     (1) Invalid state encountered
      //     (2) Top reached
      //     (3) Previously visited node hit again
      while( (walkingState < stateCount) && (walkingState != 0) && (boolArray[walkingState] == FALSE) )
      {
         // mark this node as visited...
         boolArray[walkingState] = TRUE;

         // Move up...
         walkingState = states[walkingState].parent;

      }

      if(walkingState >= stateCount)
      {
         free(boolArray);
         HSM_DPRINT("ERROR: Traversing upward from state ""%s"" found an invalid parent state reference\n", states[i].displayName);
         return HSM_INVALID_STATE;
      }

      if(walkingState != 0)
      {
         free(boolArray);
         HSM_DPRINT("ERROR: Traversing upward from state ""%s"" found a circular reference loop\n", states[i].displayName);
         return HSM_CIRCULAR_HIERARCY;
      }
   }

   // Make sure transition list is sorted.
   // as a secondary action, count entries as we progress
   last = 0;
   for(i=0; i< transCount; i++)
   {
      if(transitions[i].ev < last)
      {
         HSM_DPRINT("ERROR: Found transition table event out of order at position %d\n", i);
         return HSM_EVENT_LIST_NOT_SORTED;
      }

      if(transitions[i].to > stateCount)
      {
         HSM_DPRINT("ERROR: Found invalid ""to"" state %d at position %d\n",transitions[i].to, i);
         return HSM_INVALID_STATE;
      }

      if(transitions[i].from >= stateCount)
      {
         HSM_DPRINT("ERROR: Found invalid ""from"" state %d at position %d\n",transitions[i].from, i);
         return HSM_INVALID_STATE;
      }

      last = transitions[i].ev;
   }

   // Ensure composite states have picker functions defined

   childCountArray = malloc(sizeof(uint16_t) * stateCount);

   if(childCountArray == NULL)
   {
      HSM_DPRINT("ERROR: Out of memory\n");
      return HSM_OUT_OF_MEMORY;
   }

   // store count of child nodes for each parent
   memset(childCountArray, 0, sizeof(uint16_t) * stateCount);

   // Pass 1.. count # of children for each node
   for(i=1; i < stateCount; i++) childCountArray[states[i].parent]++;

   // Pass 2.. verify the substate picker functions are defined for states that:
   //    (a) has children and
   //    (b) is a target of a transition
   for(i=0; i < stateCount; i++)
   {
      // Does this node have children?
      if(childCountArray[i] != 0)
      {
         // Yes... look for any transitions that terminate here...
         for(j = 0; j < transCount; j++)
         {
            // Does the "to" state match the state we are looking at?
            if(transitions[j].to == i)
            {
               // Yes... make sure we have a picker function
               if(states[i].pickerFunc == NULL_SUBSTATE_PICKER_FUNC)
               {
                  free(childCountArray);
                  HSM_DPRINT("ERROR: Missing substate picker function for composite state ""%s"" which is a target of one or more transitions\n", states[i].displayName);
                  return HSM_MISSING_PICKER_FUNC;
               }

               // No need to look further for this state
               break;
            }
         }
         if( i != 0 && j == transCount && states[i].pickerFunc != NULL_SUBSTATE_PICKER_FUNC)
         {
            HSM_DPRINT("WARNING: Substate picker function defined for composite state ""%s"", but no transitions terminate here\n", states[i].displayName);
         }
      }
      else
      {
         if(states[i].pickerFunc != NULL_SUBSTATE_PICKER_FUNC)
         {
            HSM_DPRINT("ERROR: Unexpected substate picker function on state %s\n", states[i].displayName);
            return HSM_MISSING_PICKER_FUNC;
         }
      }
   }

   free(childCountArray);

   // Now that we have verified the integrity of table, create a handle
   *hsm = malloc( sizeof(HSM_Handle_t));

   if(hsm == NULL )
   {
      HSM_DPRINT("ERROR: Out of memory\n");
      return HSM_OUT_OF_MEMORY;
   }

   (*hsm)->states         = states;
   (*hsm)->transitions    = transitions;
   (*hsm)->currentState   = stateCount;
   (*hsm)->hsmDisposition = HSM_CREATED;
   (*hsm)->transCount     = transCount;
   (*hsm)->stateCount     = stateCount;

   return HSM_NO_ERROR;
}

// initialize hsm (enter top state)
HSM_Error_t HSM_init( HSM_Handle_t *hsm )
{
   HSM_Error_t err;

   // Traverse downward from top to leaf node following initState pointers,
   //   running all entry functions on the way down
   event_t junk = {0, 0};

   if(hsm->hsmDisposition != HSM_CREATED) return HSM_NOT_INITIALIZED;

   // Start at the top...
   hsm->currentState = 0;

   // Run the entry function if it exists
   if(hsm->states[hsm->currentState].entryFunc != NULL)
      hsm->states[hsm->currentState].entryFunc(junk);

   err = traverseCompositeState(hsm);

   if(err == HSM_NO_ERROR)
   {
      hsm->hsmDisposition = HSM_INITIALIZED;
   }

   return err;
}

// Destroy a state machine
HSM_Error_t HSM_destroy( HSM_Handle_t *hsm)
{
   // Free memory created when sm was built
   free(hsm);

   return HSM_NO_ERROR;
}

// Process a new event,
HSM_Error_t HSM_processEvent( HSM_Handle_t *hsm, event_t ev)
{
   uint16_t tempState;
   uint16_t transTableListOffset;
   uint16_t transTo = hsm->stateCount;
   bool_t found, localTrans;
   HSM_Error_t err;

   // Bail if this hsm is not initialized
   if(hsm->hsmDisposition != HSM_INITIALIZED)
   {
      HSM_DPRINT("ERROR: Call to processEvent with uninitialized state machine\n");
      return HSM_NOT_INITIALIZED;
   }

   // Find the first index where this event appears in the table
   transTableListOffset = findIndexForEvent(hsm->transitions, hsm->transCount, ev.ev);

   // Bail out if the event doesn't exist in the table
   if(transTableListOffset == 0xFFFF)
   {
      HSM_DPRINT("WARNING: Event %d not handled by this state or its ancestors\n", (uint16_t)ev.ev);
      return HSM_EV_NOT_IN_TABLE;
   }

   // Keep a temp state for navigation purposes...
   tempState = hsm->currentState;

   // Move upward until we run out of states...
   while(tempState < hsm->stateCount)
   {
      // temporary offset into event list...
      uint16_t scanOffset = transTableListOffset;

      // Id of lowest common ancestor
      uint16_t lca;

      found = FALSE;

      do // Scan through event table for this state
      {

         // If we found a transition for the specified event coming from this state
         if( hsm->transitions[scanOffset].from == tempState )
         {
            // .. and the guard function passes (or isn't defined)
            if ( (hsm->transitions[scanOffset].guard     == NULL) ||
                 (hsm->transitions[scanOffset].guard(ev) == TRUE) )
            {
               // Mark the state we wish to move to...
               transTo = hsm->transitions[scanOffset].to;

               localTrans = hsm->transitions[scanOffset].local;

               // Note that we found something
               // printf("Found matching transition at line %d\n", scanOffset);
               found = TRUE;

               // Exit the do/while loop
               break;
            }
         }

      }while(hsm->transitions[++scanOffset].ev == ev.ev);

      // Did we find a match?
      if(found == TRUE)
      {

         // If this is not an internal transition, call exit functions up to, but
         //   not including the lca
         if(transTo != hsm->stateCount)
         {

            // compute lca between current state, and the state to move to...
            lca = findLca(hsm, hsm->currentState, transTo);

            // move upward towards lca ancestor
            while(hsm->currentState != lca)
            {
               // If the state we are transitioning to is in this line, don't execute the
               //   exit function if this is a local transition...
               if(hsm->currentState != transTo || localTrans == FALSE)
               {
                  // Run the exit funtion of our current state
                  if(hsm->states[hsm->currentState].exitFunc != NULL_EXIT_FUNC)
                     hsm->states[hsm->currentState].exitFunc(ev);
               }

               // Move up...
               hsm->currentState = hsm->states[hsm->currentState].parent;
            }

            // Make sure we didn't hit an invalid state...
            if(hsm->currentState > hsm->stateCount)
            {
               DPRINT("ERROR: Found invalid parent state reference during tree traversal\n");
               return HSM_INVALID_STATE;
            }

         }

         // Execute action function associated with the transition
         if(hsm->transitions[scanOffset].action != NULL)
            hsm->transitions[scanOffset].action(ev);


         // If this was not an internal transition...
         if(transTo != hsm->stateCount)
         {

            // Move down the tree towards the target..
            do
            {
               // Find the child of the current node that is on the line of descent for the target state...
               hsm->currentState = findLineFromAToB(hsm, hsm->currentState, transTo);

               // Validity check..
               if(hsm->currentState >= hsm->stateCount)
               {
                  HSM_DPRINT("ERROR: Invalid state returned from %s\n", __FUNCTION__);
                  return HSM_INVALID_STATE;
               }

               // If we are on the "transitionfrom state, don't execute
               //   entry function if this is a local transition...
               if(hsm->currentState != hsm->transitions[scanOffset].from || localTrans == FALSE)
               {
                  // Run the entry function...
                  if(hsm->states[hsm->currentState].entryFunc != NULL_EXIT_FUNC)
                     hsm->states[hsm->currentState].entryFunc(ev);
               }

            }while(hsm->currentState != transTo);
         }

         // If we landed on a composite state, drill down....
         err = traverseCompositeState(hsm);

         if(err != HSM_NO_ERROR)
            return err;

         break; // We don't need to look any  further

      }

      if(found == TRUE) break;

      tempState = hsm->states[tempState].parent;
   }

   if(found == FALSE)
   {
//      HSM_DPRINT("WARNING: Handler not found\n");
      return HSM_NO_EV_HANDLER_FOUND;
   }

   return HSM_NO_ERROR;
}

//////////////////
// LOCAL FUNCTIONS
//////////////////

// Find the first line in the transition table that matches the given event
//   Uses a binary search for quick access.
static uint16_t findIndexForEvent(const transDef_t *table, uint16_t tableSize, uint16_t ev )
{
   return (findKey(table, ev, 0, tableSize - 1));
}

// Recursive helper functions for binary search
static uint16_t findKey(const transDef_t *table, uint16_t key, uint16_t min, uint16_t max)
{
   uint16_t mid = (min + max)/2;
   uint16_t thisKey = table[mid].ev;

   if(thisKey == key)
   {
      // back up to first entry in this group...
      while(mid--)
      {
         if(table[mid].ev != key) return mid + 1;
      }
      return 0;
   }
   else if (thisKey > key)
   {
      // can't narrow any further - no match available.
      if(min == mid) return 0xFFFF;

      // Recurse, moving in closer
      else return findKey( table, key, min, mid-1 );
   }
   else
   {
      // can't narrow any further - no match available.
      if(max == mid) return 0xFFFF;

      // Recurse, moving in closer
      else return findKey( table, key, mid+1, max );

   }
}

// Find the lowest common ancestor for the two states.
static uint16_t findLca(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB)
{
      uint16_t aAncestor, bAncestor;


      if(stateA >= hsm->stateCount || stateB >= hsm->stateCount) return hsm->stateCount;

      aAncestor = hsm->states[stateA].parent;
      do
      {
         bAncestor = hsm->states[stateB].parent;
         do
         {

           if(aAncestor == bAncestor) return aAncestor;

           bAncestor = hsm->states[bAncestor].parent;

        } while(bAncestor != hsm->stateCount);

        aAncestor = hsm->states[aAncestor].parent;

     }while(aAncestor != hsm->stateCount);

     // Ideally, since every state has been checked in the init function for
     //   validity and an upward path to the top state, we should never be here.
     HSM_DPRINT("ERROR: No common ancestor found for states %s and %s\n", hsm->states[stateA].displayName, hsm->states[stateB].displayName);
     return hsm->stateCount;
}

// Find the child of A that is B OR that is a direct ancestor of B.
static uint16_t findLineFromAToB(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB )
{
   uint16_t walkingState = stateB;

   while(hsm->states[walkingState].parent != stateA && hsm->states[walkingState].parent <= hsm->stateCount)
   {
      walkingState = hsm->states[walkingState].parent;
   }

   return walkingState;
}

// Call the "pickerFunc" for this state and all contained states.  Since the init
//  function confirmed that all composite states have picker functions, we know
//  that a leaf node will be reached.
static HSM_Error_t traverseCompositeState(HSM_Handle_t *hsm )
{
   // As long as there are substates, enter them and exeucte their entry function, if any
   while(hsm->states[hsm->currentState].pickerFunc != NULL_SUBSTATE_PICKER_FUNC)
   {
      event_t junk = {0, 0};
      uint16_t test = hsm->states[hsm->currentState].pickerFunc(junk);

      // Verify valid state returned
      if(test >= hsm->stateCount)
      {
         HSM_DPRINT("ERROR: Substate picker function defined for composite state %s, returned inavlid state %d\n", hsm->states[hsm->currentState].displayName, test);
         return HSM_PICKER_RETURNED_INVALID_STATE;
      }

      // Verify returned state is actually a child of current state
      if(hsm->states[test].parent != hsm->currentState)
      {
         HSM_DPRINT("ERROR: Substate picker function defined for composite state %s, returned state %s, which is not a child\n", hsm->states[hsm->currentState].displayName, hsm->states[test].displayName);
         return HSM_PICKER_RETURNED_NON_CHILD;
      }

      // Update current state
      hsm->currentState = test;

      // Call the entry function if it exists.
      if(hsm->states[hsm->currentState].entryFunc != NULL)
         hsm->states[hsm->currentState].entryFunc(junk);

   }
   return HSM_NO_ERROR;

}
