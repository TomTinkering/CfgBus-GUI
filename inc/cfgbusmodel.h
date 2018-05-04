#ifndef CFGBUSMODEL_H
#define CFGBUSMODEL_H

#include <QAbstractTableModel>
#include "cfgbusentry.h"
#include "cfgbusmaster.h"

namespace Cfg{

enum ModelColumn
{
    IDColumn    = 0,
    AddrColumn  = 1,
    RWColumn    = 2,
    TypeColumn  = 3,
    NameColumn  = 4,
    ValueColumn = 5,
    EndColumn   = 6 //should be last
};

}

class CfgBusModel : public QAbstractTableModel, public CfgBusMaster
{
    Q_OBJECT

public:

    explicit CfgBusModel(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void clearEntries()
    {
        if(getNrEntries()>0)
        {
            int rows = getNrEntries()-1;
            beginRemoveRows(QModelIndex(),0,rows);
            CfgBusMaster::clearEnties();
            m_checked.clear();
            endRemoveRows();

            auto topleft  = createIndex(0,0,nullptr);
            auto btmRight = createIndex(rows,Cfg::EndColumn,nullptr);
            emit dataChanged(topleft,btmRight);
        }
    }

    bool retrieveEntryList()
    {
        clearEntries();

        beginInsertRows(QModelIndex(),0,0);
        bool res = CfgBusMaster::retrieveEntryList();
        endInsertRows();

        if(res && getNrEntries()>0)
        {
            //TODO fix this, wrap master?
            beginInsertRows(QModelIndex(),1,getNrEntries()-1);
            endInsertRows();

            m_checked.clear();
            for(int i=0; i<getNrEntries(); i++)
            {
                m_checked.push_back(Qt::Unchecked);
            }

            auto topleft  = createIndex(0,0,nullptr);
            auto btmRight = createIndex(getNrEntries()-1,Cfg::EndColumn,nullptr);
            emit dataChanged(topleft,btmRight);

            return true;
        }
        else
        {
           beginRemoveRows(QModelIndex(),0,0);
           endRemoveRows();
        }

        return false;
    }

    bool updateEntry(uint32_t entry)
    {
        if(CfgBusMaster::updateEntry(entry))
        {
            auto topleft  = createIndex(entry,Cfg::ValueColumn,nullptr);
            auto btmRight = createIndex(entry,Cfg::ValueColumn,nullptr);
            emit dataChanged(topleft,btmRight);

            emit entryUpdated(this->getEntries()[entry]);
            return true;
        }

        emit packetsUpdated();
        return false;
    }

    const QVector<Qt::CheckState>& getSelected() const
    {
        return m_checked;
    }


private:
    QVector<Qt::CheckState> m_checked = {Qt::Unchecked};
    bool validIndex(const QModelIndex& index, int role, bool rw) const;

signals:
    void entryUpdated(const CfgBusEntry* entry);
    void entryCheckedChanged(const CfgBusEntry* entry, Qt::CheckState);
    void packetsUpdated();

};

#endif // CFGBUSMODEL_H
