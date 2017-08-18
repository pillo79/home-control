#ifndef __TRENDVALUE_H__
#define __TRENDVALUE_H__

#include <QQueue>
#include <QString>

typedef struct {
	double min, mean, max;
	int timecode;
} DataPt;

class TrendValue {
		QString m_name;
		QString m_unit;
		QString m_fmt;
		int m_maxPoints;
		bool m_filter;

		DataPt m_lastPt;
		QQueue<DataPt> m_dataPts;
		QList<float> m_samples;

		double m_histMin;
		double m_histMax;
		double m_dataMin;
		double m_dataMax;
		double m_last;

	public:
		TrendValue(const QString &name, const QString &unit, const QString &fmt, int maxPoints, bool filter);
		const QString &name()		{ return m_name; }
		const QString &unit()		{ return m_unit; }
		int maxPoints()			{ return m_maxPoints; }

		double value() const		{ return m_last; }
		operator double() const		{ return m_last; }

		void setValue(double v);
		double operator=(double v)	{ setValue(v); return m_last; }

		void step(int timecode);

		const QQueue<DataPt> &dataPts() { return m_dataPts; }
		const DataPt &lastPt() const	{ return m_lastPt; }
		double dataMin() const		{ return m_dataMin; }
		double dataMax() const		{ return m_dataMax; }
		double histMin() const		{ return m_histMin; }
		double histMax() const		{ return m_histMax; }

		QString format(double value)	{ return QString().sprintf(qPrintable(m_fmt), value); }
		QString format()		{ return format(m_last); }
};

#endif /* __TRENDVALUE_H__ */
