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
#include "engine.h"

uint8_t activeLine;
extern game_t game;

typedef enum timeSettingSubstate_e
{
   SUBSTATE_STATUS,        // Displaying Status
   SUBSTATE_TIME_OPTION,   // Selecting scheme
   SUBSTATE_EVEN,          // Period 1 setting
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

void timeOptionNavButtonHandler( event_t ev)
{

   switch(ev.ev)
   {
      case EV_BUTTON_CENTER:
         switch(subState)
         {
            case SUBSTATE_TIME_OPTION:
               switch(pickRow)
               {
                  case 1:
                  default:
                     setOptionStr("timeControl", "equal");
                     period = 1;
                     subState = SUBSTATE_EVEN;
                     drawEvenScreen(period);
                     break;
                  case 2:
                     setOptionStr("timeControl", "odds");
                     subState = SUBSTATE_ODDS;
                     pickRow = 0;
                     drawOddsScreen();
                     break;
                  case 3:
                     setOptionStr("timeControl", "untimed");
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

      case EV_BUTTON_LEFT:
         switch(subState)
         {
            case SUBSTATE_STATUS:
               {
                  event_t outputEvent = { EV_GOTO_OPTION_MENU, 0};
                  putEvent(EVQ_EVENT_MANAGER, &outputEvent);
               }
               break;

            case SUBSTATE_TIME_OPTION:
               break;

            case SUBSTATE_EVEN:
            {

                  int  min;

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
                        } else {
                           min--;
                        }

                        options.game.timeControl.timeSettings[period-1].totalTime = min * 60;

                        displayTotalTime(min, 1);

                        break;

                     case 2:

                        if(options.game.timeControl.timeSettings[period-1].increment != 0)
                        {
                           displayIncrement(--options.game.timeControl.timeSettings[period-1].increment, 2);
                        }
                        break;

                     case 3:

                        if(options.game.timeControl.timeSettings[period-1].moves != 0)
                        {
                           displayMoves(--options.game.timeControl.timeSettings[period-1].moves);
                        }
                        break;
                  }

               break;
            }

            case SUBSTATE_ODDS:
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

            case SUBSTATE_UNTIMED:
               if(pickRow == 1)
               {
                  long int depth = getOptionVal("searchDepth");
                  if(depth > MIN_PLY_DEPTH)
                  {
                     char temp[4];
                     setOptionVal("searchDepth",depth-1);
                     sprintf(temp,"%3ld",depth-1);
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

         }
         break;

      case EV_BUTTON_RIGHT:
         switch(subState)
         {
            case SUBSTATE_STATUS:
               {
                  subState = SUBSTATE_TIME_OPTION;

                  displayClear();
                  displayWriteLine(0, "--- Time Options ---", TRUE);
                  displayWriteLine(1, " Equal Times", FALSE);
                  displayWriteLine(2, " Time Odds", FALSE);
                  displayWriteLine(3, " No Time", FALSE);

                  if(isOptionStr("timeControl", "equal"))
                     pickRow = 1;
                  else if(isOptionStr("timeControl", "odds"))
                     pickRow = 2;
                  else if(isOptionStr("timeControl", "untimed"))
                     pickRow = 3;

                  displayWriteChars(pickRow,0,1,">");
               }

               break;

            case SUBSTATE_TIME_OPTION:
                break;

            case SUBSTATE_EVEN:
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

                        if(options.game.timeControl.timeSettings[period-1].moves < 99)
                        {
                           displayMoves(++options.game.timeControl.timeSettings[period-1].moves);
                        }
                        break;
                  }
               }

               break;

            case SUBSTATE_ODDS:

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

            case SUBSTATE_UNTIMED:
               if(pickRow == 1)
               {

                  long int depth = getOptionVal("searchDepth");

                  if(depth < MAX_PLY_DEPTH)
                  {
                     char temp[4];
                     setOptionVal("searchDepth",depth+1);
                     sprintf(temp,"%3ld",depth+1);
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

      case EV_BUTTON_UP:
         switch(subState)
         {
            case SUBSTATE_STATUS:
                break;

            case SUBSTATE_TIME_OPTION:
               if(pickRow > 1)
               {
                  displayWriteChars(pickRow,0,1," ");
                  pickRow--;
                  displayWriteChars(pickRow,0,1,">");
               }

               break;

            case SUBSTATE_EVEN:

               if(pickRow > 1)
               {
                  displayWriteChars(pickRow,0,1," ");
                  pickRow--;
                  displayWriteChars(pickRow,0,1,">");
               }
               break;

            case SUBSTATE_ODDS:
               if(pickRow > 0)
               {
                  displayWriteChars(pickRow,0,1," ");
                  pickRow--;
                  displayWriteChars(pickRow,0,1,">");
               }
               break;

            case SUBSTATE_UNTIMED:
               if( pickRow > 0 )
               {
                  displayWriteChars(pickRow,0,1," ");
                  displayWriteChars(pickRow,17,3,"   ");

                  pickRow--;

                  displayWriteChars(pickRow,0,1,">");

                  if(pickRow == 1)
                  {
                     char temp[4];
                     setOptionStr("computerStrategy", "fixedDepth");
                     sprintf(temp,"%3ld",getOptionVal("searchDepth"));
                     displayWriteChars(pickRow,17,3,temp);
                  }
                  else if (pickRow == 2)
                  {
                     char temp[4];
                     setOptionStr("computerStrategy", "fixedTime");
                     sprintf(temp,"%3d",options.game.timeControl.compStrategySetting.timeInMs / 1000);
                     displayWriteChars(pickRow,17,3,temp);
                  }
                  else
                  {
                     setOptionStr("computerStrategy", "tillButton");
                  }
               }
               break;


         }
         break;

      case EV_BUTTON_DOWN:
         switch(subState)
         {
            case SUBSTATE_STATUS:
                break;

            case SUBSTATE_TIME_OPTION:
               if(pickRow < 3)
               {
                  displayWriteChars(pickRow,0,1," ");
                  pickRow++;
                  displayWriteChars(pickRow,0,1,">");
               }
               break;

            case SUBSTATE_EVEN:
               if(pickRow < 2 || (pickRow < 3 && (period != 3)))
               {
                  displayWriteChars(pickRow,0,1," ");
                  pickRow++;
                  displayWriteChars(pickRow,0,1,">");
               }

               break;

            case SUBSTATE_ODDS:
               if(pickRow < 3)
               {
                  displayWriteChars(pickRow,0,1," ");
                  pickRow++;
                  displayWriteChars(pickRow,0,1,">");
               }

               break;

            case SUBSTATE_UNTIMED:

               if( pickRow < 3 )
               {
                  displayWriteChars(pickRow,0,1," ");
                  displayWriteChars(pickRow,17,3,"   ");

                  pickRow++;

                  displayWriteChars(pickRow,0,1,">");

                  if(pickRow == 1)
                  {
                     char temp[4];
                     setOptionStr("computerStrategy", "fixedDepth");
                     sprintf(temp,"%3ld",getOptionVal("searchDepth"));
                     displayWriteChars(pickRow,17,3,temp);
                  }
                  else if (pickRow == 2)
                  {
                     char temp[4];
                     setOptionStr("computerStrategy", "fixedTime");
                     sprintf(temp,"%3d",options.game.timeControl.compStrategySetting.timeInMs / 1000);
                     displayWriteChars(pickRow,17,3,temp);
                  }
                  else
                  {
                     setOptionStr("computerStrategy", "tillButton");

                  }
               }

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


   if(isOptionStr("timeControl", "untimed"))
   {
         displayWriteLine(0, "Untimed Game", TRUE);
         displayWriteLine(1, "Computer will search", TRUE);

         if     (isOptionStr("computerStrategy", "fixedTime"))
            sprintf(tempString,"for %d seconds", options.game.timeControl.compStrategySetting.timeInMs / 1000);
         else if(isOptionStr("computerStrategy", "fixedDepth"))
            sprintf(tempString,"for %ld ply", getOptionVal("searchDepth"));
         else if(isOptionStr("computerStrategy", "tillButton"))
            sprintf(tempString,"until button press");
         else
            sprintf(tempString, "???");

         displayWriteLine(2, tempString, TRUE);
   }
   else if(isOptionStr("timeControl", "equal"))
   {

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

   }
   else if(isOptionStr("timeControl", "odds"))
   {

         displayWriteLine(0, "Time Odds Game", TRUE);

         options.game.timeControl.timeSettings[WHITE].moves = 0;
         options.game.timeControl.timeSettings[BLACK].moves = 0;

         strcpy(lineString[WHITE],"W: ");
         strcat(lineString[WHITE], createPeriodSummary(&options.game.timeControl.timeSettings[WHITE]));

         strcpy(lineString[BLACK],"B: ");
         strcat(lineString[BLACK], createPeriodSummary(&options.game.timeControl.timeSettings[BLACK]));

         displayWriteLine(1,lineString[WHITE],FALSE);
         displayWriteLine(2,lineString[BLACK],FALSE);


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

   if(isOptionStr("computerStrategy", "fixedDepth"))
   {
      sprintf(lineText, ">Fixed Depth(ply) %2ld", getOptionVal("searchDepth"));
      pickRow = 1;
   }
   else
   {
      sprintf(lineText, " Fixed Depth(ply)   ");
   }
   displayWriteLine(1,lineText, FALSE);

   if(isOptionStr("computerStrategy", "fixedTime"))
   {
      sprintf(lineText, ">Fixed Time(sec) %3d", options.game.timeControl.compStrategySetting.timeInMs / 1000);
      pickRow = 2;
   }
   else
   {
      sprintf(lineText, " Fixed Time(sec)    ");
   }
   displayWriteLine(2,lineText, FALSE);

   if(isOptionStr("computerStrategy", "tillButton"))
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
