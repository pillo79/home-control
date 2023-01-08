#include "trendlabel.h"

#include <QPainter>
#include <cmath>

TrendLabel::TrendLabel(QWidget *parent)
	: QLabel	(parent)
	, m_value	(NULL)
	, m_yGridStep	(0.0)
{

}

void TrendLabel::setValue(TrendValue *value, QColor cold, QColor warm)
{
	m_value = value;
	m_cold = cold;
	m_warm = warm;
	update();
}

void TrendLabel::setYGrid(double step)
{
	m_yGridStep = step;
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

	QPainter p;
	p.begin(this);

	int imin, xmin;
	if (width() > pts.length()) {
		imin = 0;
		xmin = width()-pts.length();
	} else {
		imin = pts.length()-width();
		xmin = 0;
	}

	if (m_yGridStep) {
		p.setPen(QColor(96, 96, 96));
		double y = m_value->dataMin();
		modf(y/m_yGridStep, &y);
		y *= m_yGridStep;
		while (y<m_value->dataMax()) {
			int y_grid = scaleY(y, m_value->dataMin(), m_value->dataMax(), height());
			p.drawLine(0, y_grid, width(), y_grid);
			y += m_yGridStep/2;
			int y_tick = scaleY(y, m_value->dataMin(), m_value->dataMax(), height());
			p.drawLine(0, y_tick, 5, y_tick);
			p.drawLine(width()-5, y_tick, width(), y_tick);
			y += m_yGridStep/2;
		}
	}

	// evaluate limits
	QFont tinyFont = p.font();
	tinyFont.setPixelSize(10);
	p.setFont(tinyFont);

	QString yMaxStr = m_value->format(m_value->dataMax());
	QString yMinStr = m_value->format(m_value->dataMin());

	int yLabelWidth = 0;
	QSize sz;
	QFontMetrics fm = p.fontMetrics();
	sz = fm.size(0, yMaxStr); if (sz.width() > yLabelWidth) yLabelWidth = sz.width();
	sz = fm.size(0, yMinStr); if (sz.width() > yLabelWidth) yLabelWidth = sz.width();
	++yLabelWidth;

	// draw y axis limits
	p.setPen(QColor(96, 96, 96));
	p.drawText(width()-yLabelWidth, 0, yLabelWidth, 15, Qt::AlignRight | Qt::AlignTop, yMaxStr);
	p.drawText(width()-yLabelWidth, height()-16, yLabelWidth, 15, Qt::AlignRight | Qt::AlignBottom, yMinStr);

	// draw x axis labels up to a certain point
	for (int x=xmin, i=imin; x<width(); ++x, ++i) {
		const DataPt &pt = pts[i];
		if (!(pt.timecode % 60)) {
			QString hour = QString::number((pt.timecode/60) % 24);
			sz = fm.size(0, hour);
			if (x+sz.width()/2 < width()-yLabelWidth)
				p.drawText(x-16, height()-16, 30, 15, Qt::AlignHCenter | Qt::AlignBottom, hour);
		}
	}

	// draw graph
	for (int x=xmin, i=imin; x<width(); ++x, ++i) {
		const DataPt &pt = pts[i];
		int y_top = scaleY(pt.min, m_value->dataMin(), m_value->dataMax(), height());
		int y_mean = scaleY(pt.mean, m_value->dataMin(), m_value->dataMax(), height());
		int y_bot = scaleY(pt.max, m_value->dataMin(), m_value->dataMax(), height());

		double r = ratio(pt.mean, m_value->dataMin(), m_value->dataMax());
		QColor c = mix(r, m_cold, m_warm);
		p.setPen(c);
		p.drawLine(x, y_top, x, y_bot);
		p.setPen(c.darker(150));
		p.drawLine(x, y_mean-1, x, y_mean+1);
	}

	p.end();
	QLabel::paintEvent(event);
}

