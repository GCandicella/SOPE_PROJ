all: main.c
	gcc main.c -o simpledu

clean:
	rm *.o simpledu