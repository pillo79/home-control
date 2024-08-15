#ifndef __TRENDBASE_H__
#define __TRENDBASE_H__

#include <QtCore>
#include "sqlite3.h"

#include "trendvalue.h"
#include "trendctrl.h"

typedef QList<DataPt> DataList;
typedef QMap<QString, DataList> DataMap;

class TrendBase {
	QList<TrendValue *> m_values;
	QList<TrendCtrl *> m_ctrls;

	QString m_filename;
	sqlite3 *m_sqlite;

	explicit TrendBase();

	DataMap getRecords(int first_code, int last_code, QStringList values = QStringList());

public:
	void registerValue(TrendValue *val);
	void registerCtrl(TrendCtrl *ctrl);
	int open();
	int step(int epoch);
public:
	static TrendBase *instance();
};

inline TrendBase *TrendBase::instance()
{
	static TrendBase *theBase = new TrendBase();
	return theBase;
}

#endif
