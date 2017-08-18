#include "trendbase.h"

#define DB_PATH "/media/mmcblk0p2"

TrendBase::TrendBase()
{
	m_sqlite = NULL;
}

void TrendBase::registerValue(TrendValue *val)
{
	m_values.append(val);
}

int TrendBase::open()
{
	m_filename = QString(DB_PATH "/%1.db").arg(QDate::currentDate().year());
	bool createTable = !QFile::exists(m_filename);

	int rc = sqlite3_open(qPrintable(m_filename), &m_sqlite);
	if (rc) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_sqlite));
		return rc;
	}

	if (createTable) {
		QString q = "CREATE TABLE 'data' ('timecode' INTEGER PRIMARY KEY";
		foreach(TrendValue *v, m_values) {
			q += QString(", 'min_%1' REAL").arg(v->name());
			q += QString(", 'val_%1' REAL").arg(v->name());
			q += QString(", 'max_%1' REAL").arg(v->name());
		}
		q += QString(")");

		rc = sqlite3_exec(m_sqlite, qPrintable(q), NULL, NULL, NULL);
		if (rc) {
			fprintf(stderr, "Can't create database: %s\n", sqlite3_errmsg(m_sqlite));
			return rc;
		}
	}

	return rc;
}

int TrendBase::step(int timecode)
{
	QString names = "'timecode'";
	QString values = QString::number(timecode);

	foreach(TrendValue *v, m_values) {
		const DataPt &d = v->lastPt();
		names += QString(", 'min_%1'").arg(v->name());
		values += QString(", %1").arg(d.min, 0, 'e', 6);
		names += QString(", 'val_%1'").arg(v->name());
		values += QString(", %1").arg(d.mean, 0, 'e', 6);
		names += QString(", 'max_%1'").arg(v->name());
		values += QString(", %1").arg(d.max, 0, 'e', 6);
	}

	QString q = QString("INSERT INTO 'data' (%1) VALUES (%2)").arg(names, values);
	int rc = sqlite3_exec(m_sqlite, qPrintable(q), NULL, NULL, NULL);
	if (rc) {
		fprintf(stderr, "Can't insert in database: %s\n", sqlite3_errmsg(m_sqlite));
		return rc;
	}

	return rc;
}
