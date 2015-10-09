#include <modbusio.h>

#include "modbusdevice.h"

ModbusIOPtrList BitInput::mElements;
ModbusIOPtrList BitOutput::mElements;
ModbusIOPtrList WordInput::mElements;
ModbusIOPtrList WordOutput::mElements;

ModbusIO::ModbusIO(QString name, ModbusDevice *dev)
{
	mDev = dev;
	mName = name;
	if (objList())
		objList()->append(this);
}

ModbusIO::~ModbusIO()
{
	if (objList())
		objList()->removeOne(this);
}

BitInput::BitInput(QString name, ModbusDevice *dev, int bitAddr)
	: ModbusIO(name, dev)
{
	mBitAddr = bitAddr;
}

int BitInput::getValue() const
{
	return mDev->getDigInput(mBitAddr);
}

BitOutput::BitOutput(QString name, ModbusDevice *dev, int bitAddr)
	: ModbusIO(name, dev)
{
	mBitAddr = bitAddr;
}

int BitOutput::getValue() const
{
	return mDev->getDigOutput(mBitAddr);
}

void BitOutput::setValue(int value)
{
	mDev->setDigOutput(mBitAddr, !!value);
}

WordInput::WordInput(QString name, ModbusDevice *dev, int wordAddr)
	: ModbusIO(name, dev)
{
	mWordAddr = wordAddr;
}

int WordInput::getValue() const
{
	return mDev->getInputVal(mWordAddr);
}

WordOutput::WordOutput(QString name, ModbusDevice *dev, int wordAddr)
	: ModbusIO(name, dev)
{
	mWordAddr = wordAddr;
}

int WordOutput::getValue() const
{
	return mDev->getOutputVal(mWordAddr);
}

void WordOutput::setValue(int value)
{
	mDev->setOutputVal(mWordAddr, value);
}
