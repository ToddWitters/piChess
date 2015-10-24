#ifndef EVENT_H
#define EVENT_H

#include <semaphore.h>

#include "types.h"

#define EVENT_QUEUE_SIZE 10

typedef enum eQueIndex_e
{
    EVQ_EVENT_MANAGER,

    EVQ_TOTAL
}evQueueIndex_t;

typedef enum event_e
{
   EV_PIECE_DROP,    // Reed switch closed
   EV_PIECE_LIFT,    // Reed switch opened
   EV_BUTTON_POS,    // Nav button has changed position
   EV_BUTTON_STATE,  // Nav button press/release
   EV_MENU_PICK,     // Menu item picked
   EV_ENGINE,        // Chess engine event
   EV_TIMER,         // Timer elapsed
}event_t;

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

typedef enum menuEvent_e
{
   M_EV_BACK = -1,
   M_EV_IGNORED,
   M_EV_START_GAME,
   M_EV_LOAD_GAME,
   M_EV_GAME_SETTINGS,
   M_EV_TIME_SETTINGS,
   M_EV_PLAYERS,
   M_EV_HUMAN_WHITE,
   M_EV_HUMAN_BLACK,
   M_EV_HUMAN_BOTH,
   M_EV_SETUP_POS,
   M_EV_DIAGNOSTICS,
   M_EV_DIAG_SWITCHES,
   M_EV_DIAG_JOYSTICK,
   M_EV_DIAG_DISPLAY,
   M_EV_DIAG_LEDS,
   M_EV_ABOUT,

}menuEvent_t;

typedef enum buttonPress_e
{
   B_RELEASED,
   B_PRESSED
}buttonPress_t;


typedef enum engineEvent_e
{
   ENG_MOVE
}engineEvent_t;



typedef struct eventData_t
{
   event_t ev;     // The event
   int     param;  // param depends upon event...
}eventData_t;

void         initEvent( void );
void         putEvent(evQueueIndex_t indx, eventData_t *evData);
eventData_t *getEvent( evQueueIndex_t indx );
void         clearEvents( evQueueIndex_t indx );
sem_t        *getQueueSem(evQueueIndex_t indx);


#endif
