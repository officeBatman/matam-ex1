CC = gcc
OBJS = IsraeliQueue.o HackEnrollment.o main.o
EXEC = HackEnrollment
DEBUG_FLAG = -g
DIR = /new_home/courses/mtm/public/2223b/ex1
CFLAGS = -std=c99 -lm -I$(DIR) -Itool -Wall -pedantic-errors -Werror -DNDEBUG
COMP_TOOL = $(CC) $(DEBUG_FLAG) $(CFLAGS) -c tool/$*.c -o $@

program: $(OBJS)
	$(CC) $(DEBUG_FLAG) $(CFLAGS) $(OBJS) -o $(EXEC)

IsraeliQueue.o: IsraeliQueue.h IsraeliQueue.c

HackEnrollment.o: tool/HackEnrollment.c tool/HackEnrollment.h IsraeliQueue.h
	$(COMP_TOOL)

main.o: tool/main.h
	$(COMP_TOOL)

clean:
	rm -f $(OBJS) $(EXEC)
