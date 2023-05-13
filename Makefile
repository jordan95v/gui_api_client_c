EXE=prog
CC=gcc
CFLAGS= `pkg-config --cflags gtk+-3.0`
GTKLIBS= `pkg-config --libs gtk+-3.0`

$(EXE): main.o
	$(CC) $(CFLAGS) -o $@ $^ $(GTKLIBS)
	rm -rf *.o

main.o: main.c

%.o: %.c
	$(CC) $(CFLAGS) -c $<

install:
	apt install gcc libgtk-4-1 libgtk-4-dev -y

uninstall:
	apt remove libgtk-4-1 libgtk-4-dev -y

clean:
	rm -rf *.o $(EXE)