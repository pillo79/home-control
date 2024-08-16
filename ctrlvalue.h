#ifndef __CTRLVALUE_H__
#define __CTRLVALUE_H__

#include <QMutex>
#include <QString>

enum OBSERVERS {
	O_CONF,
	O_CTRL,
	O_UI_CTRL,
	O_UI_STAT,
	O_NET,
	OBS_COUNT
};

class CtrlObs;

class CtrlVal {
	protected:
		mutable QMutex m_mutex;
		CtrlObs * m_obs[OBS_COUNT];
		CtrlObs * m_lastObs;

		void notify(bool changed);

	public:
		CtrlVal();
		virtual ~CtrlVal() { };

		virtual QString format() = 0;
};

class CtrlBoolObs;

class CtrlBoolVal : public CtrlVal {
		friend class CtrlBoolObs;

		bool m_min, m_max;
		bool m_val;

		void setValue(CtrlBoolObs *src, bool val);

	public:
		CtrlBoolVal(bool val = false);
		virtual ~CtrlBoolVal() { };

		CtrlBoolObs &operator ()(bool obsId) { return *(CtrlBoolObs *) m_obs[obsId]; }

		bool value() const;
		operator bool() const { return value(); };

		virtual QString format();
};

class CtrlIntObs;

class CtrlIntVal : public CtrlVal {
		friend class CtrlIntObs;

		int m_min, m_max;
		int m_val;

		void setValue(CtrlIntObs *src, int val);

	public:
		CtrlIntVal(int min, int max, int val = 0);
		virtual ~CtrlIntVal() { };

		CtrlIntObs &operator ()(int obsId) { return *(CtrlIntObs *) m_obs[obsId]; }

		int value() const;
		operator int() const { return value(); };

		virtual QString format();
};

#endif /* __CTRLVALUE_H__ */
