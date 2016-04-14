#ifndef __ST_X_H__
#define __ST_X_H__

void   ST_X_Entry( event_t ev );
void   ST_X_Exit( event_t ev );
bool_t ST_X_EV_X_GuardX(event_t ev );
void   ST_X_EV_X_ActionX( event_t ev );
uint16_t ST_X_PickSubstate(event_t ev);

#endif