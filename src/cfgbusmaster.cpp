#include "cfgbusmaster.h"
#include "modbus.h"


CfgBusEntry * CfgBusMaster::retrieveListEntry(uint16_t entry)
{
    //read entry data
    uint8_t raw[Cfg::listEntrySize*2]   = {0};

    if(modRead(Cfg::listAddr+(entry*Cfg::listEntrySize),
               Cfg::listEntrySize,
               reinterpret_cast<uint16_t*>(raw)) != Cfg::listEntrySize)
    {
        return nullptr;
    }

    //decode
    uint16_t address = (raw[0] << 8) + raw[1];
    uint8_t size     = raw[2];
    Cfg::Type type   = static_cast<Cfg::Type>(raw[3]);
    bool writeable   = raw[4] != 0;
    std::string name = std::string(reinterpret_cast<char *>(&raw[5]));

    //store
    return new CfgBusEntry(address,size,type,name,writeable);
}


bool CfgBusMaster::retrieveEntryList()
{
    m_gotlist = false;
    m_entries.clear();


    for(uint16_t i=0; i<Cfg::nrFixedEntries; i++)
    {
        auto res = retrieveListEntry(i);
        if(res == nullptr)
            return false;

        m_entries.push_back(res);
    }

    //determine slave endiannes compare to this machine
    m_slaveSameEndian = true; //prevent reordering  of bytes
    if(!updateEntry(Cfg::cfg_magic))
        return false;

    uint16_t magic = m_entries[Cfg::cfg_magic]->getValue<uint16_t>();
    m_slaveSameEndian = (magic == 0xC0DE);

    //determine remaining number of entries
    if(!updateEntry(Cfg::cfg_nr_entries))
        return false;

    uint16_t nrEntries = m_entries[Cfg::cfg_nr_entries]->getValue<uint16_t>();
    if(nrEntries < Cfg::nrFixedEntries)
        return false;

    //retrieve remaining entries
    for(uint16_t i=Cfg::nrFixedEntries; i<nrEntries; i++)
    {
        auto res = retrieveListEntry(i);
        if(res == nullptr)
            return false;

        m_entries.push_back(res);
    }

    m_gotlist     = true;
    m_packets     = 0;
    m_errors      = 0;

    return true;
}


template <typename T>
bool CfgBusMaster::setEntryValue(uint32_t entry, T value)
{
    if(!m_gotlist || entry >= m_entries.size() || !isConnected())
         return false;

    //don't update entry until value is written succesfully
    auto tmpEntry = m_entries[entry];
    if(!tmpEntry->setValue<T>(value))
        return false;

    auto start  = tmpEntry->getAddress();
    int nrRegs = tmpEntry->getSize();

    uint16_t data[nrRegs];
    tmpEntry->getRawValue(data,nrRegs,m_slaveSameEndian);

    if(modWrite(start,nrRegs,data) != nrRegs)
        return false;

    m_entries[entry]->setValue<T>(value);
    return true;
}


bool CfgBusMaster::updateEntry(uint32_t entry)
{
    if(!m_entries.empty() && entry < m_entries.size())
    {
        auto start  = m_entries[entry]->getAddress();
        int nrRegs  = m_entries[entry]->getSize();

        uint16_t data[nrRegs];
        if(modRead(start,nrRegs,data) != nrRegs)
            return false;

        m_entries[entry]->setRawValue(data,nrRegs,m_slaveSameEndian);
        return true;
    }

    return false;
}


template <typename T>
T CfgBusMaster::getEntryValue(uint32_t entry) const
{
    if(m_gotlist && entry < m_entries.size())
        return m_entries[entry]->getValue<T>();

    T k = 0;
    return k;
}


template <>
std::string CfgBusMaster::getEntryValue(uint32_t entry) const
{
    if(entry < m_entries.size())
        return m_entries[entry]->getValue<std::string>();

    return "";
}


bool CfgBusMaster::modbusConnect(const std::string& port,
                                 int baud,
                                 char parity,
                                 int dataBits,
                                 int stopBits,
                                 int RTS,
                                 int timeout)
{
    modbusDisconnect();
    m_timeout = timeout;
    m_modbus = modbus_new_rtu(port.c_str(),baud,parity,dataBits,stopBits,RTS);

    #ifdef LIB_MODBUS_DEBUG_OUTPUT
        modbus_set_debug(static_cast<modbus_t *>(m_modbus), 1);
    #endif

    if(m_modbus == nullptr || modbus_connect(static_cast<modbus_t *>(m_modbus)) == -1)
        return false;

    modbus_set_response_timeout(static_cast<modbus_t *>(m_modbus), timeout, 0);
    m_connected = true;
    return true;
}


void CfgBusMaster::clearEnties()
{
     m_entries.clear();
}


void CfgBusMaster::modbusDisconnect()
{
    if(m_modbus != nullptr) {
        modbus_close(static_cast<modbus_t *>(m_modbus));
        modbus_free(static_cast<modbus_t *>(m_modbus));
        m_modbus = nullptr;
    }

    m_connected = false;
}


bool CfgBusMaster::isConnected() const
{
    return m_connected;
}


int CfgBusMaster::flush()
{
    return modbus_flush(static_cast<modbus_t *>(m_modbus));
}




