#include "trendctrl.h"
#include "trendbase.h"

#include <math.h>

TrendCtrl::TrendCtrl(const QString &name, int min, int max)
	: m_name	(name)
	, m_min		(min)
	, m_max		(max)
	, m_last	(0)
{
	TrendBase::instance()->registerCtrl(this);
}

void TrendCtrl::setCtrl(int v)
{
	if (v < m_min) {
		m_last = m_min;
	} else if (v > m_max) {
		m_last = m_max;
	} else {
		m_last = v;
	}
}
