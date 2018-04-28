#include "cfgbusentry.h"
#include <limits>
#include <string.h>

//=======================================================
// HELPER FUNCTIONS
//=======================================================
//template <typename T, typename U>
//static bool ranged_cast(T* dest, U val)
//{
//    //if source is signed, and destination is not
//    if(!std::numeric_limits<T>::is_signed && std::numeric_limits<U>::is_signed && val < 0)
//        return false;

//    if(std::numeric_limits<T>::max() < val)
//        return false;

//    if(std::numeric_limits<T>::lowest() > val)
//         return false;

//    //made it here, perform cast, store and return true
//    *dest = static_cast<T>(val);
//    return true;
//}
using std::numeric_limits;

template <typename T, typename U>
static bool ranged_cast(T* dest, const U value) {

    const intmax_t botT = intmax_t(numeric_limits<T>::min() );
    const intmax_t botU = intmax_t(numeric_limits<U>::min() );
    const uintmax_t topT = uintmax_t(numeric_limits<T>::max() );
    const uintmax_t topU = uintmax_t(numeric_limits<U>::max() );

    if (botT > botU && value < static_cast<U> (botT))
        return false;

    if(topT < topU && value > static_cast<U> (topT))
        return false;

    //made it here, perform cast, store and return true
    *dest = static_cast<T>(value);
    return true;
}

namespace Cfg{

    /*
     * flip bytes in datatype if slave endiannes is not gui machine
     * endiannes
     */
    template <typename T>
    T flipEndian(bool slaveSameEndian, void* val)
    {
        //if slave endiannes differs, invert bytes
        if(!slaveSameEndian)
        {
            T result;
            uint8_t* pOut = reinterpret_cast<uint8_t*>(&result);
            uint8_t* pIn = reinterpret_cast<uint8_t*>(&val);

            for(uint16_t i=0; i<sizeof(T); i++)
            {
                pOut[i] = pIn[sizeof(T)-i-1];
            }

            return result;
        }
        else
        {
            return *reinterpret_cast<T*>(val);
        }
    }

    template uint16_t flipEndian<uint16_t>(bool slaveSameEndian, void* value);
    template uint32_t flipEndian<uint32_t>(bool slaveSameEndian, void* value);
    template uint64_t flipEndian<uint64_t>(bool slaveSameEndian, void* value);
    template int16_t  flipEndian<int16_t> (bool slaveSameEndian, void* value);
    template int32_t  flipEndian<int32_t> (bool slaveSameEndian, void* value);
    template int64_t  flipEndian<int64_t> (bool slaveSameEndian, void* value);
    template float    flipEndian<float>   (bool slaveSameEndian, void* value);
    template double   flipEndian<double>  (bool slaveSameEndian, void* value);
    template bool     flipEndian<bool>    (bool slaveSameEndian, void* value);
}


//=======================================================
// IMPLEMENTATION
//=======================================================
template <typename T>
bool CfgBusEntry::setValue(T value)
{
    switch(this->getType().raw())
    {
        //allow setting of unknown errors (not in Cfg::Error) as they will be displayed as unknown
        case Cfg::t_err:
        case Cfg::t_u16:
            return ranged_cast(&mValue._u16,value);
        case Cfg::t_u32:
            return ranged_cast(&mValue._u32,value);
        case Cfg::t_u64:
            return ranged_cast(&mValue._u64,value);
        case Cfg::t_i16:
            return ranged_cast(&mValue._i16,value);
        case Cfg::t_i32:
            return ranged_cast(&mValue._i32,value);
        case Cfg::t_i64:
            return ranged_cast(&mValue._i64,value);
        case Cfg::t_flt:
            return ranged_cast(&mValue._flt,value);
        case Cfg::t_dbl:
            return ranged_cast(&mValue._dbl,value);
        case Cfg::t_bool:
            mValue._bool = static_cast<bool>(value);
            return true;
        case Cfg::t_str:
            mString = std::to_string(value);
            return true;
        default:
           return false;
    }
}


