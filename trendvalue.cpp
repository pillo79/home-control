#include "trendvalue.h"
#include "trendbase.h"

#include <math.h>

TrendValue::TrendValue(const QString &name, const QString &unit, const QString &fmt, int maxPoints, double range)
	: m_name	(name)
	, m_unit	(unit)
	, m_fmt		(fmt)
	, m_maxPoints	(maxPoints)
	, m_range	(range)
	, m_histMin	(0)
	, m_histMax	(0)
	, m_dataMin	(0)
	, m_dataMax	(0)
	, m_last	(0)
{
	TrendBase::instance()->registerValue(this);
}

void TrendValue::setValue(double v)
{
	if (m_range == 0.0) {
		// filter is disabled, add everything
		m_last = v;
		m_samples.append(v);
	} else if (m_samples.isEmpty()) {
		// add if meaningful
		if (v != 0) {
			m_last = v;
			m_samples.append(v);
		}
	} else {
		// filter out bad samples
		if (fabs(v-m_last) < m_range) {
			// not so different from previous value, add
			m_last = v;
			m_samples.append(v);
		} else if (v < m_last) {
			// nudge m_last a bit down, but do not add yet
			m_last -= m_range/10.0;
		} else {
			// nudge m_last a bit up, but do not add yet
			m_last += m_range/10.0;
		}
	}
}

void TrendValue::step(int timecode)
{
	// update m_lastPt
	m_lastPt.timecode = timecode;
	if (m_samples.isEmpty()) {
		m_lastPt.min = m_lastPt.max = m_lastPt.mean = 0.0;
	} else {
		qSort(m_samples);
		m_lastPt.min = m_samples.first();
		m_lastPt.max = m_samples.last();
		double mean = 0.0;
		foreach(double v, m_samples)
			mean += v;
		m_lastPt.mean = mean / m_samples.length();
	}

	m_dataPts.enqueue(m_lastPt);
	if (m_dataPts.length() > m_maxPoints)
		m_dataPts.dequeue();

	if (m_lastPt.min < m_histMin)	m_histMin = m_lastPt.min;
	if (m_lastPt.max > m_histMax)	m_histMax = m_lastPt.max;

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
