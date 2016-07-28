#include "hsm.h"
#include "hsmDefs.h"
#include "st_timeOptionMenu.h"
#include "options.h"
#include "display.h"
#include "stdio.h"
#include "string.h"
#include "event.h"
#include "switch.h"
#include "diag.h"

uint8_t activeLine;

typedef enum timeSettingSubstate_e
{
   SUBSTATE_STATUS,        // Displaying Status
   SUBSTATE_TIME_OPTION,   // Selecting scheme
   SUBSTATE_EVEN,         // Period 1 setting
   SUBSTATE_ODDS,          // Odds settings
   SUBSTATE_UNTIMED,       // Computer strategy setting
}timeSettingSubstate_t;

timeSettingSubstate_t subState;
uint8_t pickRow;
uint8_t period;

static void drawStatusScreen( void );
static void drawEvenScreen( uint8_t period );
static void drawOddsScreen( void );
static void drawUntimedScreen( void );

static char *createPeriodSummary( periodTimingSettings_t *settings );

static void displayTotalTime( int min, int line );
static void displayIncrement( int sec, int line );
static void displayMoves( int moves );

void timeOptionMenuEntry( event_t ev )
{
   subState = SUBSTATE_STATUS;
   displayClear();
   drawStatusScreen();
}

void timeOptionMenuExit( event_t ev )
{
   displayClear();
}

