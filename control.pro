TEMPLATE = app
TARGET = control
DEPENDPATH += .
INCLUDEPATH += . \
	../libmodbus/src/

# Input
HEADERS += hardware.h modbusdevice.h modbusio.h plc.h control.h
SOURCES += main.cpp plc.cpp hardware.cpp modbusdevice.cpp modbusio.cpp control.cpp
LIBS += -L../libmodbus/src -lmodbus

HEADERS += dlg_main.h
SOURCES += dlg_main.cpp
FORMS += dlg_main.ui
