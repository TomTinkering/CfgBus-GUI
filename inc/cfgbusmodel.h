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
    SizeColumn  = 2,
    RWColumn    = 3,
    TypeColumn  = 4,
    NameColumn  = 5,
    ValueColumn = 6,
    EndColumn   = 7, //should be last
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
            auto topleft  = createIndex(entry,6,nullptr);
            auto btmRight = createIndex(entry,6,nullptr);
            emit dataChanged(topleft,btmRight);
            return true;
        }

        return false;
    }

private:
    bool validIndex(const QModelIndex& index, int role, bool rw) const;

};

#endif // CFGBUSMODEL_H
