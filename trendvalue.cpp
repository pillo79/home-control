#include "trendvalue.h"

#include <math.h>

TrendValue::TrendValue(const QString &unit, const QString &fmt, int maxPoints, bool filter)
	: m_unit	(unit)
	, m_fmt		(fmt)
	, m_maxPoints	(maxPoints)
	, m_filter	(filter)
	, m_histMin	(0)
	, m_histMax	(0)
	, m_dataMin	(0)
	, m_dataMax	(0)
	, m_last	(0)
{

}

void TrendValue::setValue(double v)
{
	double topval = fabs(fmax(m_last, v));
	if (!m_filter) {
		m_last = v;
		m_samples.append(v);
	} else if (topval == 0) {
		// add if meaningful
		if (v != 0) {
			m_last = v;
			m_samples.append(v);
		}
	} else {
		// add if not so different from previous value
		int pct = fabs(m_last-v)*100/topval;
		if ((m_last == 0) || (pct < 10)) {
			m_last = v;
			m_samples.append(v);
		}
	}
}

void TrendValue::step(int timecode)
{
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
