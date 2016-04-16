void inGameEntry( event_t ev );
void inGameExit( event_t ev );
uint16_t inGamePickSubstate( event_t ev);

extern game_t game;

void inGame_moveClockTick( event_t ev);
void inGame_SetPosition( const char *FEN);
