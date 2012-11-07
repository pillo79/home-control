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

void BitOutput::setValue(int value)
{
	mDev->setDigOutput(mBitAddr, !!value);
}

#endif /* __MODBUSIO_H__ */
