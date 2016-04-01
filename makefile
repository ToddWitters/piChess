DEFS = -DDEBUG_OUTPUT

CC = gcc

CFLAGS = $(DEFS) -Wall

sources = bcm2835.c      \
			 bitboard.c     \
			 board.c        \
			 constants.c    \
			 diag.c         \
			 display.c      \
          event.c        \
			 eventManager.c \
			 gpio.c         \
			 i2c.c          \
			 led.c          \
			 main.c         \
			 menu.c         \
			 moves.c        \
		    options.c      \
			 sfInterface.c  \
			 specChars.c    \
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


