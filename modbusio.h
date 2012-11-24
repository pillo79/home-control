#ifndef __MODBUSIO_H__
#define __MODBUSIO_H__

#include <errno.h>
#include <QList>
#include <QString>

class ModbusDevice;

typedef QList<const class ModbusIO *> ModbusIOPtrList;

class ModbusIO {
	public:
		ModbusIO(QString name, ModbusDevice *dev);
		virtual ~ModbusIO();

		virtual int  getValue() { return -ENOTSUP; }
		virtual void setValue(int value) { }
	protected:
		ModbusDevice *mDev;
		QString mName;

		virtual ModbusIOPtrList *objList() { return NULL; }
};

class BitInput : public ModbusIO {
		int mBitAddr;
	public:
		BitInput(QString name, ModbusDevice *dev, int bitAddr);
		virtual ~BitInput() { }

		virtual int getValue();

	public:
		static const ModbusIOPtrList *elements() { return &mElements; }
	protected:
		virtual ModbusIOPtrList *objList() { return &mElements; }
	private:
		static ModbusIOPtrList mElements;
};

class BitOutput : public ModbusIO {
		int mBitAddr;
	public:
		BitOutput(QString name, ModbusDevice *dev, int bitAddr);
		virtual ~BitOutput() { }

		virtual int  getValue();
		virtual void setValue(int value);

	public:
		static const ModbusIOPtrList *elements() { return &mElements; }
	protected:
		virtual ModbusIOPtrList *objList() { return &mElements; }
	private:
		static ModbusIOPtrList mElements;
};

class WordInput : public ModbusIO {
		int mWordAddr;
	public:
		WordInput(QString name, ModbusDevice *dev, int wordAddr);
		virtual ~WordInput() { }

		virtual int getValue();

	public:
		static const ModbusIOPtrList *elements() { return &mElements; }
	protected:
		virtual ModbusIOPtrList *objList() { return &mElements; }
	private:
		static ModbusIOPtrList mElements;
};

class WordOutput : public ModbusIO {
		int mWordAddr;
	public:
		WordOutput(QString name, ModbusDevice *dev, int wordAddr);
		virtual ~WordOutput() { }

		virtual int  getValue();
		virtual void setValue(int value);

	public:
		static const ModbusIOPtrList *elements() { return &mElements; }
	protected:
		virtual ModbusIOPtrList *objList() { return &mElements; }
	private:
		static ModbusIOPtrList mElements;
};

#endif /* __MODBUSIO_H__ */
