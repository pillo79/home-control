#ifndef __DLG_MAIN_H__
#define __DLG_MAIN_H__

#include <QWidget>
#include <QTimer>

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

		void on_pbRiscCaldaia_toggled(bool checked);
		void on_pbRiscPompaCalore_toggled(bool checked);

		void on_pbVelMinus_clicked();
		void on_pbVelPlus_clicked();

		void on_pbTrasfAccumulo_toggled(bool checked);

		void on_pbApriCucina_clicked();
		void on_pbChiudiCucina_clicked();

		void updateScreen();

	private:
		Ui::MainDlg ui;
		QTimer screenUpdate;
};

#endif /* __DLG_MAIN_H__ */
