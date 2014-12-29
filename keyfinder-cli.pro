QT   += core
LIBS += -lkeyfinder
LIBS += -lboost_system
LIBS += -lavcodec
LIBS += -lavformat
LIBS += -lavutil

CONFIG += c++11

SOURCES = src/keyfinder_cli.cpp

TARGET  = keyfinder-cli
DESTDIR = build
OBJECTS_DIR = build

