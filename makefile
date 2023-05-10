CC = gcc
OBJS = HackEnrollment.o main.o
EXEC = HackEnrollment
DEBUG_FLAG = -g
DIR = /new_home/courses/mtm/public/2223b/ex1
CFLAGS = -std=c99 -lm -I$(DIR) -Itool -Wall -pedantic-errors -Werror -DNDEBUG
COMP_TOOL = $(CC) $(DEBUG_FLAG) $(CFLAGS) -c tool/$*.c -o $@

program: $(OBJS)
	$(CC) $(DEBUG_FLAG) $(CFLAGS) $(OBJS) -o $(EXEC)

IsraeliQueue.o: IsraeliQueue.c

HackEnrollment.o: tool/HackEnrollment.c tool/HackEnrollment.h
	$(COMP_TOOL)

main.o: tool/HackEnrollment.h
	$(COMP_TOOL)

clean:
	rm -f $(OBJS) $(EXEC)
