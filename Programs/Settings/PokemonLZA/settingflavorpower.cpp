#include "settingflavorpower.h"
#include "Helpers/jsonhelper.h"
#include "Helpers/ocrentrydatabase.h"

namespace Setting::PokemonLZA
{

SettingFlavorPower::SettingFlavorPower(const QString &name) : SettingBase(name)
{
    QVBoxLayout* vBoxLayout = new QVBoxLayout(this);
    vBoxLayout->setContentsMargins(0,0,0,0);

    QHBoxLayout* hBoxLayout = new QHBoxLayout();
    hBoxLayout->setContentsMargins(0,0,0,0);
    vBoxLayout->addLayout(hBoxLayout);

    QFont font = this->font();
    font.setBold(true);

    for (int i = 0; i < 3; i++)
    {
        QVBoxLayout* slotLayout = new QVBoxLayout();
        slotLayout->setContentsMargins(0,0,0,0);
        hBoxLayout->addLayout(slotLayout);

        m_power[i] = new QListWidget();
        m_power[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        m_power[i]->setFixedHeight(100);
        m_power[i]->setSortingEnabled(true);
        m_power[i]->setDragDropMode(QAbstractItemView::DragDrop);
        m_power[i]->setDefaultDropAction(Qt::MoveAction);

        QLabel* label = new QLabel("Slot " + QString::number(i + 1));
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);

        slotLayout->addWidget(label);
        slotLayout->addWidget(m_power[i]);
    }

    QLabel* label = new QLabel("Available Powers");
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);

    m_filter = new QLineEdit();
    m_filter->setValidator(new QRegularExpressionValidator(QRegularExpression("[A-Za-z\-]*")));
    m_filter->setPlaceholderText("Filter");
    connect(m_filter, &QLineEdit::textChanged, this, &SettingFlavorPower::OnFilterChanged);

    m_allPower = new QListWidget();
    m_allPower->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_allPower->setSortingEnabled(true);
    m_allPower->setDragDropMode(QAbstractItemView::DragDrop);
    m_allPower->setDefaultDropAction(Qt::MoveAction);

    vBoxLayout->addWidget(label);
    vBoxLayout->addWidget(m_filter);
    vBoxLayout->addWidget(m_allPower);

    ResetLists();
}

void SettingFlavorPower::Load(QJsonObject &object)
{
    QStringList selectedPowers;

    QVariant value;
    for (int i = 0; i < 3; i++)
    {
        if (JsonHelper::ReadValue(object, m_name + "Slot" + QString::number(i), value))
        {
            QStringList const list = value.toStringList();
            m_power[i]->addItems(list);
            selectedPowers << list;
        }
    }

    // find all selected items from available powers
    QList<QListWidgetItem*> matchedItems;
    for (QString const& power : std::as_const(selectedPowers))
    {
        matchedItems << m_allPower->findItems(power, Qt::MatchExactly);
    }

    // remove selected items from available powers
    for (QListWidgetItem* item : std::as_const(matchedItems))
    {
        delete m_allPower->takeItem(m_allPower->row(item));
    }
}

void SettingFlavorPower::Save(QJsonObject &object) const
{
    QStringList list;
    for (int i = 0; i < 3; i++)
    {
        list.clear();
        for (int j = 0; j < m_power[i]->count(); j++)
        {
            list << m_power[i]->item(j)->text();
        }

        object.insert(m_name + "Slot" + QString::number(i), QJsonArray::fromStringList(list));
    }
}

void SettingFlavorPower::ResetDefault()
{
    ResetLists();
}

QList<QStringList> SettingFlavorPower::GetPowerSlots() const
{
    QList<QStringList> powerSlots(3);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < m_power[i]->count(); j++)
        {
            powerSlots[i] << m_power[i]->item(j)->text();
        }
    }

    return powerSlots;
}

void SettingFlavorPower::OnFilterChanged()
{
    for (int i = 0; i < m_allPower->count(); i++)
    {
        QListWidgetItem* item = m_allPower->item(i);
        item->setHidden(!item->text().contains(m_filter->text()));
    }
}

void SettingFlavorPower::ResetLists()
{
    OCREntryDatabase::EnsureDatabase("PokemonLZA/FlavorPowers");
    OCREntries const& entries = OCREntryDatabase::GetEntries("PokemonLZA/FlavorPowers", LT_English);

    for (int i = 0; i < 3; i++)
    {
        m_power[i]->clear();
    }

    m_allPower->clear();
    m_allPower->addItems(entries.keys());
}

}