template <>
bool CfgBusEntry::setValue<std::string>(std::string value)
{
    if(value.length() > 255)
        return false;

    try
    {
        std::size_t idx = 0;

        switch(this->getType().raw())
        {
            //allow setting of unknown errors (not in Cfg::Error) as they will be displayed as unknown
            case Cfg::t_err:
            case Cfg::t_u16:
            case Cfg::t_u32:
            case Cfg::t_u64:
            case Cfg::t_i16:
            case Cfg::t_i32:
            case Cfg::t_i64:
                //also use for unsigned to catch -1 being reported as large numbers.
                //this means range u64 is limited to max(i64)
                return setValue(std::stoll(value,&idx,0));
            case Cfg::t_flt:
            case Cfg::t_dbl:
                return setValue(std::stod(value,&idx));
            case Cfg::t_bool:
                if      (value.compare("False") == 0)  mValue._bool = false;
                else if (value.compare("false") == 0)  mValue._bool = false;
                else if (value.compare("0")     == 0)  mValue._bool = false;
                else if (value.compare("True")  == 0)  mValue._bool = true;
                else if (value.compare("true")  == 0)  mValue._bool = true;
                else if (value.compare("1")     == 0)  mValue._bool = true;
                else
                  return false;

                //if we reach this, conversion was succesfull
                return true;
            case Cfg::t_str:
                mString = value;
                return true;
            default:
               return false;
        }
    } catch (...)
    {
        return false;
    }
}


bool CfgBusEntry::setRawValue(uint16_t *value, uint16_t nrRegs, bool slaveSameEndian)
{
    if(nrRegs != this->getSize())
        return false;

    uint16_t tmp;

    switch(this->getType().raw())
    {
        case Cfg::t_str:
            value[nrRegs-1] = 0; //be sure to terminate string
            mString = std::string(reinterpret_cast<char *>(value));
            break;
        case Cfg::t_u16:
        case Cfg::t_err:
            mValue._u16 = Cfg::flipEndian<uint16_t>(slaveSameEndian,value);
            break;
        case Cfg::t_u32:
            mValue._u32 = Cfg::flipEndian<uint32_t>(slaveSameEndian,value);
            break;
        case Cfg::t_u64:
            mValue._u64 = Cfg::flipEndian<uint64_t>(slaveSameEndian,value);
            break;
        case Cfg::t_i16:
            mValue._i16 = Cfg::flipEndian<int16_t>(slaveSameEndian,value);
            break;
        case Cfg::t_i32:
            mValue._i32 = Cfg::flipEndian<int32_t>(slaveSameEndian,value);
            break;
        case Cfg::t_i64:
            mValue._i64 = Cfg::flipEndian<int64_t>(slaveSameEndian,value);
            break;
        case Cfg::t_flt:
            mValue._flt = Cfg::flipEndian<float>(slaveSameEndian,value);
            break;
        case Cfg::t_dbl:
            mValue._dbl = Cfg::flipEndian<double>(slaveSameEndian,value);
            break;
        case Cfg::t_bool:
            tmp = Cfg::flipEndian<uint16_t>(slaveSameEndian,value);
            mValue._bool = tmp != 0;
            break;

        default:
           return false;
    }

    return true;
}


bool CfgBusEntry::getRawValue(uint16_t *value, uint16_t nrRegs, bool slaveSameEndian)
{
    if(nrRegs != this->getSize())
        return false;

    auto type = this->getType().raw();

    if( type == Cfg::t_u16 || type == Cfg::t_err)
    {
        uint16_t tmp = Cfg::flipEndian<uint16_t>(slaveSameEndian,&mValue._u16);
        memcpy(value,&tmp,sizeof(uint16_t));
    }
    else if(type == Cfg::t_u32)
    {
        uint32_t tmp = Cfg::flipEndian<uint32_t>(slaveSameEndian,&mValue._u32);
        memcpy(value,&tmp,sizeof(uint32_t));
    }
    else if(type == Cfg::t_u64)
    {
        uint64_t tmp = Cfg::flipEndian<uint64_t>(slaveSameEndian,&mValue._u64);
        memcpy(value,&tmp,sizeof(uint64_t));
    }
    else if(type == Cfg::t_i16)
    {
        int16_t tmp = Cfg::flipEndian<int16_t>(slaveSameEndian,&mValue._i16);
        memcpy(value,&tmp,sizeof(int16_t));
    }
    else if(type == Cfg::t_i32)
    {
        int32_t tmp = Cfg::flipEndian<int32_t>(slaveSameEndian,&mValue._i32);
        memcpy(value,&tmp,sizeof(int32_t));
    }
    else if(type == Cfg::t_i64)
    {
        int64_t tmp = Cfg::flipEndian<uint64_t>(slaveSameEndian,&mValue._i64);
        memcpy(value,&tmp,sizeof(int64_t));
    }
    else if(type == Cfg::t_flt)
    {
        float tmp = Cfg::flipEndian<float>(slaveSameEndian,&mValue._flt);
        memcpy(value,&tmp,sizeof(float));
    }
    else if(type == Cfg::t_dbl)
    {
        double tmp = Cfg::flipEndian<double>(slaveSameEndian,&mValue._dbl);
        memcpy(value,&tmp,sizeof(double));
    }
    else if(type == Cfg::t_bool)
    {
        uint16_t tmp = (mValue._bool) ? 0x0101 : 0x0000;
        memcpy(value,&tmp,sizeof(uint16_t));
    }
    else if(type == Cfg::t_str)
    {
        strncpy(reinterpret_cast<char *>(value),mString.c_str(),nrRegs*2);
        value[nrRegs-1] = 0; //be sure to terminate string
    }
    else
    {
        return false;
    }

    return true;
}


