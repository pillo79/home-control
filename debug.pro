TEMPLATE = app
TARGET = debug
DEPENDPATH += .
INCLUDEPATH += . \
	../libmodbus/src/

# Input
SOURCES += debug.cpp
LIBS += -L../libmodbus/src -lmodbus
