#include "trendvalue.h"

TrendValue::TrendValue(const QString &unit, int maxPoints)
	: m_unit	(unit)
	, m_maxPoints	(maxPoints)
{

}

void TrendValue::setValue(double v)
{
	m_last = v;

	if (!m_sampleCount) {
		m_dataPt.min = m_dataPt.max = m_dataPt.mean = v;
	} else {
		if (v < m_dataPt.min)	m_dataPt.min = v;
		if (v > m_dataPt.max)	m_dataPt.max = v;
		m_dataPt.mean += v;
	}
	++m_sampleCount;
}

void TrendValue::step()
{
	if (!m_sampleCount) {
		m_dataPt.min = m_dataPt.max = m_dataPt.mean = 0.0;
	} else {
		m_dataPt.mean /= m_sampleCount;
	}
	m_dataPts.enqueue(m_dataPt);
	if (m_dataPts.length() > m_maxPoints)
		m_dataPts.dequeue();

	if (m_dataPt.min < m_histMin)	m_histMin = m_dataPt.min;
	if (m_dataPt.max > m_histMax)	m_histMax = m_dataPt.max;

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

	m_sampleCount = 0;
}
