#include "hardware.h"

struct Hardware HW;

static ModbusDevice *Undefined;

static ModbusDevice *Seneca_16DI_8DO_1;
static ModbusDevice *Seneca_10DO_2;
static ModbusDevice *Seneca_3AO_3;
static ModbusDevice *Seneca_4RTD_4, *Seneca_4RTD_5;
static ModbusDevice *Seneca_24DO_6;
static ModbusDevice *Burosoft_Temp_32;

int InitHardware()
{
	Undefined = new ModbusDevice(-1);

	Seneca_16DI_8DO_1 = new Seneca_16DI_8DO(1);
	Seneca_10DO_2 = new Seneca_10DO(2);
	Seneca_3AO_3 = new Seneca_3AO(3);
	Seneca_4RTD_4 = new Seneca_4RTD(4);
	Seneca_4RTD_5 = new Seneca_4RTD(5);
	Seneca_24DO_6 = new Seneca_24DO(6);
	Burosoft_Temp_32 = new Burosoft_Temp(0x20);

#define INIT(var, type, dev, num) HW. var = new type( #var, dev, num)
	// outputs
	INIT(Caldaia.xAlimenta,				BitOutput,	Seneca_16DI_8DO_1, 1);
	INIT(Caldaia.xStartCaldaia,			BitOutput,	Seneca_16DI_8DO_1, 2);
	INIT(Caldaia.xStartPompa,			BitOutput,	Seneca_16DI_8DO_1, 3);
	// inputs
	INIT(Caldaia.xFiammaAccesa,			BitInput,	Seneca_16DI_8DO_1, 1);
	// outputs
	INIT(Riscaldamento.xChiudiValvola,		BitOutput,	Seneca_16DI_8DO_1, 4);
	INIT(Riscaldamento.xStartPompaGiorno,		BitOutput,	Seneca_16DI_8DO_1, 5);
	INIT(Riscaldamento.xStartPompaNotte,		BitOutput,	Seneca_16DI_8DO_1, 6);
	INIT(Riscaldamento.xStartPompaSoffitta,		BitOutput,	Seneca_16DI_8DO_1, 7);
	INIT(Riscaldamento.xStartFanCoilStanzaSoffitta,	BitOutput,	Seneca_16DI_8DO_1, 8);
	INIT(Riscaldamento.xStartFanCoilBagnoSoffitta,	BitOutput,	Seneca_10DO_2, 1);
	// inputs
	INIT(Riscaldamento.xValvolaAperta,		BitInput,	Seneca_16DI_8DO_1, 2);
	// outputs
	INIT(FanCoilCorridoio.xChiudiValvola,		BitOutput,	Seneca_10DO_2, 2);
	INIT(FanCoilCorridoio.xStartVentilatore,	BitOutput,	Seneca_10DO_2, 3);
	INIT(FanCoilCorridoio.xMuoviSerrandaCucina,	BitOutput,	Seneca_10DO_2, 9);
	INIT(FanCoilCorridoio.xApriSerrandaCucina,	BitOutput,	Seneca_10DO_2, 8);
	// word outputs
	INIT(FanCoilCorridoio.wLivelloVentilatore,	WordOutput,	Seneca_3AO_3, 1);
	// inputs
	INIT(FanCoilCorridoio.xValvolaAperta,		BitInput,	Seneca_16DI_8DO_1, 3);
	INIT(FanCoilCorridoio.wTemperaturaUscitaAria,	WordInput,	Seneca_4RTD_4, 1);
	// outputs
	INIT(PompaCalore.xStopPompaCalore,		BitOutput,	Seneca_10DO_2, 10);
	INIT(PompaCalore.xRichiestaCaldo,		BitOutput,	Seneca_10DO_2, 4);
	INIT(PompaCalore.xRichiestaFreddo,		BitOutput,	Seneca_10DO_2, 5);
//	INIT(PompaCalore.xInserResistenze,		BitOutput,	Seneca_24DO_6, 1);	// obsoleto
	INIT(PompaCalore.xConfigResistenze[0],		BitOutput,	Seneca_24DO_6, 9);
	INIT(PompaCalore.xConfigResistenze[1],		BitOutput,	Seneca_24DO_6, 10);
	INIT(PompaCalore.xConfigResistenze[2],		BitOutput,	Seneca_24DO_6, 11);
	INIT(PompaCalore.xConfigResistenze[3],		BitOutput,	Seneca_24DO_6, 12);
	INIT(PompaCalore.xConfigResistenze[4],		BitOutput,	Seneca_24DO_6, 13);

	INIT(PompaCalore.xForzaValvole,			BitOutput,	Seneca_24DO_6, 2);
	INIT(PompaCalore.xForza3VieApri,		BitOutput,	Seneca_24DO_6, 3);
	INIT(PompaCalore.xForza3VieChiudi,		BitOutput,	Seneca_24DO_6, 4);
	INIT(PompaCalore.xForzaRiscApri,		BitOutput,	Seneca_24DO_6, 5);
	INIT(PompaCalore.xForzaRiscFerma,		BitOutput,	Seneca_24DO_6, 6);
	// word inputs
	INIT(PompaCalore.wTemperaturaACS,		WordInput,	Seneca_4RTD_4, 3);
	INIT(PompaCalore.wTemperaturaBoiler,		WordInput,	Seneca_4RTD_4, 4);
	INIT(PompaCalore.wTemperaturaPannelli,		WordInput,	Seneca_4RTD_5, 2);
	// word inputs
	INIT(Pannelli.wPotenzaProdotta,			WordInput,	Seneca_16DI_8DO_1, 5);
	INIT(Pannelli.wPotenzaConsumata,		WordInput,	Seneca_16DI_8DO_1, 4);
	// outputs
	INIT(Accumulo.xStartPompa,			BitOutput,	Seneca_10DO_2, 6);
	INIT(Accumulo.xAcquaDaAccumulo,			BitOutput,	Seneca_10DO_2, 7);
	// word inputs
	INIT(Accumulo.wTemperatura,			WordInput,	Seneca_4RTD_5, 4);
	// word inputs
	INIT(Ambiente.wTemperaturaEsterna,		WordInput,	Seneca_4RTD_5, 1);
	INIT(Ambiente.wUmiditaEsterna,			WordInput,	Undefined, -1);
	INIT(Ambiente.wTemperaturaZonaGiorno,		WordInput,	Undefined, -1);
	INIT(Ambiente.wUmiditaZonaGiorno,		WordInput,	Undefined, -1);
	INIT(Ambiente.wTemperaturaZonaNotte,		WordInput,	Undefined, -1);
	INIT(Ambiente.wUmiditaZonaNotte,		WordInput,	Undefined, -1);
	INIT(Ambiente.wTemperaturaSoffitta,		WordInput,	Burosoft_Temp_32, 1);
	INIT(Ambiente.wUmiditaSoffitta,			WordInput,	Burosoft_Temp_32, 2);
	return 0;
}

