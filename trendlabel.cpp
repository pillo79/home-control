#include "trendlabel.h"

#include <QPainter>

TrendLabel::TrendLabel(QWidget *parent)
	: QLabel	(parent)
	, m_value	(NULL)
{

}

void TrendLabel::setValue(TrendValue *value, const QString &fmt, QColor cold, QColor warm)
{
	m_fmt = fmt;
	m_value = value;
	m_cold = cold;
	m_warm = warm;
	update();
}

inline double ratio(double v, double min, double max)
{
	return (v-min)/(max-min);
}

inline int scaleY(double v, double min, double max, int height)
{
	return (int)((height-1)*(1.0-ratio(v, min, max)));
}

inline QColor mix(double r, const QColor &color1, const QColor &color2)
{
	if (r < 0.15)
		r=0.00;
	else if (r>0.85)
		r=1.00;
	else
		r=(r-0.15)/0.70;
	return QColor(
		color1.red()*(1-r) + color2.red()*r,
		color1.green()*(1-r) + color2.green()*r,
		color1.blue()*(1-r) + color2.blue()*r,
		255);
}

void TrendLabel::paintEvent(QPaintEvent *event)
{
	if (!m_value) {
		QLabel::paintEvent(event);
		return;
	}

	const QQueue<DataPt> &pts = m_value->dataPts();
	const QRect geo = geometry();

	QPainter p;
	p.begin(this);

	int imin, xmin;
	if (geo.width() > pts.length()) {
		imin = 0;
		xmin = geo.width()-pts.length();
	} else {
		imin = pts.length()-geo.width();
		xmin = 0;
	}

	for (int x=xmin, i=imin; x<geo.width(); ++x, ++i) {
		const DataPt &pt = pts[i];
		int y_top = scaleY(pt.min, m_value->dataMin(), m_value->dataMax(), geo.height());
		int y_mean = scaleY(pt.mean, m_value->dataMin(), m_value->dataMax(), geo.height());
		int y_bot = scaleY(pt.max, m_value->dataMin(), m_value->dataMax(), geo.height());

		// draw x axis step
		if (!(pt.timecode % 60)) {
			p.setPen(QColor(0,0,0));
			p.drawLine(x, height()-5, x, height());
		}

		// draw graph
		double r = ratio(pt.mean, m_value->dataMin(), m_value->dataMax());
		QColor c = mix(r, m_cold, m_warm);
		p.setPen(c);
		p.drawLine(x, y_top, x, y_bot);
		p.setPen(c.darker(250));
		p.drawLine(x, y_mean-1, x, y_mean+1);
	}

	p.end();
	QLabel::paintEvent(event);
}

