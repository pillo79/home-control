#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <QThread>
#include <QMutex>

class ControlThread : public QThread {
	private:
		void run();
	public:
		ControlThread();
		~ControlThread();

		QMutex mFields;

		bool xRiscaldaNotte;
		bool xRiscaldaGiorno;
		bool xRiscaldaSoffitta;
		bool xFanCoil;

		bool xUsaPompaCalore;
		bool xUsaCaldaia;
};

ControlThread &control();

#endif /* __CONTROL_H__ */
