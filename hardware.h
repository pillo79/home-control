#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "modbusdevice.h"
#include "modbusio.h"

struct Hardware {
	BitOutput *xResetPLC;
	struct {
		// outputs
		BitOutput *xAlimenta;
		BitOutput *xStartCaldaia;
		BitOutput *xStartPompa;
		// inputs
		BitInput *xFiammaAccesa;
	} Gas;
	struct {
		// outputs
		BitOutput *xChiudiValvola;
		BitOutput *xStartPompaGiorno;
		BitOutput *xStartPompaNotte;
		BitOutput *xStartPompaSoffitta;
		BitOutput *xStartFanCoilStanzaSoffitta;
		BitOutput *xStartFanCoilBagnoSoffitta;
		BitOutput *xStartPompaCircuito;
		// inputs
		BitInput *xValvolaAperta;
	} Riscaldamento;
	struct {
		// outputs
		BitOutput *xChiudiValvola;
		BitOutput *xStartVentilatore;
		BitOutput *xMuoviSerrandaCucina;
		BitOutput *xApriSerrandaCucina;
		// word outputs
		WordOutput *wLivelloVentilatore;
		// inputs
		BitInput *xValvolaAperta;
		// word inputs
		WordInput *wTemperaturaUscitaAria;
	} FanCoilCorridoio;
	struct {
		// outputs
		BitOutput *xStopPompaCalore;
		BitOutput *xRichiestaCaldo;
		BitOutput *xRichiestaFreddo;
		BitOutput *xConfigResistenze[5];
		BitOutput *xForzaValvole;
		BitOutput *xForza3VieApri;
		BitOutput *xForza3VieChiudi;
		BitOutput *xForzaRiscApri;
		BitOutput *xForzaRiscFerma;
		// inputs
		BitInput *xStatoPompaCalore;
		// word inputs
		WordInput *wTemperaturaACS;
		WordInput *wTemperaturaBoiler;
		WordInput *wTemperaturaPannelli;
	} PompaCalore;
	struct {
		// outputs
		BitOutput *xStartPompa;
		BitOutput *xAcquaDaAccumulo;
		// word inputs
		WordInput *wTemperatura;
	} Accumulo;
	struct {
		FloatInput *wEnergiaProdotta;
		FloatInput *wPotenzaProdotta;
		FloatInput *wEnergiaConsumata;
		FloatInput *wPotenzaConsumata;
		FloatInput *wPotenzaResistenze;
	} Pannelli;
	struct {
		// word inputs
		WordInput *wTemperaturaEsterna;
		WordInput *wUmiditaEsterna;
		WordInput *wTemperaturaZonaGiorno;
		WordInput *wUmiditaZonaGiorno;
		WordInput *wTemperaturaZonaNotte;
		WordInput *wUmiditaZonaNotte;
		WordInput *wTemperaturaSoffitta;
		WordInput *wUmiditaSoffitta;
	} Ambiente;
};

extern struct Hardware HW;

int InitHardware();
void ReadHardwareInputs();
void WriteHardwareOutputs();
uint32_t GetCommErrorMask();

#endif /* __HARDWARE_H__ */