void timeOptionMenuButtonHandler( event_t ev)
{

   switch(ev.ev)
   {
      case EV_BUTTON_STATE:
         switch(subState)
         {
            case SUBSTATE_TIME_OPTION:
               switch(pickRow)
               {
                  case 1:
                  default:
                     period = 1;
                     subState = SUBSTATE_EVEN;
                     drawEvenScreen(period);
                     break;
                  case 2:
                     subState = SUBSTATE_ODDS;
                     pickRow = 0;
                     drawOddsScreen();
                     break;
                  case 3:
                     subState = SUBSTATE_UNTIMED;
                     pickRow = 1;
                     drawUntimedScreen();
                     break;
               }
               break;

            case SUBSTATE_EVEN:
               if( (period < 3) && options.game.timeControl.timeSettings[period-1].moves != 0)
               {
                  pickRow=1;
                  drawEvenScreen(++period);
               }
               else
               {
                  subState = SUBSTATE_STATUS;
                  drawStatusScreen();
               }
               break;
            case SUBSTATE_ODDS:
            case SUBSTATE_UNTIMED:
                subState = SUBSTATE_STATUS;
               drawStatusScreen();
               break;


            default:
               break;
         }
         break;

      case EV_BUTTON_POS:
         switch(subState)
         {
            case SUBSTATE_STATUS:

               if(ev.data == POS_LEFT)
               {
                  event_t outputEvent = { EV_GOTO_OPTION_MENU, 0};
                  putEvent(EVQ_EVENT_MANAGER, &outputEvent);
               }
               else if(ev.data == POS_RIGHT)
               {
                  subState = SUBSTATE_TIME_OPTION;

                  displayClear();
                  displayWriteLine(0, "--- Time Options ---", TRUE);
                  displayWriteLine(1, " Equal Times", FALSE);
                  displayWriteLine(2, " Time Odds", FALSE);
                  displayWriteLine(3, " No Time", FALSE);

                  switch(options.game.timeControl.type)
                  {
                     case TIME_EQUAL: pickRow = 1; break;
                     case TIME_ODDS:  pickRow = 2; break;
                     case TIME_NONE:  pickRow = 3; break;
                  }
                  displayWriteChars(pickRow,0,1,">");
               }
               break;

            case SUBSTATE_TIME_OPTION:
               switch(ev.data)
               {
                  case POS_LEFT:
                  case POS_UP:
                     if(pickRow > 1)
                     {
                        displayWriteChars(pickRow,0,1," ");
                        pickRow--;
                        displayWriteChars(pickRow,0,1,">");
                     }
                     break;
                  case POS_RIGHT:
                  case POS_DOWN:
                     if(pickRow < 3)
                     {
                        displayWriteChars(pickRow,0,1," ");
                        pickRow++;
                        displayWriteChars(pickRow,0,1,">");
                     }
                     break;
               }
               break;

            case SUBSTATE_EVEN:
               switch(ev.data)
               {
                  case POS_RIGHT:
                     {

                        int  min;

                        switch(pickRow)
                        {
                           case 1:
                              // Intervals are 1-30   in increments of 1
                              //              30-120  in increments of 5
                              //             120-240  in increments of 15

                              min = options.game.timeControl.timeSettings[period-1].totalTime / 60;

                              if (min < 30) {
                                 min++;
                              } else if (min < 120) {
                                 min += 5;
                              } else if (min < 240) {
                                 min += 15;
                              } else {
                                 break;
                              }

                              options.game.timeControl.timeSettings[period-1].totalTime = min * 60;

                              displayTotalTime(min, 1);

                              break;

                           case 2:

                              if(options.game.timeControl.timeSettings[period-1].increment < 99)
                              {
                                 displayIncrement(++options.game.timeControl.timeSettings[period-1].increment, 2);
                              }
                              break;

                           case 3:

                              if(options.game.timeControl.timeSettings[period-1].increment < 99)
                              {
                                 displayMoves(++options.game.timeControl.timeSettings[period-1].moves);
                              }
                              break;
                        }
                     }
                     break;

                  case POS_LEFT:
                     {
                        int min;

                        switch(pickRow)
                        {
                           case 1:
                           // Intervals are 1-30   in increments of 1
                           //              30-120  in increments of 5
                           //             120-240  in increments of 15

                           min = options.game.timeControl.timeSettings[period-1].totalTime / 60;

                           if (min > 120) {
                              min -= 15;
                           } else if (min > 30) {
                              min -= 5;
                           } else if (min > 1) {
                              min--;
                           } else {
                              break;
                           }

                           options.game.timeControl.timeSettings[period-1].totalTime = min * 60;

                           displayTotalTime(min, 1);

                           break;

                           case 2:
                              if(options.game.timeControl.timeSettings[period-1].increment > 0 )
                              {
                                 displayIncrement(--options.game.timeControl.timeSettings[period-1].increment, 2);
                              }
                              break;
                           case 3:
                              if(options.game.timeControl.timeSettings[period-1].moves > 0 )
                              {
                                 displayMoves(--options.game.timeControl.timeSettings[period-1].moves);
                              }
                              break;
                        }

                     }
                     break;
                  case POS_UP:
                     if(pickRow > 1)
                     {
                        displayWriteChars(pickRow,0,1," ");
                        pickRow--;
                        displayWriteChars(pickRow,0,1,">");
                     }
                     break;
                  case POS_DOWN:
                     if(pickRow < 2 || (pickRow < 3 && (period != 3)))
                     {
                        displayWriteChars(pickRow,0,1," ");
                        pickRow++;
                        displayWriteChars(pickRow,0,1,">");
                     }
                     break;
               }
               break;

         case SUBSTATE_ODDS:
            switch(ev.data)
            {
               case POS_RIGHT:
                  {

                     int  min;

                     switch(pickRow)
                     {
                        case 0:
                        case 2:
                           // Intervals are 1-30   in increments of 1
                           //              30-120  in increments of 5
                           //             120-240  in increments of 15

                           min = options.game.timeControl.timeSettings[pickRow==0?WHITE:BLACK].totalTime / 60;

                           if (min < 30) {
                              min++;
                           } else if (min < 120) {
                              min += 5;
                           } else if (min < 240) {
                              min += 15;
                           } else {
                              break;
                           }

                           options.game.timeControl.timeSettings[pickRow==0?WHITE:BLACK].totalTime = min * 60;

                           displayTotalTime(min, pickRow);

                           break;

                        case 1:
                        case 3:

                           if(options.game.timeControl.timeSettings[pickRow==1?WHITE:BLACK].increment < 99)
                           {
                              displayIncrement(++options.game.timeControl.timeSettings[pickRow==1?WHITE:BLACK].increment, pickRow);
                           }
                           break;
                     }
                  }
                  break;

               case POS_LEFT:
                  {
                     int min;

                     switch(pickRow)
                     {
                        case 0:
                        case 2:
                           // Intervals are 1-30   in increments of 1
                           //              30-120  in increments of 5
                           //             120-240  in increments of 15

                           min = options.game.timeControl.timeSettings[pickRow==0 ? WHITE : BLACK].totalTime / 60;

                           if (min > 120) {
                              min -= 15;
                           } else if (min > 30) {
                              min -= 5;
                           } else if (min > 0) {
                              min--;
                           } else {
                              break;
                           }

                           options.game.timeControl.timeSettings[pickRow==0?WHITE:BLACK].totalTime = min * 60;

                           displayTotalTime(min, pickRow);

                           break;

                        case 1:
                        case 3:

                           if(options.game.timeControl.timeSettings[pickRow==1?WHITE:BLACK].increment > 0)
                           {
                              displayIncrement(--options.game.timeControl.timeSettings[pickRow==1?WHITE:BLACK].increment, pickRow);
                           }
                           break;
                     }

                  }
                  break;
               case POS_UP:
                  if(pickRow > 0)
                  {
                     displayWriteChars(pickRow,0,1," ");
                     pickRow--;
                     displayWriteChars(pickRow,0,1,">");
                  }
                  break;
               case POS_DOWN:
                  if(pickRow < 3)
                  {
                     displayWriteChars(pickRow,0,1," ");
                     pickRow++;
                     displayWriteChars(pickRow,0,1,">");
                  }
                  break;
            }
            break;

            case SUBSTATE_UNTIMED:
               switch(ev.data)
               {
                  case POS_UP:
                  case POS_DOWN:
                     if( (ev.data == POS_DOWN && pickRow < 3) ||
                         (ev.data == POS_UP  && pickRow > 0 ) )
                     {
                        displayWriteChars(pickRow,0,1," ");
                        displayWriteChars(pickRow,17,3,"   ");

                        if(ev.data == POS_DOWN) pickRow++; else pickRow--;

                        displayWriteChars(pickRow,0,1,">");

                        if(pickRow == 1)
                        {
                           char temp[4];
                           sprintf(temp,"%3d",options.game.timeControl.compStrategySetting.depth);
                           displayWriteChars(pickRow,17,3,temp);
                        }
                        else if (pickRow == 2)
                        {
                           char temp[4];
                           sprintf(temp,"%3d",options.game.timeControl.compStrategySetting.timeInMs / 1000);
                           displayWriteChars(pickRow,17,3,temp);
                        }
                     }
                     break;

                 case POS_LEFT:
                     if(pickRow == 1)
                     {
                        if(options.game.timeControl.compStrategySetting.depth > 4)
                        {
                           char temp[4];
                           sprintf(temp,"%3d",--options.game.timeControl.compStrategySetting.depth);
                           displayWriteChars(1,17,3,temp);
                       }
                     }
                     else if(pickRow == 2)
                     {
                        if(options.game.timeControl.compStrategySetting.timeInMs > 1000)
                        {
                           char temp[4];
                           options.game.timeControl.compStrategySetting.timeInMs -= 1000;
                           sprintf(temp,"%3d",options.game.timeControl.compStrategySetting.timeInMs / 1000);
                           displayWriteChars(2,17,3,temp);
                        }
                     }
                     break;

                 case POS_RIGHT:
                     if(pickRow == 1)
                     {
                        if(options.game.timeControl.compStrategySetting.depth < 30)
                        {
                           char temp[4];
                           sprintf(temp,"%3d",++options.game.timeControl.compStrategySetting.depth);
                           displayWriteChars(1,17,3,temp);
                       }
                     }
                     else if(pickRow == 2)
                     {
                        if(options.game.timeControl.compStrategySetting.timeInMs < 999 * 1000)
                        {
                           char temp[4];
                           options.game.timeControl.compStrategySetting.timeInMs += 1000;
                           sprintf(temp,"%3d",options.game.timeControl.compStrategySetting.timeInMs / 1000);
                           displayWriteChars(2,17,3,temp);
                        }
                     }
                     break;
               }
               break;

            default:
               break;
         }
         break;

      default:
         break;
   }
}

