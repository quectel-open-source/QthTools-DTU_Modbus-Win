/*
Copyright (C) 2023  Quectel Wireless Solutions Co.,Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef ADDTTLVENUM_H
#define ADDTTLVENUM_H

#include <QWidget>
#include <QBoxLayout>
#include <QMenu>

namespace Ui {
class addTtlvEnum;
}

class addTtlvEnum : public QWidget
{
    Q_OBJECT

public:
    explicit addTtlvEnum(QMap<QString,QVariant> info,QWidget *parent = nullptr);
    ~addTtlvEnum();

private:
    Ui::addTtlvEnum *ui;
    QHBoxLayout *qhBoxLayout;
    QWidget *newWidget;
    QMenu *table_widget_menu;
    int removeRow;

private slots:
    void addEnumSlot(bool flag);
//    void on_comboBox_numType_currentTextChanged(const QString &arg1);
    void on_checkBox_event_toggled(bool checked);
    void isValueChanged(QString value);
    void slotActionRemove();
    void tableContextMenuRequested(const QPoint &pos);
};

#endif // ADDTTLVENUM_H