// #define DUMP(x) do { printf("%s: ", #x); int ret = x; if (ret < 0) printf("err %i\n", -errno); else printf("OK\n"); } while (0)
// #define DUMP(x) do { int ret = x; if (ret < 0) printf("%s err %i\n", #x, -errno); } while (0)
#define DUMP(x) x

void ReadHardwareInputs()
{
	DUMP(Seneca_16DI_8DO_1->updateInputs());
	DUMP(Seneca_10DO_2->updateInputs());
	DUMP(Seneca_3AO_3->updateInputs());
	DUMP(Seneca_4RTD_4->updateInputs());
	DUMP(Seneca_4RTD_5->updateInputs());
	DUMP(Seneca_24DO_6->updateInputs());
//	DUMP(Burosoft_Temp_32->updateInputs());
}

void WriteHardwareOutputs()
{
	DUMP(Seneca_16DI_8DO_1->updateOutputs());
	DUMP(Seneca_10DO_2->updateOutputs());
	DUMP(Seneca_3AO_3->updateOutputs());
	DUMP(Seneca_4RTD_4->updateOutputs());
	DUMP(Seneca_4RTD_5->updateOutputs());
	DUMP(Seneca_24DO_6->updateOutputs());
//	DUMP(Burosoft_Temp_32->updateOutputs());
}

uint32_t GetCommErrorMask()
{
	int mask = 0;

	if (Seneca_16DI_8DO_1->failing())	mask |= (1 << 1);
	if (Seneca_10DO_2->failing())		mask |= (1 << 2);
	if (Seneca_3AO_3->failing())		mask |= (1 << 3);
	if (Seneca_4RTD_4->failing())		mask |= (1 << 4);
	if (Seneca_4RTD_5->failing())		mask |= (1 << 5);
	if (Seneca_24DO_6->failing())		mask |= (1 << 6);
//	if (Burosoft_Temp_32->failing())	mask |= (1 << 7);

	return mask;
}



