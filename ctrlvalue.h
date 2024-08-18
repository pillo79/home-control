#ifndef __CTRLVALUE_H__
#define __CTRLVALUE_H__

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
		QString m_name;
		CtrlObs * m_obs[OBS_COUNT];
		CtrlObs * m_lastObs;

		void notify(bool changed);

	public:
		CtrlVal(QString name);
		virtual ~CtrlVal() { };

		virtual QString format() { return token(); };
		virtual QString token() = 0;
};

class CtrlBoolObs;

class CtrlBoolVal : public CtrlVal {
		friend class CtrlBoolObs;

		bool m_val;
		QString m_trueStr, m_falseStr;

		void setValue(CtrlBoolObs *src, bool val);

	public:
		CtrlBoolVal(QString name, bool val = false, QString trueStr = "true", QString falseStr = "false");
		virtual ~CtrlBoolVal() { };

		CtrlBoolObs &operator ()(bool obsId) { return *(CtrlBoolObs *) m_obs[obsId]; }

		bool value() const { return m_val; };
		operator bool() const { return value(); };

		virtual QString format() { return m_val ? m_trueStr : m_falseStr; };
		virtual QString token() { return m_val ? "true" : "false"; };
};

class CtrlIntObs;

class CtrlIntVal : public CtrlVal {
		friend class CtrlIntObs;

		int m_min, m_max;
		int m_val;
		QString m_fmt;

		void setValue(CtrlIntObs *src, int val);

	public:
		CtrlIntVal(QString name, int min, int max, int val = 0, QString fmt = QString("%d"));
		virtual ~CtrlIntVal() { };

		CtrlIntObs &operator ()(int obsId) { return *(CtrlIntObs *) m_obs[obsId]; }

		int value() const { return m_val; };
		operator int() const { return value(); };

		virtual QString format() { return QString().sprintf(qPrintable(m_fmt), m_val); };
		virtual QString token() { return QString::number(m_val); };
};

class CtrlFloatObs;

class CtrlFloatVal : public CtrlVal {
		friend class CtrlFloatObs;

		double m_min, m_max;
		double m_val;
		QString m_fmt;

		void setValue(CtrlFloatObs *src, double val);

	public:
		CtrlFloatVal(QString name, double min, double max, double val = 0, QString fmt = QString("%.1f"));
		virtual ~CtrlFloatVal() { };

		CtrlFloatObs &operator ()(int obsId) { return *(CtrlFloatObs *) m_obs[obsId]; }

		double value() const { return m_val; };
		operator double() const { return value(); };

		virtual QString format() { return QString().sprintf(qPrintable(m_fmt), m_val); };
		virtual QString token() { return QString::number(m_val, 'e', 6); };
};

#endif /* __CTRLVALUE_H__ */
