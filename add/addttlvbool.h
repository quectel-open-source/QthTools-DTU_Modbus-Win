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
#ifndef ADDTTLVBOOL_H
#define ADDTTLVBOOL_H

#include <QWidget>
#include <QButtonGroup>
#include <QVariant>
#include <QCheckBox>

namespace Ui {
class addTtlvBool;
}

class addTtlvBool : public QWidget
{
    Q_OBJECT

public:
    explicit addTtlvBool(QMap<QString,QVariant> info,int bitAddr,bool bitEnable[],QWidget *parent = nullptr);
    ~addTtlvBool();
    QButtonGroup pButtonGroup;
    QButtonGroup radioGroup;

public slots:
    void showToolTipSlot(void);
    void hideToolTipSlot(void);

private slots:
    void on_checkBox_event_clicked(bool checked);

private:
    Ui::addTtlvBool *ui;
    void setBitEnable(int bit,bool flag);
};

#endif // ADDTTLVBOOL_H
