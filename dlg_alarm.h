#ifndef __DLG_ALARM_H__
#define __DLG_ALARM_H__

#include <QWidget>

#include "ui_dlg_alarm.h"

class AlarmDlg : public QWidget
{
//	Q_OBJECT

	public:
		AlarmDlg(QWidget *parent = 0);

	private:
		Ui::AlarmDlg ui;
};

#endif /* __DLG_ALARM_H__ */
