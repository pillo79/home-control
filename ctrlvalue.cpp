#include "ctrlvalue.h"
#include "ctrlobserver.h"

CtrlVal::CtrlVal()
	: m_lastObs(NULL)
{
	memset(m_obs, 0, sizeof(m_obs));
}

void CtrlVal::notify(bool changed)
{
	for (int i = 0; i<OBS_COUNT; ++i) {
		emit m_obs[i]->updated();
		if (changed)
			emit m_obs[i]->changed();

		if (m_obs[i] != m_lastObs) {
			emit m_obs[i]->updatedByOthers();
			if (changed)
				emit m_obs[i]->changedByOthers();
		}
	}
}

CtrlBoolVal::CtrlBoolVal(bool val)
	: CtrlVal()
	, m_val(val)
{
	for (int i = 0; i < OBS_COUNT; ++i) {
		m_obs[i] = new CtrlBoolObs(this, i);
	}
}

void CtrlBoolVal::setValue(CtrlBoolObs *src, bool newVal)
{
	bool changed = false;

	if (newVal != m_val) {
		changed = true;
		m_val = newVal;
	}
	m_lastObs = src;

	notify(changed);
}

QString CtrlBoolVal::format()
{
	return QString(m_val ? "true" : "false");
}

CtrlIntVal::CtrlIntVal(int min, int max, int val)
	: CtrlVal()
	, m_min(min)
	, m_max(max)
{
	if (val < m_min) {
		m_val = m_min;
	} else if (val > m_max) {
		m_val = m_max;
	} else {
		m_val = val;
	}

	for (int i = 0; i < OBS_COUNT; ++i) {
		m_obs[i] = new CtrlIntObs(this, i);
	}
}

void CtrlIntVal::setValue(CtrlIntObs *src, int newVal)
{
	bool changed = false;

	if (newVal < m_min) {
		newVal = m_min;
	} else if (newVal > m_max) {
		newVal = m_max;
	}

	if (newVal != m_val) {
		changed = true;
		m_val = newVal;
	}
	m_lastObs = src;

	notify(changed);
}

QString CtrlIntVal::format()
{
	return QString::number(m_val);
}

