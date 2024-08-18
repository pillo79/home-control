#ifndef __TRENDVALUE_H__
#define __TRENDVALUE_H__

#include "ctrlvalue.h"

#include <QQueue>
#include <QString>

typedef struct {
	double min, mean, max;
	int timecode;
} DataPt;

class TrendValue : public CtrlFloatVal {
		QString m_unit;
		int m_maxPoints;
		double m_range;

		DataPt m_lastPt;
		QQueue<DataPt> m_dataPts;
		QList<float> m_samples;

		double m_histMin;
		double m_histMax;
		double m_dataMin;
		double m_dataMax;

	public:
		TrendValue(const QString &name, const QString &unit, const QString &fmt, int maxPoints, double range);
		virtual ~TrendValue() { }

		const QString &unit()		{ return m_unit; }
		int maxPoints()			{ return m_maxPoints; }

		void setValue(double v);
		double operator=(double v)	{ setValue(v); return value(); }

		void step(int timecode);

		const QQueue<DataPt> &dataPts() { return m_dataPts; }
		const DataPt &lastPt() const	{ return m_lastPt; }
		double dataMin() const		{ return m_dataMin; }
		double dataMax() const		{ return m_dataMax; }
		double histMin() const		{ return m_histMin; }
		double histMax() const		{ return m_histMax; }

		QString format(double value) const	{ return QString().sprintf(qPrintable(m_fmt), value); }
		virtual QString format() const		{ return format(value()); }
};

#endif /* __TRENDVALUE_H__ */
