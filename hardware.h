#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "modbusdevice.h"
#include "modbusio.h"

struct Hardware {
	struct {
		// outputs
		BitOutput *xAlimenta;
		BitOutput *xStartCaldaia;
		BitOutput *xStartPompa;
		// inputs
		BitInput *xFiammaAccesa;
	} Caldaia;
	struct {
		// outputs
		BitOutput *xChiudiValvola;
		BitOutput *xStartPompaGiorno;
		BitOutput *xStartPompaNotte;
		BitOutput *xStartPompaSoffitta;
		BitOutput *xStartFanCoilStanzaSoffitta;
		BitOutput *xStartFanCoilBagnoSoffitta;
		// inputs
		BitInput *xValvolaAperta;
	} Riscaldamento;
	struct {
		// outputs
		BitOutput *xChiudiValvola;
		BitOutput *xAlimenta;
		BitOutput *xStartVentilatore;
		BitOutput *xMuoviSerrandaCucina;
		BitOutput *xChiudiSerrandaCucina;
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
		// word inputs
		WordInput *wTemperaturaACS;
		WordInput *wTemperaturaBoiler;
		WordInput *wTemperaturaPannelli;
	} PompaCalore;
	struct {
		// outputs
		BitOutput *xStartPompa;
		BitOutput *xApriValvola;
		// word inputs
		WordInput *wTemperatura;
	} Accumuli;
	struct {
		// word inputs
		WordInput *wTemperaturaEsterna;
		WordInput *wTemperaturaZonaGiorno;
		WordInput *wTemperaturaSoffitta;
	} Ambiente;
};

extern struct Hardware HW;

int InitHardware();
int DoneHardware();

#endif /* __HARDWARE_H__ */
