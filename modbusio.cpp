#include <modbusio.h>

#include "modbusdevice.h"

ModbusIO::ModbusIO(ModbusDevice *dev)
{
	mDev = dev;
}

BitInput::BitInput(ModbusDevice *dev, int bitAddr)
	: ModbusIO(dev)
{
	mBitAddr = bitAddr;
}

int BitInput::getValue()
{
	return mDev->getDigInput(mBitAddr);
}

BitOutput::BitOutput(ModbusDevice *dev, int bitAddr)
	: ModbusIO(dev)
{
	mBitAddr = bitAddr;
}

int BitOutput::getValue()
{
	return mDev->getDigOutput(mBitAddr);
}

void BitOutput::setValue(int value)
{
	mDev->setDigOutput(mBitAddr, !!value);
}

WordInput::WordInput(ModbusDevice *dev, int wordAddr)
	: ModbusIO(dev)
{
	mWordAddr = wordAddr;
}

int WordInput::getValue()
{
	return mDev->getInputVal(mWordAddr);
}

WordOutput::WordOutput(ModbusDevice *dev, int wordAddr)
	: ModbusIO(dev)
{
	mWordAddr = wordAddr;
}

int WordOutput::getValue()
{
	return mDev->getOutputVal(mWordAddr);
}

void WordOutput::setValue(int value)
{
	mDev->setOutputVal(mWordAddr, !!value);
}
