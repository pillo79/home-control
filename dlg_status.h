#ifndef __DLG_STATUS_H__
#define __DLG_STATUS_H__

#include <QWidget>
#include <QTimer>

#include "ui_dlg_status.h"

class StatusDlg : public QWidget
{
	Q_OBJECT

	public:
		StatusDlg(QWidget *parent = 0, QWidget *controlDlg = 0);

	public slots:
		void on_pbSetNotte_toggled(bool checked);
		void on_pbSetGiorno_toggled(bool checked);
		void on_pbSetSoffitta_toggled(bool checked);

		void updateScreen();

	private:
		Ui::StatusDlg ui;
		QWidget *m_controlDlg;
		QWidget *m_alarmDlg;

		void updateStatoRisc();
};

#endif /* __DLG_STATUS_H__ */
