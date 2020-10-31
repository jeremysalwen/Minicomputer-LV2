OBJECTS = minicomputer.o
LIBRARY = libminicomputer.so
TTLS = minicomputer.ttl manifest.ttl
CC = gcc
CFLAGS += -std=c99 -Wall -Wextra -fPIC
INSTALLDIR = $(DESTDIR)/usr/lib/lv2/
INSTALLNAME = minicomputer.lv2/
$(LIBRARY) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -shared -o $@

.SUFFIXES : .c .o

.c.o:
	$(CC) $(CFLAGS) -c $<

minicomputer.o: minicomputer.c
.PHONY : clean install uninstall

clean :
	rm -f $(LIBRARY) $(OBJECTS)

install :
	mkdir -p $(INSTALLDIR)$(INSTALLNAME)
	install $(LIBRARY) $(INSTALLDIR)$(INSTALLNAME)
	install -m 644 $(TTLS) $(INSTALLDIR)$(INSTALLNAME)

uninstall :
	rm -rf $(INSTALLDIR)$(INSTALLNAME)
