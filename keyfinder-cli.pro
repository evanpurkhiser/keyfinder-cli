QT   += core
LIBS += -lkeyfinder
LIBS += -lboost_system
LIBS += -lavcodec
LIBS += -lavformat
LIBS += -lavutil
LIBS += -lavresample

CONFIG += c++11

HEADERS = src/key_notations.h

SOURCES = src/keyfinder_cli.cpp

TARGET  = keyfinder-cli
DESTDIR = build
OBJECTS_DIR = build

