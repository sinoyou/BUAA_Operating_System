INCLUDES	:= -I./ -I../include/

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean

compile:
	$(CC) $(INCLUDES) *.c 

testmain: compile
	$(CC) -o testmain *.o


clean:
	rm -rf *~ *.o 
	rm testmain

# include ../include.mk
