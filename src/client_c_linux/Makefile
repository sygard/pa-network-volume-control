 CC=gcc
 CFLAGS= -lm -O3 -g -lX11
 LDFLAGS=
 EXECUTABLE=pa-nvc
 HEADERFILES=volumecontrol.h link.h
 SOURCES=volumecontrol.c link.c
 OBJECTS=$(SOURCES:.c=.o)

all: $(EXECUTABLE) $(HEADERFILES) $(SOURCES)

$(EXECUTABLE): $(OBJECTS)
	    $(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c
	    $(CC) $(CFLAGS) -c -o $@ $<

clean:
	@$(RM) -rf *.o
	@$(RM) $(EXECUTABLE)

