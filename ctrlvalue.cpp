#include "ctrlvalue.h"
#include "ctrlobserver.h"

#include "state.h"

CtrlVal::CtrlVal(QString name)
	: m_name(name)
	, m_lastObs(NULL)
{
	memset(m_obs, 0, sizeof(m_obs));

	State::registerValue(this);
}

void CtrlVal::notify(bool changed)
{
	for (int i = 0; i<OBS_COUNT; ++i) {
		if (!m_obs[i]) continue;
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

CtrlBoolVal::CtrlBoolVal(QString name, bool val, QString trueStr, QString falseStr)
	: CtrlVal(name)
	, m_val(val)
	, m_trueStr(trueStr)
	, m_falseStr(falseStr)
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

CtrlIntVal::CtrlIntVal(QString name, int min, int max, int val, QString fmt)
	: CtrlVal(name)
	, m_min(min)
	, m_max(max)
	, m_fmt(fmt)
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

CtrlFloatVal::CtrlFloatVal(QString name, double min, double max, double val, QString fmt)
	: CtrlVal(name)
	, m_min(min)
	, m_max(max)
	, m_fmt(fmt)
{
	if (val < m_min) {
		m_val = m_min;
	} else if (val > m_max) {
		m_val = m_max;
	} else {
		m_val = val;
	}

	for (int i = 0; i < OBS_COUNT; ++i) {
		m_obs[i] = new CtrlFloatObs(this, i);
	}
}

void CtrlFloatVal::setValue(CtrlFloatObs *src, double newVal)
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
