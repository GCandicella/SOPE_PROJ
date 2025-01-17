CC=gcc
CFLAGS=-Wall -lm
DEPS=main.h
OBJ=main.o

simpledu: $(OBJ)
	@ $(CC) -o $@ $^ $(CFLAGS)

%.o: %.c $(DEPS)
	@ $(CC) -c -o $@ $< $(CFLAGS)

clean:
	@ rm *.o simpledu