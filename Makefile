LDLIBS = -lcurses   # curses

all: beatnik

run:
	./beatnik

beatnik: beatnik.o
	gcc -o $@ $< $(LDLIBS) 

c.o:
	gcc -c $<