static void drawStatusScreen( void )
{
   char lineString[3][21];
   char tempString[21];
   int numPeriods = 1;

   displayClear();

   switch(options.game.timeControl.type)
   {
      case TIME_NONE:
         displayWriteLine(0, "Untimed Game", TRUE);
         displayWriteLine(1, "Computer will search", TRUE);
         switch(options.game.timeControl.compStrategySetting.type)
         {
            case STRAT_FIXED_TIME:
               sprintf(tempString,"for %d seconds", options.game.timeControl.compStrategySetting.timeInMs / 1000);
               break;
            case STRAT_FIXED_DEPTH:
               sprintf(tempString,"for %d ply", options.game.timeControl.compStrategySetting.depth);
               break;
            case STRAT_TILL_BUTTON:
               sprintf(tempString,"until button press");
               break;
         }
         displayWriteLine(2, tempString, TRUE);
         break;

      case TIME_EQUAL:

         strcpy(lineString[0], createPeriodSummary(&options.game.timeControl.timeSettings[0]));

         if(options.game.timeControl.timeSettings[0].moves != 0)
         {

            numPeriods++;
            strcpy(lineString[1], createPeriodSummary(&options.game.timeControl.timeSettings[1]));

            if(options.game.timeControl.timeSettings[1].moves != 0)
            {
               numPeriods++;
               options.game.timeControl.timeSettings[2].moves = 0;
               strcpy(lineString[2], createPeriodSummary(&options.game.timeControl.timeSettings[2]));

            }
         }

         switch(numPeriods)
         {
            case 1:
               displayWriteLine(0, "Time Controls", TRUE);
               displayWriteLine(1, lineString[0], TRUE);
               break;
            case 2:
               displayWriteLine(0, "Time Controls", TRUE);
               displayWriteLine(1, lineString[0], TRUE);
               displayWriteLine(2, lineString[1], TRUE);
               break;
            case 3:
               displayWriteLine(0, lineString[0], TRUE);
               displayWriteLine(1, lineString[1], TRUE);
               displayWriteLine(2, lineString[2], TRUE);
               break;
         }

         break;

      case TIME_ODDS:

         displayWriteLine(0, "Time Odds Game", TRUE);

         options.game.timeControl.timeSettings[WHITE].moves = 0;
         options.game.timeControl.timeSettings[BLACK].moves = 0;

         strcpy(lineString[WHITE],"W: ");
         strcat(lineString[WHITE], createPeriodSummary(&options.game.timeControl.timeSettings[WHITE]));

         strcpy(lineString[BLACK],"B: ");
         strcat(lineString[BLACK], createPeriodSummary(&options.game.timeControl.timeSettings[BLACK]));

         displayWriteLine(1,lineString[WHITE],FALSE);
         displayWriteLine(2,lineString[BLACK],FALSE);

         break;
   }

   displayWriteLine(3, "< ok        change >", TRUE);
}

