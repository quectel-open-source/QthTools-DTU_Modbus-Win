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
#ifndef ADDREGISTER_H
#define ADDREGISTER_H

#include <QWidget>
#include <QMap>
#include <QVariant>
#include "toolkit.h"

namespace Ui {
class addRegister;
}

class addRegister : public QWidget
{
    Q_OBJECT

public:
    explicit addRegister(QString name,QMap<QString,QVariant> info,QWidget *parent = nullptr);
    ~addRegister();
    bool IsContainRepeat(int startAddr,int endAddr);

private:
    Ui::addRegister *ui;
    QString oldName;
    toolKit toolkit;

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_checkBox_stateChanged(int arg1);

signals:
    void addResultSignal(QString,QMap<QString,QVariant>);
    void modResultSignal(QString,QString,QMap<QString,QVariant>);
};

#endif // ADDREGISTER_H
