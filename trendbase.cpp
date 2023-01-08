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

int TrendBase::step(int epoch)
{
	QString names = "'timecode'";
	QString values = QString::number(epoch);

	int timecode = epoch/60;
	foreach(TrendValue *v, m_values) {
		v->step(timecode);
	}

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

static int getCols_callback(void *ptr, int ncols, char **values, char **names)
{
	Q_UNUSED(ncols);
	Q_UNUSED(names);
	QString *colNames = (QString *) ptr;

	QString col = QString(values[1]);
	if (col != "timecode")
		colNames->append(QString(", %1").arg(col));

	return 0;
}

static int getRecords_callback(void *ptr, int ncols, char **values, char **names)
{
	QMap<QString, DataPt> dataPts;

	DataMap *map = (DataMap*) ptr;

	int timecode = -1;
	for (int i=0; i<ncols; ++i)
	{
		QString col = QString(names[i]);
		if (col == "timecode") {
			timecode = QString(values[i]).toInt()/60;
		} else {
			QString t = col.left(3);
			QString n = col.mid(5);

			DataPt &p = dataPts[n];
			if (t == "max") {
				p.max = QString(values[i]).toDouble();
			} else if (t == "min") {
				p.min = QString(values[i]).toDouble();
			} else {
				p.mean = QString(values[i]).toDouble();
			}
		}
	}

	QMap<QString, DataPt>::iterator it = dataPts.begin();
	while (it != dataPts.end()) {
		DataPt &p = it.value();
		p.timecode = timecode;
		(*map)[it.key()].append(p);
		++it;
	}

	return 0;
}

DataMap TrendBase::getRecords(int first_code, int last_code, QStringList varNames)
{
	QString colNames = "'timecode'";
	if (varNames.isEmpty()) {
		QString q = QString("PRAGMA table_info('data')");
		int rc = sqlite3_exec(m_sqlite, qPrintable(q), &getCols_callback, &colNames, NULL);
		if (rc) {
			fprintf(stderr, "Can't get table info from database: %s\n", sqlite3_errmsg(m_sqlite));
		}
	} else {
		foreach(const QString &n, varNames) {
			colNames += QString(", 'min_%1'").arg(n);
			colNames += QString(", 'val_%1'").arg(n);
			colNames += QString(", 'max_%1'").arg(n);
		}
	}

	QString limits = "TRUE";
	if (first_code > 0)
		limits += QString(" AND ('timecode' >= %1)").arg(first_code);
	if (last_code > 0)
		limits += QString(" AND ('timecode' <= %1)").arg(last_code);

	DataMap result;

	QString q = QString("SELECT (%1) FROM 'data' WHERE (%2) ORDER BY 'timecode' ASC").arg(colNames, limits);
	int rc = sqlite3_exec(m_sqlite, qPrintable(q), &getRecords_callback, &result, NULL);
	if (rc) {
		fprintf(stderr, "Can't select from database: %s\n", sqlite3_errmsg(m_sqlite));
	}

	return result;
}
