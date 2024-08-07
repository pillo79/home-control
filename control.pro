TEMPLATE = app
TARGET = control
DEPENDPATH += . \
	sqlite-amalgamation-3090200/
INCLUDEPATH += . \
	../libmodbus/src/ \
	sqlite-amalgamation-3090200/

RESOURCES += control.qrc

# Input
HEADERS += hardware.h modbusdevice.h modbusio.h plc_lib.h powercalc.h control.h
SOURCES += main.cpp plc_lib.cpp hardware.cpp modbusdevice.cpp modbusio.cpp powercalc.cpp control.cpp
LIBS += -L../libmodbus/src -lmodbus

HEADERS += trendlabel.h trendvalue.h trendbase.h
SOURCES += trendlabel.cpp trendvalue.cpp trendbase.cpp
SOURCES += sqlite3.c

HEADERS += dlg_control.h
SOURCES += dlg_control.cpp
FORMS += dlg_control.ui

HEADERS += dlg_status.h
SOURCES += dlg_status.cpp
FORMS += dlg_status.ui

HEADERS += dlg_alarm.h
SOURCES += dlg_alarm.cpp
FORMS += dlg_alarm.ui