static void drawEvenScreen( uint8_t period )
{
   char lineText[21];

   if( period > 3 ) return;

   displayClear();

   if(period == 1)
   {
      displayWriteLine(0,"-- Time Settings ---", FALSE);
   }
   else
   {
      sprintf(lineText,"Setting for period %d", period);
      displayWriteLine(0,lineText,FALSE);
   }

   sprintf(lineText,">Time:");
   displayWriteLine(1,lineText, FALSE);
   displayTotalTime(options.game.timeControl.timeSettings[period-1].totalTime/60, 1);

   sprintf(lineText," Increment:");
   displayWriteLine(2,lineText , FALSE);
   displayIncrement(options.game.timeControl.timeSettings[period-1].increment, 2);

   if(period != 3)
   {
      sprintf(lineText," Moves (0=SD):");
      displayWriteLine(3,lineText, FALSE);
      displayMoves(options.game.timeControl.timeSettings[period-1].moves);
   }

}

static void drawOddsScreen( void )
{
   char lineText[21];

   displayClear();

   sprintf(lineText,">W Time:");
   displayWriteLine(0,lineText, FALSE);
   displayTotalTime(options.game.timeControl.timeSettings[WHITE].totalTime/60, 0);

   sprintf(lineText," W Increment:");
   displayWriteLine(1,lineText, FALSE);
   displayIncrement(options.game.timeControl.timeSettings[WHITE].increment, 1);

   sprintf(lineText," B Time:");
   displayWriteLine(2,lineText, FALSE);
   displayTotalTime(options.game.timeControl.timeSettings[BLACK].totalTime/60, 2);

   sprintf(lineText," B Increment:");
   displayWriteLine(3,lineText, FALSE);
   displayIncrement(options.game.timeControl.timeSettings[BLACK].increment, 3);


}

