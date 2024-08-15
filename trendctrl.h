#ifndef __TRENDCTRL_H__
#define __TRENDCTRL_H__

#include <QQueue>
#include <QString>

class TrendCtrl {
		QString m_name;

	protected:
		int m_min, m_max, m_last;

	public:
		TrendCtrl(const QString &name, int min, int max);
		virtual ~TrendCtrl() { };

		const QString &name()		{ return m_name; }

		int value() const		{ return m_last; }
		operator int() const		{ return m_last; }

		void setCtrl(int v);
		int operator=(int v)	{ setCtrl(v); return m_last; }

		virtual QString format() = 0;
};

class TrendCBool : public TrendCtrl {
	public:
		TrendCBool(const QString &name) : TrendCtrl(name, 0, 1) { };
		virtual ~TrendCBool() { };

		virtual QString format() { return QString("%1").arg(m_last ? "true" : false); }
};

class TrendCInt : public TrendCtrl {
	public:
		TrendCInt(const QString &name, int min, int max) : TrendCtrl(name, min, max) { };
		virtual ~TrendCInt() { };

		virtual QString format() { return QString("%1").arg(m_last); }
};

#endif /* __TRENDCTRL_H__ */
