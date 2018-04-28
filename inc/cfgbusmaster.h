#ifndef CFGBUSMASTER_H
#define CFGBUSMASTER_H

#include "cfgbusentry.h"

class CfgBusMaster
{

public:
    explicit CfgBusMaster()
    {
        m_entries.clear();
    }

    bool retrieveEntryList();
    bool updateEntry(uint32_t entry);
    template <typename T>
    T getEntryValue(uint32_t entry) const;

    template <typename T>
    bool setEntryValue(uint32_t entry, T value);

    bool isConnected() const;
    void modbusDisconnect();
    bool modbusConnect(  const std::string& port,
                         int baud,
                         char parity,
                         int dataBits,
                         int stopBits,
                         int RTS,
                         int timeout);

    int flush();
    void resetCounters();
    void clearEnties();

    bool setSlave(int slave);
    void setTimeout(int timeout);

    int getTimeout() const;
    int getSlave() const;
    const std::string& getSlaveName() const;
    int getNrPackets() const;
    int getNrErrors() const;
    int getNrEntries() const;
    const std::vector<CfgBusEntry *>& getEntries() const;

protected:


private:
    int modRead(int start, int nrRegs, uint16_t* regs);
    int modWrite(int start, int nrRegs, uint16_t* regs);
    CfgBusEntry* retrieveListEntry(uint16_t entry);

     std::string m_slaveName = "";
     void *m_modbus          = nullptr;
     bool m_connected        = false;
     bool m_slaveSameEndian  = false;
     bool m_gotlist          = false;
     int  m_packets          = 0;
     int  m_errors           = 0;
     int  m_timeout          = 0;
     bool m_pending          = false;
     int  m_slave            = 1;

     std::vector<CfgBusEntry *> m_entries = {0};

};

#endif // CfgBusMasterr_H




