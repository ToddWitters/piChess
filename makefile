DEFS = -DDEBUG_OUTPUT

CC = gcc

CFLAGS = $(DEFS) -Wall

sources = bcm2835.c      \
			 bitboard.c     \
			 board.c        \
			 book.c         \
			 constants.c    \
			 diag.c         \
			 display.c      \
          event.c        \
			 gpio.c         \
			 hsm.c          \
			 hsmDefs.c      \
			 i2c.c          \
			 led.c          \
			 main.c         \
			 menu.c         \
			 moves.c        \
		    options.c      \
			 sfInterface.c  \
			 specChars.c    \
			 st_diagMenu.c  \
			 st_diagSwitch.c \
			 st_mainMenu.c  \
			 st_splashScreen.c \
			 st_menus.c     \
			 st_top.c       \
			 st_initPosSetup.c \
			 st_arbPosSetup.c \
			 st_inGame.c  \
			 st_playingGame.c \
			 st_optionMenu.c \
			 st_gameOptionMenu.c \
			 st_boardOptionMenu.c \
			 st_engineOptionMenu.c \
			 st_playerMove.c \
			 st_computerMove.c \
			 st_moveForComputer.c \
			 st_exitingGame.c \
			 st_inGameMenu.c \
			 st_timeOptionMenu.c \
			 st_fixBoard.c \
			 switch.c       \
			 timer.c        \
			 util.c         \
			 zobrist.c

objects = $(sources:.c=.o)

#default rule
piChess : $(objects)
	gcc -o piChess -pthread $(objects)

#Create header dependencies automatically...
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

#include header dependencies
include $(sources:.c=.d)

clean:
	rm piChess $(objects)
