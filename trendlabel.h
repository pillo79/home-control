#ifndef __TRENDLABEL_H__
#define __TRENDLABEL_H__

#include <QLabel>
#include <QColor>
#include "trendvalue.h"

class TrendLabel : public QLabel
{
		QString m_fmt;
		TrendValue *m_value;
		QColor m_cold, m_warm;
		double m_yGridStep;

		virtual void paintEvent(QPaintEvent *event);
	public:
		TrendLabel(QWidget *parent);

		void setValue(TrendValue *value, QColor cold, QColor warm);
		void setYGrid(double step);
};

#endif /* __TRENDLABEL_H__ */
