#ifndef __TRENDBASE_H__
#define __TRENDBASE_H__

#include <QtCore>
#include "sqlite3.h"

#include "trendvalue.h"

class TrendBase {
	QList<TrendValue *> m_values;

	QString m_filename;
	sqlite3 *m_sqlite;

	explicit TrendBase();

public:
	void registerValue(TrendValue *val);
	int open();
	int step(int timecode);
public:
	static TrendBase *instance();
};

inline TrendBase *TrendBase::instance()
{
	static TrendBase *theBase = new TrendBase();
	return theBase;
}

#endif
