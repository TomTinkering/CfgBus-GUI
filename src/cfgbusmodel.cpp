#include "cfgbusmodel.h"
#include "cfgbusentry.h"

//with help of http://doc.qt.io/qt-5/modelview.html#2-5-the-minimal-editing-example

CfgBusModel::CfgBusModel(QObject *parent):
    QAbstractTableModel(parent),
    CfgBusMaster()
{
}

bool CfgBusModel::validIndex(const QModelIndex& index, int role, bool writing) const
{
    Q_UNUSED(role)

    if(!index.isValid())
        return false;

    if(index.column() >=Cfg::EndColumn)
        return false;

    if(this->getNrEntries() == 0)
        return false;

    if( !(index.row() >= 0 && index.row() < this->getNrEntries()) )
        return false;

    if(writing && index.column() != Cfg::IDColumn)
    {
        if(!this->getEntries()[index.row()]->isWriteable())
            return false;

        if(index.column() != Cfg::ValueColumn)
            return false;
    }

    return true;
}

QVariant CfgBusModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case Cfg::IDColumn:
            return QString("ID");
        case Cfg::AddrColumn:
            return QString("Adr[n]");
        case Cfg::RWColumn:
            return QString("R/W");
        case Cfg::TypeColumn:
            return QString("Type");
        case Cfg::NameColumn:
            return QString("Name");
        case Cfg::ValueColumn:
            return QString("Value");
        }
    }

    return QVariant();
}


int CfgBusModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return this->getNrEntries();
}

int CfgBusModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return Cfg::EndColumn;
}


QVariant CfgBusModel::data(const QModelIndex &index, int role) const
{
    if(!validIndex(index, role, false))
        return false;

    int row = index.row();
    int col = index.column();

    auto entry = this->getEntries()[row];

    switch(role){
    case Qt::DisplayRole:

        switch(col)
        {
        case Cfg::IDColumn:
            return row;
        case Cfg::AddrColumn:
            if(entry->getAddress() < 10)
                return "0" + QString::number(entry->getAddress()) + " [" + QString::number(entry->getSize()) + "]";
            else
                return QString::number(entry->getAddress()) + " [" + QString::number(entry->getSize()) + "]";
        case Cfg::RWColumn:
            return entry->isWriteable() ? "R/W" : "R";
        case Cfg::TypeColumn:
            return QString::fromStdString(entry->getType().toString());
        case Cfg::NameColumn:
            return QString::fromStdString(entry->getName());
        case Cfg::ValueColumn:
            return QString::fromStdString(entry->getValue<std::string>());
        }

    case Qt::FontRole: //not implemented
        break;

    case Qt::BackgroundRole: //not implemented
        break;

    case Qt::TextAlignmentRole:
        switch(col)
        {
        case Cfg::IDColumn:
            return Qt::AlignCenter;
        case Cfg::RWColumn:
            return Qt::AlignLeft + Qt::AlignVCenter;
        case Cfg::AddrColumn:
            return Qt::AlignLeft + Qt::AlignVCenter;
        case Cfg::TypeColumn:
            return Qt::AlignLeft + Qt::AlignVCenter;
        case Cfg::NameColumn:
            return Qt::AlignLeft + Qt::AlignVCenter;
        case Cfg::ValueColumn:
            return Qt::AlignLeft + Qt::AlignVCenter;
        }
        break;

    case Qt::CheckStateRole:
        switch(col)
        {
            case Cfg::IDColumn:
               if(!m_checked.empty())
                   return m_checked[index.row()];
               else
                   return Qt::Unchecked;
            default:
                    break;
        }
        break;
    }

    return QVariant();
}


bool CfgBusModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(! validIndex(index, role, true))
        return false;

    if(! this->isConnected())
        return false;

    auto col = index.column();
    auto row = index.row();

    if(role == Qt::CheckStateRole && col==Cfg::IDColumn)
    {
        if(!m_checked.empty() && m_checked.count() >= row &&  getNrEntries() >= row)
        {
            m_checked[row] = (m_checked[row] == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
            emit entryCheckedChanged(this->getEntries()[row], m_checked[row]);
            emit dataChanged(index, index, QVector<int>() << role);
            return true;
        }

        return false;
    }

    if (role == Qt::EditRole && data(index, role) != value) {

        if(this->setEntryValue<std::string>(index.row(),std::string(value.toString().toUtf8().constData())))
        {
            emit dataChanged(index, index, QVector<int>() << role);
            return true;
        }
    }

    return false;
}


Qt::ItemFlags CfgBusModel::flags(const QModelIndex &index) const
{
    if(! validIndex(index, Qt::DisplayRole, false))
        return QAbstractTableModel::flags(index);

    if(!this->isConnected())
        return Qt::NoItemFlags;

    if(index.column() == Cfg::IDColumn)
        return Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsSelectable | QAbstractTableModel::flags(index);

    if(index.column() == Cfg::ValueColumn && this->getEntries()[index.row()]->isWriteable())
        return Qt::ItemIsEditable | Qt::ItemIsSelectable | QAbstractTableModel::flags(index);

    if(index.column() == Cfg::ValueColumn && !this->getEntries()[index.row()]->isWriteable())
        return Qt::ItemIsEditable | Qt::ItemIsSelectable;

    return QAbstractTableModel::flags(index);
}
