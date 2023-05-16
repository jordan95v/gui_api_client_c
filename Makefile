EXE=prog
CC=gcc
CFLAGS= `pkg-config --cflags gtk+-3.0`
GTKLIBS= `pkg-config --libs gtk+-3.0`

$(EXE): main.o curl.o cJSON.o
	$(CC) $(CFLAGS) -o $@ $^ $(GTKLIBS) -lcurl
	rm -rf *.o

main.o: main.c
curl.o: curl.c curl.h

%.o: %.c
	$(CC) $(CFLAGS) -c $<

install_ubuntu:
	apt install gcc libgtk-3-0 libgtk-3-dev -y

install_fedora:
	dnf install gcc gtk3 gtk3-devel -y

clean:
	rm -rf *.o $(EXE)