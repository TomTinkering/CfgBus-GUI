#ifndef CFGBUSENTRY_H
#define CFGBUSENTRY_H

#include <string>
#include <vector>
#include "cfgbus.h"


namespace Cfg
{

template <typename T>
static T flipEndian(bool slaveSameEndian, void* val);

}


class CfgType
{
public:
    CfgType(Cfg::Type type):
        mType(type)
    {}

    Cfg::Type raw() const
    {
        return mType;
    }

    std::string toString() const
    {
        return Cfg::Utils::typeToString(mType);
    }

private:
    const Cfg::Type mType;
};


class CfgEntryBase
{
public:
    CfgEntryBase(uint16_t addr, uint32_t size, Cfg::Type type, const std::string& name, bool writeable):
        mAddr(addr),
        mType(new CfgType(type)),
        mName(name),
        mWriteable(writeable),
        mSize(size)
    {
    }

    const CfgType& getType() const
    {
        return *mType;
    }

    uint32_t getSize() const
    {
        return mSize;
    }

    uint32_t getAddress() const
    {
        return mAddr;
    }

    const std::string getName() const
    {
        return mName;
    }

    bool isWriteable() const
    {
        return mWriteable;
    }

private:
    const uint16_t    mAddr = 0;
    const CfgType*    mType = nullptr;
    const std::string mName = "";
    const bool        mWriteable = false;
    uint32_t          mSize = 0;

};


class CfgBusEntry: public CfgEntryBase
{

public:
    CfgBusEntry(uint16_t addr, uint32_t size, Cfg::Type type, const std::string& name, bool writeable):
         CfgEntryBase(addr,size,type,name,writeable)
    { }

    template <typename T>
    T getValue() const;

    template <typename T>
    bool setValue(T value);

    bool getRawValue(uint16_t *value, uint16_t nrRegs, bool slaveSameEndian);
    bool setRawValue(uint16_t *value, uint16_t nrRegs, bool slaveSameEndian);

    const std::string toString() const;

private:

    union Generic
    {
        uint16_t _u16;
        uint32_t _u32;
        uint64_t _u64;
        int16_t  _i16;
        int32_t  _i32;
        int64_t  _i64;
        float    _flt;
        double   _dbl;
        bool     _bool;
    };

    Generic      mValue = {0};
    std::string  mString = "";
};

#endif // CFGBUSENTRY_H
