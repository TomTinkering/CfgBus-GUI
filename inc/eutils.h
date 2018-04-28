#ifndef EUTILS_H
#define EUTILS_H

#include <QString>
#include <QMap>
#include <QTime>
#include <QVariant>
#include "modbus.h"

static const QString ModbusFunctionNames[]={"Unknown [0x00]","Read Coils [0x01]","Read Discrete Inputs [0x02]","Read Holding Registers [0x03]",
                               "Read Input Registers [0x04]","Write Single Coil [0x05]","Write Single Register [0x06]",
                               "Write Multiple Coils [0x0f]","Write Multiple Registers [0x10]"};

static const QString ModbusExceptions[]={   "Unknown [0x00]",
                                            "Illegal Function [0x01]",
                                            "Illagal Data Address [0x02]",
                                            "Illegal Data Value [0x03]",
                                            "Slave Device Failure [0x04]",
                                            "Ackowledge [0x05]",
                                            "Slave Device Busy [0x06]",
                                            "Negative Acknowledge [0x07]",
                                            "Memory Parity Error [0x08]"      };

static const int ModbusFunctionCodes[]={0x1,0x2,0x3,0x4,0x5,0x6,0xf,0x10};
static const QString ModbusModeStamp[]={"[RTU]>","[TCP]>",""};

class EUtils
{
private:
    EUtils();

public:

    static QString ModbusDataTypeName(int fCode)
    {
            switch(fCode)
            {
                    case MODBUS_FC_READ_COILS:
                    case MODBUS_FC_WRITE_SINGLE_COIL:
                    case MODBUS_FC_WRITE_MULTIPLE_COILS:
                            return "Coil (binary)";
                    case MODBUS_FC_READ_DISCRETE_INPUTS:
                            return "Discrete Input (binary)";
                    case MODBUS_FC_READ_HOLDING_REGISTERS:
                    case MODBUS_FC_WRITE_SINGLE_REGISTER:
                    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                            return "Holding Register (16 bit)";
                    case MODBUS_FC_READ_INPUT_REGISTERS:
                            return "Input Register (16 bit)";
                    default:
                            break;
            }
            return "Unknown";
    }

    static bool ModbusIsWriteFunction(int fCode)
    {
            switch(fCode)
            {
                    case MODBUS_FC_READ_COILS:
                    case MODBUS_FC_READ_DISCRETE_INPUTS:
                    case MODBUS_FC_READ_HOLDING_REGISTERS:
                    case MODBUS_FC_READ_INPUT_REGISTERS:
                        return false;

                    case MODBUS_FC_WRITE_SINGLE_COIL:
                    case MODBUS_FC_WRITE_MULTIPLE_COILS:
                    case MODBUS_FC_WRITE_SINGLE_REGISTER:
                    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                         return true;

                    default:
                            break;
            }
            return false;
    }

    static bool ModbusIsWriteCoilsFunction(int fCode)
    {
            switch(fCode)
            {
                    case MODBUS_FC_READ_COILS:
                    case MODBUS_FC_READ_DISCRETE_INPUTS:
                    case MODBUS_FC_READ_HOLDING_REGISTERS:
                    case MODBUS_FC_READ_INPUT_REGISTERS:
                    case MODBUS_FC_WRITE_SINGLE_REGISTER:
                    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                        return false;

                    case MODBUS_FC_WRITE_SINGLE_COIL:
                    case MODBUS_FC_WRITE_MULTIPLE_COILS:
                         return true;

                    default:
                            break;
            }
            return false;
    }

    static bool ModbusIsWriteRegistersFunction(int fCode)
    {
            switch(fCode)
            {
                    case MODBUS_FC_READ_COILS:
                    case MODBUS_FC_READ_DISCRETE_INPUTS:
                    case MODBUS_FC_READ_HOLDING_REGISTERS:
                    case MODBUS_FC_READ_INPUT_REGISTERS:
                    case MODBUS_FC_WRITE_SINGLE_COIL:
                    case MODBUS_FC_WRITE_MULTIPLE_COILS:
                        return false;

                    case MODBUS_FC_WRITE_SINGLE_REGISTER:
                    case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                         return true;

                    default:
                            break;
            }
            return false;
    }

    static QString ModbusFunctionName(int index)
    {
        if(index <= 0x10)
            return ModbusFunctionNames[index];
        else
            return "Unknown [0x" + QString::number(index,16) + "]";
    }

    static int ModbusFunctionCode(int index)
    {
            return ModbusFunctionCodes[index];
    }

    static QString ModbusExceptionName(int index)
    {
        if(index <= 0x08)
            return ModbusExceptions[index];
        else
            return "Unknown [0x" + QString::number(index,16) + "]";
    }

    static QString TxTimeStamp()
    {
        auto time = QTime::currentTime();
        int hh = QVariant(time.toString("0.z")).value<float>()*100;
        return (time.toString("HH:mm:ss") + "." + QString::number(hh) + " Tx : ");
    }

    static QString RxTimeStamp()
    {
        auto time = QTime::currentTime();
        int hh = QVariant(time.toString("0.z")).value<float>()*100;
        return (time.toString("HH:mm:ss") + "." + QString::number(hh) + " Rx : ");
    }

    static QString SysTimeStamp()
    {
        auto time = QTime::currentTime();
        int hh = QVariant(time.toString("0.z")).value<float>()*100;
        return (time.toString("HH:mm:ss") + "." + QString::number(hh) + " Sys : ");
    }

    static QChar parity(QString p)
    {
        //the first char is what we need
        return p.at(0);
    }

    static enum {RTU = 0, TCP = 1, None = 0} ModbusMode;

    static enum {Bin = 2, UInt = 10, SInt = 11, Hex = 16} NumberFormat;

    static enum {ReadCoils = 0x1, ReadDisInputs = 0x2,
                ReadHoldRegs = 0x3, ReadInputRegs = 0x4,
                WriteSingleCoil = 0x5, WriteSingleReg = 0x6,
                WriteMultiCoils = 0xf, WriteMultiRegs = 0x10} FunctionCodes;

    static QString formatValue(int value,int frmt, bool is16Bit);

};

#endif // EUTILS_H
