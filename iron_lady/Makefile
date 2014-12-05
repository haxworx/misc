PREFIX="/usr/local"

default:
	gcc *.c -g -DPREFIX=\"$(PREFIX)\" -Wall -pedantic -ansi -o ironlady
clean:
	rm ironlady

install:
	-rm -rf $(PREFIX)/share/ironlady
	-mkdir $(PREFIX)/share/ironlady 
	cp *.txt $(PREFIX)/share/ironlady
	cp ironlady $(PREFIX)/share/ironlady
	chmod 444 $(PREFIX)/share/ironlady/*
	chmod 755 $(PREFIX)/share/ironlady/ironlady
	chown root:0 $(PREFIX)/share/ironlady/ironlady
	ln -sf $(PREFIX)/share/ironlady/ironlady $(PREFIX)/bin
	echo "Installation complete."
