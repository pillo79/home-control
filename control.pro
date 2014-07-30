TEMPLATE = app
TARGET = control
DEPENDPATH += .
INCLUDEPATH += . \
	../libmodbus/src/

# Input
HEADERS += hardware.h modbusdevice.h modbusio.h plc_lib.h control.h
SOURCES += main.cpp plc_lib.cpp hardware.cpp modbusdevice.cpp modbusio.cpp control.cpp
LIBS += -L../libmodbus/src -lmodbus

HEADERS += dlg_control.h
SOURCES += dlg_control.cpp
FORMS += dlg_control.ui

HEADERS += dlg_status.h
SOURCES += dlg_status.cpp
FORMS += dlg_status.ui

