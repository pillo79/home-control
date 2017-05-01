#include "trendvalue.h"

TrendValue::TrendValue(const QString &unit, int maxPoints)
	: m_unit	(unit)
	, m_maxPoints	(maxPoints)
{

}

void TrendValue::setValue(double v)
{
	m_last = v;
	m_samples.append(v);
}

void TrendValue::step(int timecode)
{
	qSort(m_samples);
	if (!m_samples.isEmpty()) m_samples.removeFirst();
	if (!m_samples.isEmpty()) m_samples.removeLast();

	DataPt newPt;
	if (m_samples.isEmpty()) {
		newPt.min = newPt.max = newPt.mean = 0.0;
	} else {
		newPt.min = m_samples.first();
		newPt.max = m_samples.last();
		double mean = 0.0;
		foreach(double v, m_samples)
			mean += v;
		newPt.mean = mean / m_samples.length();
	}
	newPt.timecode = timecode;
	m_dataPts.enqueue(newPt);
	if (m_dataPts.length() > m_maxPoints)
		m_dataPts.dequeue();

	if (newPt.min < m_histMin)	m_histMin = newPt.min;
	if (newPt.max > m_histMax)	m_histMax = newPt.max;

	bool first = true;
	foreach (const DataPt &p, m_dataPts) {
		if (first) {
			m_dataMin = p.min;
			m_dataMax = p.max;
			first = false;
		} else {
			if (p.min < m_dataMin)	m_dataMin = p.min;
			if (p.max > m_dataMax)	m_dataMax = p.max;
		}
	}

	m_samples.clear();
}
