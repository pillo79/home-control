TEMPLATE = app
TARGET = control
DEPENDPATH += .
INCLUDEPATH += . \
	../libmodbus/src/

# Input
HEADERS += hardware.h modbusdevice.h modbusio.h plc.h
SOURCES += main.cpp plc.cpp hardware.cpp modbusdevice.cpp modbusio.cpp
LIBS += -L../libmodbus/src -lmodbus

HEADERS += dlg_main.h
SOURCES += dlg_main.cpp

