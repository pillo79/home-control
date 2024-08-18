#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <QThread>
#include <QMutex>
#include <QTime>

#include "trendvalue.h"

#include "ctrlvalue.h"

class ControlThread : public QThread {
	private:
		void setPowerLevel(int level);
		void run();
	public:
		ControlThread();
		virtual ~ControlThread();
};

ControlThread &control();

#endif /* __CONTROL_H__ */
