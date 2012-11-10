#include "hardware.h"

struct Hardware HW;

static ModbusDevice *Seneca_16DI_8DO_1;
static ModbusDevice *Seneca_10DO_2;
static ModbusDevice *Seneca_3AO_3;
static ModbusDevice *Seneca_4RTD_4, *Seneca_4RTD_5;

int InitHardware()
{
	Seneca_16DI_8DO_1 = new Seneca_16DI_8DO(1);
	Seneca_10DO_2 = new Seneca_10DO(2);
	Seneca_3AO_3 = new Seneca_3AO(3);
	Seneca_4RTD_4 = new Seneca_4RTD(4);
	Seneca_4RTD_5 = new Seneca_4RTD(5);

	// outputs
	HW.Caldaia.xAlimenta = new BitOutput(Seneca_16DI_8DO_1, 1);
	HW.Caldaia.xStartCaldaia = new BitOutput(Seneca_16DI_8DO_1, 2);
	HW.Caldaia.xStartPompa = new BitOutput(Seneca_16DI_8DO_1, 3);
	// inputs
	HW.Caldaia.xFiammaAccesa = new BitInput(Seneca_16DI_8DO_1, 1);
	// outputs
	HW.Riscaldamento.xChiudiValvola = new BitOutput(Seneca_16DI_8DO_1, 4);
	HW.Riscaldamento.xStartPompaGiorno = new BitOutput(Seneca_16DI_8DO_1, 5);
	HW.Riscaldamento.xStartPompaNotte = new BitOutput(Seneca_16DI_8DO_1, 6);
	HW.Riscaldamento.xStartPompaSoffitta = new BitOutput(Seneca_16DI_8DO_1, 7);
	HW.Riscaldamento.xStartFanCoilStanzaSoffitta = new BitOutput(Seneca_16DI_8DO_1, 8);
	HW.Riscaldamento.xStartFanCoilBagnoSoffitta = new BitOutput(Seneca_10DO_2, 1);
	// inputs
	HW.Riscaldamento.xValvolaAperta = new BitInput(Seneca_16DI_8DO_1, 2);
	// outputs
	HW.FanCoilCorridoio.xChiudiValvola = new BitOutput(Seneca_10DO_2, 2);
	HW.FanCoilCorridoio.xStartVentilatore = new BitOutput(Seneca_10DO_2, 3);
	HW.FanCoilCorridoio.xMuoviSerrandaCucina = new BitOutput(Seneca_10DO_2, 9);
	HW.FanCoilCorridoio.xChiudiSerrandaCucina = new BitOutput(Seneca_10DO_2, 8);
	HW.FanCoilCorridoio.wLivelloVentilatore = new WordOutput(Seneca_3AO_3, 1);
	// inputs
	HW.FanCoilCorridoio.xValvolaAperta = new BitInput(Seneca_16DI_8DO_1, 3);
	HW.FanCoilCorridoio.wTemperaturaUscitaAria = new WordInput(Seneca_4RTD_4, 1);
	// outputs
	HW.PompaCalore.xStopPompaCalore = new BitOutput(Seneca_10DO_2, 10);
	HW.PompaCalore.xRichiestaCaldo = new BitOutput(Seneca_10DO_2, 4);
	HW.PompaCalore.xRichiestaFreddo = new BitOutput(Seneca_10DO_2, 5);
	HW.PompaCalore.wTemperaturaACS = new WordInput(Seneca_4RTD_4, 3);
	HW.PompaCalore.wTemperaturaBoiler = new WordInput(Seneca_4RTD_4, 4);
	HW.PompaCalore.wTemperaturaPannelli = new WordInput(Seneca_4RTD_5, 2);
	// outputs
	HW.Accumuli.xStartPompa = new BitOutput(Seneca_10DO_2, 6);
	HW.Accumuli.xApriValvola = new BitOutput(Seneca_10DO_2, 7);
	HW.Accumuli.wTemperatura = new WordInput(Seneca_4RTD_5, 4);
	// word inputs
	HW.Ambiente.wTemperaturaEsterna = new WordInput(Seneca_4RTD_5, 1);
	HW.Ambiente.wTemperaturaZonaGiorno = new WordInput(Seneca_4RTD_5, 3);
	HW.Ambiente.wTemperaturaSoffitta = new WordInput(Seneca_4RTD_4, 2);

	return 0;
}

void ReadHardwareInputs()
{
	Seneca_16DI_8DO_1->updateInputs();
	Seneca_10DO_2->updateInputs();
	Seneca_3AO_3->updateInputs();
	Seneca_4RTD_4->updateInputs();
	Seneca_4RTD_5->updateInputs();
}

void WriteHardwareOutputs()
{
	Seneca_16DI_8DO_1->updateOutputs();
	Seneca_10DO_2->updateOutputs();
	Seneca_3AO_3->updateOutputs();
	Seneca_4RTD_4->updateOutputs();
	Seneca_4RTD_5->updateOutputs();
}
