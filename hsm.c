#include "hsm.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Local functions
static uint16_t findIndexForEvent(const transDef_t *table, uint16_t tableSize, uint16_t ev );
static uint16_t findKey(const transDef_t *table, uint16_t key, uint16_t min, uint16_t max);
static uint16_t findLca(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB);
static uint16_t findLineFromAToB(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB );


#define HSM_DPRINT(...) printf("HSM>> "); printf(__VA_ARGS__); printf("\n");

extern const char *eventName[];


// Create a state machine, verify passed structures
HSM_Error_t createHSM( const stateDef_t *states,
                       const transDef_t *transitions,
                       uint16_t stateCount,
                       uint16_t transCount,
                       HSM_Handle_t **hsm)
{

   uint16_t last;
   bool_t    found;
   uint16_t *childCountArray;
   uint16_t *boolArray;

   int i,j;

   // Verify passed parameters....
   if( (NULL == states) || (NULL == transitions) )
   {
      HSM_DPRINT("ERROR: One or more passed pointers are NULL");
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
            HSM_DPRINT("ERROR: Topmost state ""%s"" has a non-NULL parent", states[i].displayName);
            return HSM_TOP_PARENT_NOT_NULL;
         }
      }
      else
      {
         if((states[i].parent == stateCount))
         {
            HSM_DPRINT("ERROR: State ""%s"" has no parent defined", states[i].displayName);
            return HSM_MISSING_PARENT_ON_SUBNODE;
         }
      }

      if( states[i].parent     > stateCount )
      {
         HSM_DPRINT("ERROR: State ""%s"" has an invalid parent state definition", states[i].displayName);
         return HSM_INVALID_STATE;
      }

   }

   // For each node, work towards top, ensuring we don't revisit any
   //   nodes (circular reference) and we eventually reach the top;

   // NOTE: Ignore top, start with next state...

   boolArray = malloc(sizeof(bool_t) * stateCount);

   if(boolArray == NULL)
   {
      HSM_DPRINT("ERROR: Out of memory");
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
      while( (walkingState < stateCount) && (walkingState != 0) && (boolArray[walkingState] == false) )
      {
         // mark this node as visited...
         boolArray[walkingState] = true;

         // Move up...
         walkingState = states[walkingState].parent;

      }

      if(walkingState >= stateCount)
      {
         free(boolArray);
         HSM_DPRINT("ERROR: Traversing upward from state ""%s"" found an invalid parent state reference", states[i].displayName);
         return HSM_INVALID_STATE;
      }

      if(walkingState != 0)
      {
         free(boolArray);
         HSM_DPRINT("ERROR: Traversing upward from state ""%s"" found a circular reference loop", states[i].displayName);
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
         HSM_DPRINT("ERROR: Found transition table event out of order at position %d", i);
         return HSM_EVENT_LIST_NOT_SORTED;
      }

      if(transitions[i].to > stateCount)
      {
         HSM_DPRINT("ERROR: Found invalid ""to"" state %d at position %d",transitions[i].to, i);
         return HSM_INVALID_STATE;
      }

      if(transitions[i].from >= stateCount)
      {
         HSM_DPRINT("ERROR: Found invalid ""from"" state %d at position %d",transitions[i].from, i);
         return HSM_INVALID_STATE;
      }

      last = transitions[i].ev;
   }

   // Ensure composite states have picker functions defined

   childCountArray = malloc(sizeof(uint16_t) * stateCount);

   if(childCountArray == NULL)
   {
      HSM_DPRINT("ERROR: Out of memory");
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
                  HSM_DPRINT("ERROR: Missing substate picker function for composite state ""%s"" which is a target of one or more transitions", states[i].displayName);
                  return HSM_MISSING_PICKER_FUNC;
               }

               // No need to look further for this state
               break;
            }
         }
         if( i != 0 && j == transCount && states[i].pickerFunc != NULL_SUBSTATE_PICKER_FUNC)
         {
            HSM_DPRINT("WARNING: Substate picker function defined for composite state ""%s"", but no transitions terminate here", states[i].displayName);
         }
      }
      else
      {
         if(states[i].pickerFunc != NULL_SUBSTATE_PICKER_FUNC)
         {
            HSM_DPRINT("WARNING: Useless substate picker function defined for non-composite state ""%s""", states[i].displayName);
         }
      }
   }

   free(childCountArray);

   // Now that we have verified the integrity of table, create a handle
   *hsm = malloc( sizeof(HSM_Handle_t));

   if(hsm == NULL )
   {
      HSM_DPRINT("ERROR: Out of memory");
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
HSM_Error_t initHSM( HSM_Handle_t *hsm )
{
   // Traverse downward from top to leaf node following initState pointers,
   //   running all entry functions on the way down
   event_t junk = {0, NULL};

   if(hsm->hsmDisposition != HSM_CREATED) return HSM_NOT_INITIALIZED;

   // Start at the top...
   hsm->currentState = 0;

   // Run the entry function if it exists
   if(hsm->states[hsm->currentState].entryFunc != NULL)
      hsm->states[hsm->currentState].entryFunc(junk);

   // As long as there are substates, enter them and exeucte their entry function, if any
   while(hsm->states[hsm->currentState].pickerFunc != NULL_SUBSTATE_PICKER_FUNC)
   {
      uint16_t test = hsm->states[hsm->currentState].pickerFunc(junk);

      if(test >= hsm->stateCount)
      {
         HSM_DPRINT("ERROR: Substate picker function defined for composite state %s, returned inavlid state %d", hsm->states[hsm->currentState].displayName, test);
         return HSM_PICKER_RETURNED_INVALID_STATE;
      }

      if(hsm->states[test].parent != hsm->currentState)
      {
         HSM_DPRINT("ERROR: Substate picker function defined for composite state %s, returned state %s, which is not a child", hsm->states[hsm->currentState].displayName, hsm->states[test].displayName);
         return HSM_PICKER_RETURNED_NON_CHILD;
      }

      hsm->currentState = test;

      printf("Set current state to %s\n", hsm->states[hsm->currentState].displayName);

      if(hsm->states[hsm->currentState].entryFunc != NULL)
         hsm->states[hsm->currentState].entryFunc(junk);

   }

   hsm->hsmDisposition = HSM_INITIALIZED;

   return HSM_NO_ERROR;
}

// Destroy a state machine
HSM_Error_t destroyHSM( HSM_Handle_t *hsm)
{
   // Free memory created when sm was built
   free(hsm);

   return HSM_NO_ERROR;
}

// Process a new event,
HSM_Error_t processEvent( HSM_Handle_t *hsm, event_t ev)
{
   uint16_t tempState, lcaState;
   uint16_t transTableListOffset;
   uint16_t transTo = hsm->stateCount;
   bool_t found, localTrans, movingFromLca;

   HSM_DPRINT("Processing Event %s", eventName[ev.ev]);

   // Bail if this hsm is not initialized
   if(hsm->hsmDisposition != HSM_INITIALIZED)
   {
      HSM_DPRINT("ERROR: Call to processEvent with uninitialized state machine");
      return HSM_NOT_INITIALIZED;
   }

   // Find the first index where this event appears in the table
   transTableListOffset = findIndexForEvent(hsm->transitions, hsm->transCount, ev.ev);

   // Bail out if the event doesn't exist in the table
   if(transTableListOffset == 0xFFFF)
   {
      HSM_DPRINT("WARNING: Event %d not found in transition table", (uint16_t)ev.ev);
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

      found = false;

      do // Scan through event table for this state
      {

         // If we found a transition for the specified event coming from this state
         if( hsm->transitions[scanOffset].from == tempState )
         {
            // .. and the guard function passes (or isn't defined)
            if ( (hsm->transitions[scanOffset].guard     == NULL) ||
                 (hsm->transitions[scanOffset].guard(ev) == true) )
            {
               // Mark the state we wish to move to...
               transTo = hsm->transitions[scanOffset].to;

               localTrans = hsm->transitions[scanOffset].local;

               // Note that we found something
               printf("Found transition at line %d\n", scanOffset);
               found = true;

               // Exit the do/while loop
               break;
            }
         }

      }while(hsm->transitions[++scanOffset].ev == ev.ev);

      // Did we find a match?
      if(found == true)
      {

         // If this is not an internal transition, call exit functions up to lca
         if(transTo != hsm->stateCount)
         {

            // move upward to lca ancestor
            lca = findLca(hsm, hsm->currentState, transTo);

            if(lca == hsm->currentState) movingFromLca = true;
            else movingFromLca = false;

            while(hsm->currentState != lca)
            {
               if(hsm->states[hsm->currentState].exitFunc != NULL_EXIT_FUNC)
                  hsm->states[hsm->currentState].exitFunc(ev);

               hsm->currentState = hsm->states[hsm->currentState].parent;
               printf("Set current state to %s\n", hsm->states[hsm->currentState].displayName);

            }

            // if we just entered the destination state &&
            //    the destination is also the LCM &&
            //    this wasn't a local transitions &&
            if(hsm->currentState == transTo && localTrans == false)
            {
               if(hsm->states[hsm->currentState].exitFunc != NULL_EXIT_FUNC)
                  hsm->states[hsm->currentState].exitFunc(ev);

               if(hsm->states[hsm->currentState].entryFunc != NULL_ENTRY_FUNC)
                  hsm->states[hsm->currentState].entryFunc(ev);
            }


         }

         // Execute action function associated with the transition
         if(hsm->transitions[scanOffset].action != NULL)
         {
            printf("Execute Action function for transition %d\n", scanOffset);
            hsm->transitions[scanOffset].action(ev);
         }

         // If this is not an internal transition...
         if(transTo != hsm->stateCount)
         {

            // If we we started at LCA &&
            //   this wasn't a local transition
            //   && the exit function is defined

            if(movingFromLca && localTrans == false)
            {
               if(hsm->states[hsm->currentState].exitFunc != NULL_EXIT_FUNC)
                  hsm->states[hsm->currentState].exitFunc(ev);

               if(hsm->states[hsm->currentState].entryFunc != NULL_ENTRY_FUNC)
                  hsm->states[hsm->currentState].entryFunc(ev);

            }

            do
            {
               hsm->currentState = findLineFromAToB(hsm, hsm->currentState, transTo);
               printf("Set current state to %s\n", hsm->states[hsm->currentState].displayName);

               if(hsm->states[hsm->currentState].entryFunc != NULL_EXIT_FUNC)
                  hsm->states[hsm->currentState].entryFunc(ev);

            }while(hsm->currentState != transTo);
         }

         while(hsm->states[hsm->currentState].pickerFunc != NULL_SUBSTATE_PICKER_FUNC)
         {

            event_t junk = {0, NULL};

            uint16_t test = hsm->states[hsm->currentState].pickerFunc(junk);

            if(test >= hsm->stateCount)
            {
               HSM_DPRINT("ERROR: Substate picker function defined for composite state %s, returned inavlid state %d", hsm->states[hsm->currentState].displayName, test);
               return HSM_PICKER_RETURNED_INVALID_STATE;
            }

            if(hsm->states[test].parent != hsm->currentState)
            {
               HSM_DPRINT("ERROR: Substate picker function defined for composite state %s, returned state %s, which is not a child", hsm->states[hsm->currentState].displayName, hsm->states[test].displayName);
               return HSM_PICKER_RETURNED_NON_CHILD;
            }

            hsm->currentState = test;

            printf("Set current state to %s\n", hsm->states[hsm->currentState].displayName);

            if(hsm->states[hsm->currentState].entryFunc != NULL)
               hsm->states[hsm->currentState].entryFunc(junk);

         }


         break; // We don't need to look any  further

      }

      if(found == true) break;

      tempState = hsm->states[tempState].parent;
   }

   if(found == false)
   {
      HSM_DPRINT("WARNING: No handler for event %d not found in state %s or its ancestors", (uint16_t)ev.ev, hsm->states[hsm->currentState].displayName);
      return HSM_NO_EV_HANDLER_FOUND;
   }

   return HSM_NO_ERROR;
}

static uint16_t findIndexForEvent(const transDef_t *table, uint16_t tableSize, uint16_t ev )
{
   return (findKey(table, ev, 0, tableSize - 1));
}

static uint16_t findKey(const transDef_t *table, uint16_t key, uint16_t min, uint16_t max)
{
   uint16_t mid = (min + max)/2;
   uint16_t thisKey = table[mid].ev;

   if(thisKey == key)
   {
      // back up to first match in this group...
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

static uint16_t findLca(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB)
{
      uint16_t aAncestor, bAncestor;


      if(stateA >= hsm->stateCount || stateB >= hsm->stateCount) return hsm->stateCount;

      aAncestor = stateA;
      do
      {
         bAncestor = stateB;
         do
         {

           if(aAncestor == bAncestor) return aAncestor;

           bAncestor = hsm->states[bAncestor].parent;

        } while(bAncestor != hsm->stateCount);

        aAncestor = hsm->states[aAncestor].parent;

     }while(aAncestor != hsm->stateCount);

     HSM_DPRINT("ERROR: No common ancestor found for states %s and %s", hsm->states[stateA].displayName, hsm->states[stateB].displayName);

     return hsm->stateCount;
}

static uint16_t findLineFromAToB(const HSM_Handle_t *hsm, uint16_t stateA, uint16_t stateB )
{
   uint16_t walkingState = stateB;

   while(hsm->states[walkingState].parent != stateA)
   {
      walkingState = hsm->states[walkingState].parent;
   }

   return walkingState;
}
