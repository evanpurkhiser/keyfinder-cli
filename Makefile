PREFIX=/usr/local

keyfinder-cli: keyfinder_cli.cpp key_notations.h
	mkdir -p bin
	$(CXX) $< -std=c++11 -Wall -lkeyfinder -lavcodec -lavformat -lavutil -lavresample -o $@

install: keyfinder-cli keyfinder-cli.1
	install -Dm 755 keyfinder-cli "${DESTDIR}${PREFIX}/bin/keyfinder-cli"
	install -Dm 644 keyfinder-cli.1 "${DESTDIR}${PREFIX}/share/man/man1/keyfinder-cli.1"

clean:
	rm keyfinder-cli
