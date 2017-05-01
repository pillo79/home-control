#ifndef __TRENDVALUE_H__
#define __TRENDVALUE_H__

#include <QQueue>
#include <QString>

typedef struct {
	double min, mean, max;
	int timecode;
} DataPt;

class TrendValue {
		QString m_unit;
		int m_maxPoints;
		QQueue<DataPt> m_dataPts;
		QList<float> m_samples;

		double m_histMin;
		double m_histMax;
		double m_dataMin;
		double m_dataMax;
		double m_last;

	public:
		TrendValue(const QString &unit, int maxPoints);
		const QString &unit()		{ return m_unit; }
		int maxPoints()			{ return m_maxPoints; }

		double value() const		{ return m_last; }
		operator double() const		{ return m_last; }

		void setValue(double v);
		double operator=(double v)	{ setValue(v); return m_last; }

		void step(int timecode);

		const QQueue<DataPt> &dataPts() { return m_dataPts; }
		double dataMin() const		{ return m_dataMin; }
		double dataMax() const		{ return m_dataMax; }
		double histMin() const		{ return m_histMin; }
		double histMax() const		{ return m_histMax; }
};

#endif /* __TRENDVALUE_H__ */
