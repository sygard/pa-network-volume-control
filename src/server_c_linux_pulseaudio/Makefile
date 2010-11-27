CC=gcc
CFLAGS=-O3 -g -Wall -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  -lpulse-mainloop-glib -lpulse -lglib-2.0
LDFLAGS=-I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include  -lpulse-mainloop-glib -lpulse -lglib-2.0
EXECUTABLE=volumecontrols
HEADERFILES=volumecontrols.h link.h mixer.h
SOURCES=volumecontrols.c link.c mixer.c
OBJECTS=$(SOURCES:.c=.o)

all: $(EXECUTABLE) 

$(EXECUTABLE): $(OBJECTS) $(HEADERFILES) $(SOURCES)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@$(RM) -rf *.o
	@$(RM) $(EXECUTABLE)

