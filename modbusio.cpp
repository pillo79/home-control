#include <modbusio.h>

ModbusIO::ModbusIO(ModbusDevice *dev)
{
	mDev = dev;
}

BitInput::BitInput(int bitAddr, ModbusDevice *dev)
	: ModbusIO(dev)
{
	mBitAddr = bitAddr;
}

int BitInput::getValue()
{
	return mDev->getDigInput(mBitAddr);
}

BitOutput::BitOutput(int bitAddr, ModbusDevice *dev)
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

WordInput::WordInput(int bitAddr, ModbusDevice *dev)
	: ModbusIO(dev)
{
	mWordAddr = bitAddr;
}

int WordInput::getValue()
{
	return mDev->getInputVal(mWordAddr);
}

WordOutput::WordOutput(int bitAddr, ModbusDevice *dev)
	: ModbusIO(dev)
{
	mWordAddr = bitAddr;
}

int WordOutput::getValue()
{
	return mDev->getOutputVal(mWordAddr);
}

void WordOutput::setValue(int value)
{
	mDev->setOutputVal(mWordAddr, !!value);
}

#endif /* __MODBUSIO_H__ */
