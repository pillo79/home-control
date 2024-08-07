#ifndef __DLG_CONTROL_H__
#define __DLG_CONTROL_H__

#include <QSettings>
#include <QPushButton>
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
		void on_pbProg_toggled(bool checked);

		void on_pbModoRisc_toggled(bool checked);
		void on_pbRiscGas_toggled(bool checked);
		void on_pbRiscPompaCalore_toggled(bool checked);
		void on_pbRiscResistenze_toggled(bool checked);
		void on_pbRiscManuale_toggled(bool checked);

		void on_pbVelMinus_clicked();
		void on_pbVelPlus_clicked();

		void on_pbApriMinus_clicked();
		void on_pbApriPlus_clicked();
		void on_pbForzaChiudi_toggled(bool checked);

		void on_pbTrasfAccumulo_clicked();

		void on_pbOK_clicked();

	private:
		Ui::ControlDlg ui;
		QSettings mSettings;
		QTimer m_closeTimer;
	//	QPushButton *mProgEn[24];
		int mLockCount;

		QColor mDisabledColor;
		QColor mForcedNormColor, mAutoNormColor;
		QColor mForcedRiscColor, mAutoRiscColor;
		QColor mForcedCondColor, mAutoCondColor;
		QColor mForcedColor, mAutoColor;

		typedef enum { bcNorm, bcRisc, bcCond, bcAuto } ButtonColor;
		void setBtnStatus(QPushButton *pb, bool state, ButtonColor mode = bcAuto, QString forced="", QString automatic="", QString off="", bool disable=false, QString disabled="");
		void setBtnStatus3Way(QPushButton *pb, bool state1, bool state2, ButtonColor mode, QString force1, QString force2, QString auto1, QString auto2, QString off, bool disable=false, QString disabled="");

		void loadSettings();
		void saveSettings();

		void resetCloseTimer();
		void updateBtnStatus();
		void lockMutex();
		void unlockMutex();
};

#endif /* __DLG_CONTROL_H__ */
