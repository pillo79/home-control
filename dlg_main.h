#ifndef __DLG_MAIN_H__
#define __DLG_MAIN_H__

#include <QWidget>

#include "ui_dlg_main.h"

class MainDlg : public QWidget
{
	Q_OBJECT

	public:
		MainDlg(QWidget *parent = 0);

	public slots:
		void on_pbNotte_toggled(bool checked);
		void on_pbGiorno_toggled(bool checked);
		void on_pbSoffitta_toggled(bool checked);
		void on_pbFanCoil_toggled(bool checked);

	private:
		Ui::MainDlg ui;
};

#endif /* __DLG_MAIN_H__ */
