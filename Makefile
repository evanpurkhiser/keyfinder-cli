PREFIX = /usr/local
MANDIR = ${PREFIX}/share/man
CXXFLAGS != pkg-config --cflags --libs fftw3 libavcodec libavformat libavutil libswresample libkeyfinder

all: keyfinder-cli

keyfinder-cli: keyfinder_cli.cpp key_notations.h
	${CXX} keyfinder_cli.cpp -std=c++11 -Wall ${CXXFLAGS} -o $@

install: keyfinder-cli keyfinder-cli.1
	install -d "${DESTDIR}${PREFIX}/bin"
	install -m 755 keyfinder-cli "${DESTDIR}${PREFIX}/bin/keyfinder-cli"
	install -d "${DESTDIR}${MANDIR}/man1"
	install -m 644 keyfinder-cli.1 "${DESTDIR}${MANDIR}/man1/keyfinder-cli.1"

clean:
	rm -f keyfinder-cli
