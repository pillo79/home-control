#include <state.h>

#define MAX_PTS 480 // 60*24/3 (1 day in steps of 3 minutes)

State::State()
	: wTemperaturaACS	("TemperaturaACS",	"°C",	"%.1f", MAX_PTS, 0.5)
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
	wApriCucinaPerc = 40;
	wVelFanCoil = 20;
	xModoRiscaldamento = true;
}

State &s() {
	static State instance;
	return instance;
}
