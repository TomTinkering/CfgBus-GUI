#ifndef CFGBUS_H
#define CFGBUS_H

#include <stdint.h>
#include <stdbool.h>
#include "modbus.h"

//address at which the cfgbus list can be requested
#define CFG_LIST_REG_INDEX  (5000)   //excl 40001 range offset
#define CFG_DEVICE_NAME     "Hover"  //10 chars max
#define CFG_LIST_NAME_CHARS (16)     //max characters in entry name string (excl. terminating \0)

//I am not a big fan of complicated macro structures, but this was the only way I could think
//of that allows an easy definition of entries, that can be easily accessed (cfg.vars._varname),
//and that leaves no room for error by multiple definitions of the same thing (like nr. entries)

//assign initial values in CfgInit()

//USE _STRxx FOR STRING ENTRIES of size xx!!!
//--------------------------------------------------------------------------------------------------
//FORMAT      | entry type | var name       | cfg_type | writeable | name string         |
//--------------------------------------------------------------------------------------------------
#define CFG_ENTRIES(_ENTRY) \
        _ENTRY( magic          , uint16_t   , _U16     , false     , "Magic Value"       ) \
        _ENTRY( nr_entries     , uint16_t   , _U16     , false     , "Nr Entries"        ) \
        _ENTRY( dev_name[12]   , char       , _STR12   , false     , "Device Name"       ) \
        _ENTRY( err_code       , uint16_t   , _U16     , true      , "Error Code"        ) \
        _ENTRY( err_cnt        , uint16_t   , _U16     , true      , "Error Count"       ) \
        _ENTRY( speed_l        , int16_t    , _I16     , true      , "Speed Left (.1%)"  ) \
        _ENTRY( speed_r        , int16_t    , _I16     , true      , "Speed Right (.1%)" ) \
        _ENTRY( pwm_l          , int32_t    , _I32     , false     , "PWM-Left"          ) \
        _ENTRY( pwm_r          , int32_t    , _I32     , false     , "PWM-Right"         ) \
        _ENTRY( pos_l          , uint16_t   , _U16     , false     , "HALLPos-Left"      ) \
        _ENTRY( pos_r          , uint16_t   , _U16     , false     , "HALLPos-Right"     ) \
        _ENTRY( vbat           , float      , _FLT     , false     , "Battery Voltage"   )

//-------------------------------------------------------------------------------------------------------------
// DO NOT EDIT BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING
//-------------------------------------------------------------------------------------------------------------

uint32_t CfgAvailable();
int CfgRead(uint8_t * data, uint32_t len);
int CfgWrite(uint8_t * data, uint32_t len);
void CfgFlushRx();
void CfgFlushTx();
uint32_t CfgTick();
void CfgInit();
void CfgSetError(int error);
void CfgRegRead(uint16_t start, uint16_t nr_regs, uint8_t* data);
mb_exception_t CfgRegWrite(uint16_t start, uint16_t nr_regs, uint8_t* data);
bool CfgValidRange(uint16_t first, uint16_t cnt);

typedef enum
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
} cfg_err_t;

typedef enum
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
} cfg_type_t;


//DONT TOUCH! required for CFGbus operation
#define _STRUCT(_var,_sys,_3,_4,_5) _sys _var;
#define _COUNT(_1,_2,_3,_4,_5) +1
#define CFG_NR_ENTRIES (0+CFG_ENTRIES(_COUNT))


typedef struct
{
  CFG_ENTRIES(_STRUCT)
} cfg_t;

//defined as union so that struct members will be placed in same memory
//as configbus registers, only multiple of 2-byte types allowed to ensure
//allignment with registers
typedef union{
  cfg_t    vars;
  uint16_t regs[(sizeof(cfg_t)+1)/2];
} cfg_mem_t;

extern volatile cfg_mem_t cfg;

#endif //CFGBUS_H

