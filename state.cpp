#include "state.h"
#include "ctrlobserver.h"
#include "ctrlvalue.h"

#define MAX_PTS 480 // 60*24/3 (1 day in steps of 3 minutes)

State::State()
	: m_settings("/media/mmcblk0p2/settings.ini", QSettings::IniFormat)

	, xModoRiscaldamento	(true)
	, wVelFanCoil		(0, 100, 20)
	, wApriCucinaPerc	(0, 100, 40)

	, wTemperaturaACS	("TemperaturaACS",	"°C",	"%.1f", MAX_PTS, 0.5)
	, wTemperaturaBoiler	("TemperaturaBoiler",	"°C",	"%.1f", MAX_PTS, 0.5)
	, wTemperaturaAccumulo	("TemperaturaAccumulo",	"°C",	"%.1f", MAX_PTS, 0.5)
	, wTemperaturaPannelli	("TemperaturaPannelli",	"°C",	"%.1f", MAX_PTS, 0.5)
	, wTempLegnaH		("TempGiorno",		"°C",	"%.1f", MAX_PTS, 0.5)
	, wTempLegnaL		("TempNotte",		"°C",	"%.1f", MAX_PTS, 0.5)

	, wPotProdotta		("PotProdotta",		"W",	"%.0f", MAX_PTS, 0.0)
	, wPotConsumata		("PotConsumata",	"W",	"%.0f", MAX_PTS, 0.0)
	, wPotResistenze	("PotResistenze",	"W",	"%.0f", MAX_PTS, 0.0)
	, wEnergProdotta	("EnergProdotta",	"kWh",	"%.3f", MAX_PTS, 0.0)
	, wEnergConsumata	("EnergConsumata",	"kWh",	"%.3f", MAX_PTS, 0.0)
	, wEnergPassivo		("EnergPassivo",	"kWh",	"%.3f", MAX_PTS, 0.0)

	, wTempEsterno		("TempEsterno",		"°C",	"%.1f", MAX_PTS, 0.5)
{
	loadSettings();
}

void State::loadSettings()
{
	xModoRiscaldamento(O_CONF)	= m_settings.value("pbModoRisc").toBool();

	xAttivaZonaNotte(O_CONF)	= m_settings.value("pbNotte").toBool();
	xAttivaZonaGiorno(O_CONF)	= m_settings.value("pbGiorno").toBool();
	xAttivaZonaSoffitta(O_CONF)	= m_settings.value("pbSoffitta").toBool();
	xAttivaFanCoil(O_CONF)		= m_settings.value("pbFanCoil").toBool();

	xSetManuale(O_CONF)		= m_settings.value("pbRiscManuale").toBool();
	xUsaGas(O_CONF)			= m_settings.value("pbRiscGas").toBool();
	xUsaPompaCalore(O_CONF)		= m_settings.value("pbRiscPompaCalore").toBool();

	xForzaChiudi(O_CONF)		= m_settings.value("pbForzaChiudi").toBool();

	wVelFanCoil(O_CONF)		= m_settings.value("wVelFanCoil").toInt();
	wApriCucinaPerc(O_CONF)		= m_settings.value("wApriCucinaPerc").toInt();
}

void State::saveSettings()
{
	m_settings.setValue("pbModoRisc",		xModoRiscaldamento.value());

	m_settings.setValue("pbNotte",			xAttivaZonaNotte.value());
	m_settings.setValue("pbGiorno",			xAttivaZonaGiorno.value());
	m_settings.setValue("pbSoffitta",		xAttivaZonaSoffitta.value());
	m_settings.setValue("pbFanCoil",		xAttivaFanCoil.value());

	m_settings.setValue("pbRiscManuale",		xSetManuale.value());
	m_settings.setValue("pbRiscGas",		xUsaGas.value());
	m_settings.setValue("pbRiscPompaCalore",	xUsaPompaCalore.value());

	m_settings.setValue("pbForzaChiudi",		xForzaChiudi.value());

	m_settings.setValue("wVelFanCoil",		wVelFanCoil.value());
	m_settings.setValue("wApriCucinaPerc",		wApriCucinaPerc.value());
}

State &s() {
	static State instance;
	return instance;
}