static void drawUntimedScreen( void )
{
   char lineText[21];
   displayClear();

   displayWriteLine(0,"-Computer Strategy--", FALSE);

   if(options.game.timeControl.compStrategySetting.type == STRAT_FIXED_DEPTH)
   {
      sprintf(lineText, ">Fixed Depth(ply) %2d", options.game.timeControl.compStrategySetting.depth);
      pickRow = 1;
   }
   else
   {
      sprintf(lineText, " Fixed Depth(ply)   ");
   }
   displayWriteLine(1,lineText, FALSE);

   if(options.game.timeControl.compStrategySetting.type == STRAT_FIXED_TIME)
   {
      sprintf(lineText, ">Fixed Time(sec) %3d", options.game.timeControl.compStrategySetting.timeInMs / 1000);
      pickRow = 2;
   }
   else
   {
      sprintf(lineText, " Fixed Time(sec)    ");
   }
   displayWriteLine(2,lineText, FALSE);

   if(options.game.timeControl.compStrategySetting.type == STRAT_TILL_BUTTON)
   {
      displayWriteLine(3,">Until Button", FALSE);
      pickRow = 3;
   }
   else
   {
      displayWriteLine(3," Until Button", FALSE);
   }
}


static char *createPeriodSummary( periodTimingSettings_t *settings )
{
   static char summaryString[21];

   char tempString[21];
   char preface[10];
   int hour,min,sec;

   hour = (settings->totalTime / 3600);
   min  = (settings->totalTime % 3600) / 60;
   sec  = (settings->totalTime % 60);

   if(settings->moves == 0)
   {
      sprintf(preface,"G/");
   }
   else
   {
      sprintf(preface,"%dmv/",settings->moves);
   }

   if(hour)
   {
      if(min)
      {
         if(sec)
         {
            sprintf(summaryString, "%s%d:%02d:%02d", preface, hour, min, sec);
         }
         else
         {
            sprintf(summaryString, "%s%dhr%s %dmin", preface, hour, (hour>1?"s":""), min);
         }
      }
      else
      {
         if(sec)
         {
            sprintf(summaryString, "%s%d:%02d:%02d", preface, hour, min, sec);
         }
         else
         {
            sprintf(summaryString, "%s%dhr%s", preface, hour, (hour>1?"s":""));
         }
      }
   }
   else
   {
      if(min)
      {
         if(sec)
         {
            sprintf(summaryString, "%s%dmin%s %dsec", preface, min, (min>1?"s":""), sec);
         }
         else
         {
            sprintf(summaryString, "%s%dmin%s", preface, min, (min>1?"s":""));
         }
      }
      else
      {
         sprintf(summaryString, "%s%dsec%s", preface, sec, (sec>1?"s":""));
      }
   }

   if(settings->increment != 0)
   {
      sprintf(tempString, " +%ds", settings->increment);
      strcat(summaryString, tempString);
   }

   return summaryString;

}


static void displayTotalTime( int min, int line )
{

   char numString[21];

   if(min < 60)
   {
      sprintf(numString,"%dm",min);
   }
   else if (min%60 != 0)
   {
      sprintf(numString,"%dh%dm",min/60,min%60);
   }
   else
   {
      sprintf(numString,"%dh",min/60);
   }
   displayWriteChars( line, 15, 5, "     ");

   displayWriteChars( line, 20-strlen(numString), strlen(numString), numString );

}

static void displayIncrement( int sec, int line)
{
   char numString[21];

   sprintf(numString,"%ds",sec);

   displayWriteChars( line, 15, 4, "     ");

   displayWriteChars( line, 20-strlen(numString), strlen(numString), numString );

}

static void displayMoves( int moves )
{
   char numString[21];

   sprintf(numString,"%d",moves);

   displayWriteChars( 3, 15, 4, "     ");

   displayWriteChars( 3, 20-strlen(numString), strlen(numString), numString );

}