template <>
std::string CfgBusEntry::getValue<std::string> () const
{
    switch(this->getType().raw())
    {
        case Cfg::t_u16:
            return std::to_string(mValue._u16);
        case Cfg::t_u32:
            return std::to_string(mValue._u32);
        case Cfg::t_u64:
            return std::to_string(mValue._u64);
        case Cfg::t_i16:
            return std::to_string(mValue._i16);
        case Cfg::t_i32:
            return std::to_string(mValue._i32);
        case Cfg::t_i64:
            return std::to_string(mValue._i64);
        case Cfg::t_flt:
            return std::to_string(mValue._flt);
        case Cfg::t_dbl:
            return std::to_string(mValue._dbl);
        case Cfg::t_bool:
            if(mValue._bool)
                return "True";
            else
                return "False";
        case Cfg::t_str:
            return mString;
        case Cfg::t_err:

#ifdef CFG_STRING_CONSTANTS
            if(mValue._u16 <= static_cast<uint16_t>(Cfg::cfg_unknown))
                return Cfg::ErrorStrings[mValue._u16];
            else
                return std::to_string(mValue._u16) + ": Unknown";
#else
            return std::to_string(mValue._u16);
#endif

    default:
        return "";
    }
}


template <typename T>
T CfgBusEntry::getValue() const
{
    switch(this->getType().raw())
    {
        case Cfg::t_err:
        case Cfg::t_u16:
            return static_cast<T>(mValue._u16);
        case Cfg::t_u32:
            return static_cast<T>(mValue._u32);
        case Cfg::t_u64:
            return static_cast<T>(mValue._u64);
        case Cfg::t_i16:
            return static_cast<T>(mValue._i16);
        case Cfg::t_i32:
            return static_cast<T>(mValue._i32);
        case Cfg::t_i64:
            return static_cast<T>(mValue._i64);
        case Cfg::t_flt:
            return static_cast<T>(mValue._flt);
        case Cfg::t_dbl:
            return static_cast<T>(mValue._dbl);
        case Cfg::t_bool:
            return static_cast<T>(mValue._bool);
    default:
        return static_cast<T>(0);
    }
}


const std::string CfgBusEntry::toString() const
{
    return getValue<std::string>();
}


//instantiate valid templates
template uint16_t CfgBusEntry::getValue<uint16_t>() const;
template uint32_t CfgBusEntry::getValue<uint32_t>() const;
template uint64_t CfgBusEntry::getValue<uint64_t>() const;
template int16_t  CfgBusEntry::getValue<int16_t>() const;
template int32_t  CfgBusEntry::getValue<int32_t>() const;
template int64_t  CfgBusEntry::getValue<int64_t>() const;
template float    CfgBusEntry::getValue<float>() const;
template double   CfgBusEntry::getValue<double>() const;
template bool     CfgBusEntry::getValue<bool>() const;

template bool CfgBusEntry::setValue(uint16_t value);
template bool CfgBusEntry::setValue(uint32_t value);
template bool CfgBusEntry::setValue(uint64_t value);
template bool CfgBusEntry::setValue(int16_t value);
template bool CfgBusEntry::setValue(int32_t value);
template bool CfgBusEntry::setValue(int64_t value);
template bool CfgBusEntry::setValue(float value);
template bool CfgBusEntry::setValue(double value);
template bool CfgBusEntry::setValue(bool value);