static void _endianRegCpy(uint16_t* dst, uint16_t* src, uint16_t nrRegs)
{
    const uint16_t _e = 1;
    bool guiBigEndian = ( (*(char*)&_e) == 0 );

    /* to clarify:
     * the slave does not correct for endiannes, but interprets data by the byte order
     * by which it is received, assuming it's own endiannes.
     *
     * libmodbus corrects for endiannes, meaning, if this machine is little endian
     * the bytes of the uint16_t registers presented to/by libmodbus will be flipped.
     * to get the original order of the bytes we have to reverse this flipping.
     *
     * then, if this machine is big-endian, and the slave little endian or visa versa
     * the resulting data must be corrected for endiannes. However since CfgBus supports
     * multi-register values (u32, float, double, str) etc., we need to know size of
     * of the data we are correcting for. Therefor this is not done in this function,
     * but only once the buffer is processed and stored in its proper datatype.
     *
     * this method is only called in the modRead and modWrite function, to correct
     * for libmodnus potential modification of data order
     */

    //data has not been changed by network byte order
    if(guiBigEndian)
    {
        memcpy(dst,src,nrRegs*2);
    }
    //all registers are corrected for network byte order
    else
    {
        for(uint32_t i = 0; i<nrRegs; i++)
            dst[i] = ((src[i] >> 8) & 0x00FF) | ((src[i] << 8) & 0xFF00);
    }
}


//read registers from modbus into regs, and correct for endiannes
int CfgBusMaster::modRead(int start, int nrRegs, uint16_t* regs)
{
    if(m_modbus == nullptr)
        return -1;

    //request data from modbus
    uint16_t rcvBuff[nrRegs] = {0};
    modbus_set_slave(static_cast<modbus_t *>(m_modbus), m_slave);
    int ret = modbus_read_registers(static_cast<modbus_t *>(m_modbus), start, nrRegs, rcvBuff);

    if(ret != nrRegs)
    {
        m_errors++;
        modbus_flush(static_cast<modbus_t *>(m_modbus));
        return ret;
    }

    //correct byte order (as sent by slave)
    _endianRegCpy(regs,rcvBuff,nrRegs);

    m_packets++;
    return ret;
}


int CfgBusMaster::modWrite(int start, int nrRegs, uint16_t* regs)
{
    if(m_modbus == nullptr)
        return -1;

    uint16_t txBuff[nrRegs] = {0};

    //correct for modbus endiannes so that slave receives byte order
    //as presented in regs
    _endianRegCpy(txBuff,regs,nrRegs);

    modbus_set_slave(static_cast<modbus_t *>(m_modbus), m_slave);
    int ret = modbus_write_registers(static_cast<modbus_t *>(m_modbus), start, nrRegs, txBuff);

    if(ret != nrRegs)
    {
        m_errors++;
        modbus_flush(static_cast<modbus_t *>(m_modbus));
    }

    return ret;
}


bool CfgBusMaster::setSlave(int slave)
{
    if(slave >= 246 || slave < 0)
        return false;

    m_slave = slave;
    return true;
}

int CfgBusMaster::getSlave() const
{
    return m_slave;
}


void CfgBusMaster::resetCounters()
{
    m_packets = 0;
    m_errors = 0;
}


int CfgBusMaster::getNrPackets() const
{
    return m_packets;
}


int CfgBusMaster::getNrErrors() const
{
    return m_errors;
}


void CfgBusMaster::setTimeout(int timeout)
{
    if(m_modbus != nullptr && timeout >= 0)
    {
        m_timeout = timeout;
        modbus_set_response_timeout(static_cast<modbus_t *>(m_modbus), timeout, 0);
    }
}

int CfgBusMaster::getTimeout() const
{
    return m_timeout;
}

const std::string& CfgBusMaster::getSlaveName() const
{
    return m_slaveName;
}

int CfgBusMaster::getNrEntries() const
{
    if (m_entries.empty())
        return 0;

    return m_entries.size();
}

const std::vector<CfgBusEntry *>& CfgBusMaster::getEntries() const
{
    return m_entries;
}




//templates
template bool CfgBusMaster::setEntryValue<std::string>(uint32_t entry, std::string value);
template bool CfgBusMaster::setEntryValue<uint16_t>(uint32_t entry, uint16_t value);
template bool CfgBusMaster::setEntryValue<uint32_t>(uint32_t entry, uint32_t value);
template bool CfgBusMaster::setEntryValue<uint64_t>(uint32_t entry, uint64_t value);
template bool CfgBusMaster::setEntryValue<int16_t>(uint32_t entry, int16_t value);
template bool CfgBusMaster::setEntryValue<int32_t>(uint32_t entry, int32_t value);
template bool CfgBusMaster::setEntryValue<int64_t>(uint32_t entry, int64_t value);
template bool CfgBusMaster::setEntryValue<float>(uint32_t entry, float value);
template bool CfgBusMaster::setEntryValue<double>(uint32_t entry, double value);
template bool CfgBusMaster::setEntryValue<bool>(uint32_t entry, bool value);

template uint16_t CfgBusMaster::getEntryValue<uint16_t>(uint32_t entry) const;
template uint32_t CfgBusMaster::getEntryValue<uint32_t>(uint32_t entry) const;
template uint64_t CfgBusMaster::getEntryValue<uint64_t>(uint32_t entry) const;
template int16_t CfgBusMaster::getEntryValue<int16_t>(uint32_t entry) const;
template int32_t CfgBusMaster::getEntryValue<int32_t>(uint32_t entry) const;
template int64_t CfgBusMaster::getEntryValue<int64_t>(uint32_t entry) const;
template float CfgBusMaster::getEntryValue<float>(uint32_t entry) const;
template double CfgBusMaster::getEntryValue<double>(uint32_t entry) const;
template bool CfgBusMaster::getEntryValue<bool>(uint32_t entry) const;
