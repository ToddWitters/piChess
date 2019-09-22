#include "types.h"

#ifndef SWITCH_H
#define SWITCH_H

typedef void (*cbPtr_t)(int sq, bool_t state);

void switchInit( void );

void switchPoll ( void );

// Revert to blank board (debounced switch states all reset to zero) and begin debouncing
void StartSwitchPoll( void );

// Stop switch polling (all debounce states and sample values are left in tact)
void StopSwitchPoll( void );

// Continue polling, picking up with existng debounce states in tact (but counters all reset)
void ResumeSwitchPoll( void );

// Return all debounced  reed switch states in a 64-bit value
uint64_t GetSwitchStates ( void );

// Register for a callback function
void SetSwitchCallback(cbPtr_t cb);

void setButtonRepeat(uint8_t delay, uint8_t interval);

void SW_flipBoard( void );

#endif
