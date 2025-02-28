#ifndef __CTRLOBSERVER_H__
#define __CTRLOBSERVER_H__

#include "ctrlvalue.h"

#include <QObject>

class CtrlObs : public QObject {
		Q_OBJECT
		friend class CtrlVal;

		int m_obsId;

	public:
		CtrlObs(int obsId);
		virtual ~CtrlObs() { };

	signals:
		void updated();		// after any call to setValue
		void updatedByOthers(); // after calls by other obs to setValue
		void changed();		// after the value changes
		void changedByOthers();	// after a value change by other obs
};

class CtrlBoolObs : public CtrlObs {
		CtrlBoolVal *m_val;

	public:
		CtrlBoolObs(CtrlBoolVal *val, int obsId);
		virtual ~CtrlBoolObs() { };

		bool value() const { return m_val->value(); }
		operator bool() const { return value(); }

		void setValue(bool newVal) { m_val->setValue(this, newVal); }
		bool operator =(bool newVal) { setValue(newVal); return value(); }
};

class CtrlIntObs : public CtrlObs {
		CtrlIntVal *m_val;

	public:
		CtrlIntObs(CtrlIntVal *val, int obsId);
		virtual ~CtrlIntObs() { };

		int value() const { return m_val->value(); }
		operator int() const { return value(); }

		void setValue(int newVal) { m_val->setValue(this, newVal); }
		int operator =(int newVal) { setValue(newVal); return value(); }
		int operator +=(int delta) { setValue(value() + delta); return value(); }
		int operator -=(int delta) { setValue(value() - delta); return value(); }
};

class CtrlFloatObs : public CtrlObs {
		CtrlFloatVal *m_val;

	public:
		CtrlFloatObs(CtrlFloatVal *val, int obsId);
		virtual ~CtrlFloatObs() { };

		double value() const { return m_val->value(); }
		operator double() const { return value(); }

		void setValue(double newVal) { m_val->setValue(this, newVal); }
		double operator =(double newVal) { setValue(newVal); return value(); }
		double operator +=(double delta) { setValue(value() + delta); return value(); }
		double operator -=(double delta) { setValue(value() - delta); return value(); }
};

inline CtrlObs::CtrlObs(int obsId)
	: QObject()
	, m_obsId(obsId)
{

}

inline CtrlBoolObs::CtrlBoolObs(CtrlBoolVal *val, int obsId)
	: CtrlObs(obsId)
	, m_val(val)
{

}

inline CtrlIntObs::CtrlIntObs(CtrlIntVal *val, int obsId)
	: CtrlObs(obsId)
	, m_val(val)
{

}

inline CtrlFloatObs::CtrlFloatObs(CtrlFloatVal *val, int obsId)
	: CtrlObs(obsId)
	, m_val(val)
{

}

#endif /* __CTRLOBSERVER_H__ */
