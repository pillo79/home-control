#ifndef __DLG_CONTROL_H__
#define __DLG_CONTROL_H__

#include <QWidget>
#include <QTimer>

#include "ui_dlg_control.h"

class ControlDlg : public QWidget
{
	Q_OBJECT

	public:
		ControlDlg(QWidget *parent = 0);

	public slots:
		void updateScreen();

	private slots:
		void on_pbNotte_toggled(bool checked);
		void on_pbGiorno_toggled(bool checked);
		void on_pbSoffitta_toggled(bool checked);
		void on_pbFanCoil_toggled(bool checked);

		void on_pbRiscCaldaia_toggled(bool checked);
		void on_pbRiscPompaCalore_toggled(bool checked);
		void on_pbRiscResistenze_toggled(bool checked);
		void on_pbRiscManuale_toggled(bool checked);

		void on_pbVelMinus_clicked();
		void on_pbVelPlus_clicked();

		void on_pbTrasfAccumulo_clicked();

		void on_pbApriCucina_clicked();
		void on_pbChiudiCucina_clicked();

		void on_pbOK_clicked();

	private:
		Ui::ControlDlg ui;
		QTimer m_closeTimer;
		int mLockCount;

		void resetCloseTimer();
		void updateStatoRisc();
		void lockMutex();
		void unlockMutex();
};

#endif /* __DLG_CONTROL_H__ */
