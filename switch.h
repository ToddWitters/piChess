#include "types.h"

#ifndef SWITCH_H
#define SWITCH_H

typedef enum buttonPos_e
{
   POS_CENTER,
   POS_RIGHT,
   POS_LEFT,
   POS_DOWN,
   POS_UP,
   POS_UP_RIGHT,
   POS_UP_LEFT,
   POS_DOWN_RIGHT,
   POS_DOWN_LEFT,
   POS_ERROR
}buttonPos_t;

typedef enum buttonPress_e
{
   B_RELEASED,
   B_PRESSED
}buttonPress_t;


typedef void (*cbPtr_t)(int sq, bool_t state);

void switchInit( void );

void switchPoll ( void );

// Revert to blank board (debounced switch states all reset to zero) and begin debouncing
void StartSwitchPoll( void );

// Stop switch polling (all debounce states and sample values are left in tact)
void StopSwitchPoll( void );

// Continue polling, picking up with existng debounce states in tact (but counters all reset)
void ResumeSwitchPoll( void );

// Return all debounced switch states in a 64-bit value
uint64_t GetSwitchStates ( void );

// Register for a callback function
void SetSwitchCallback(cbPtr_t cb);

void setButtonRepeat(uint8_t delay, uint8_t interval);

#endif
