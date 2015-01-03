LIBS = -lkeyfinder -lboost_system -lavcodec -lavformat -lavutil -lavresample

bin/keyfinder-cli: src/keyfinder_cli.cpp src/key_notations.h
	mkdir -p bin
	g++ -std=c++11 -Wall $(LIBS) -o $@ $^

clean:
	rm -r bin
