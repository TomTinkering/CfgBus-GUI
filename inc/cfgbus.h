#ifndef CFGBUS_H
#define CFGBUS_H

#include <stdint.h>
#include <string.h>

namespace Cfg{

const uint32_t nrFixedEntries = 5;
const uint32_t listEntrySize = 11;
const uint32_t listAddr = 5000;

enum HeaderRegisters
{
  cfg_magic         = 0,
  cfg_nr_entries    = 1,
  cfg_devname       = 2,
  cfg_err_code      = 3,
  cfg_err_cnt       = 4,
};

enum Error
{
  cfg_ok              = 0,
  cfg_invalid_data    = 1,
  cfg_rx_timeout      = 2,
  cfg_rx_error        = 3,
  cfg_tx_timeout      = 4,
  cfg_tx_error        = 5,
  cfg_invalid_entry   = 6,
  cfg_invalid_cmd     = 7,
  cfg_len_mismatch    = 8,
  cfg_crc_err         = 9,
  cfg_illegal_write   = 10,
  cfg_unknown         = 11 //should be last
};

enum Type
{
  t_u16   = 0,
  t_u32   = 1,
  t_u64   = 2,
  t_i16   = 3,
  t_i32   = 4,
  t_i64   = 5,
  t_flt   = 6,
  t_dbl   = 7,
  t_bool  = 8,
  t_str   = 9,
  t_err   = 10,
  t_unknown = 11//should be last
};


static const std::string ErrorStrings[] =
{
  "0: Ok",
  "1: Invalid Data",
  "2: RX Timeout",
  "3: RX Error",
  "4: TX Timeout",
  "5: TX Error",
  "6: Invalid Entry",
  "7: Invalid CMD",
  "8: Write Len Mismatch",
  "9: CRC Error",
  "10: Illegal Write",
  "11: Unknown"
};


static const std::string TypeStrings[] =
{
  "Uint16",
  "Uint32",
  "Uint64",
  "Int16",
  "Int32",
  "Int64",
  "Float",
  "Double",
  "Bool",
  "String",
  "Error",
  "Unknown"
};


class Utils{

public:
    explicit Utils()
    {}

    static const std::string typeToString(Type t)
    {
        if(t >= t_unknown)
            return "Unknown";

        return TypeStrings[t];
    }

    static const std::string errorToString(Error e)
    {
        if(e >= cfg_unknown)
            return "Unknown";

        return ErrorStrings[e];
    }

};


}

#endif // CFGBUS_H
