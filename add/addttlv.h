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
#ifndef ADDTTLV_H
#define ADDTTLV_H

#include <QWidget>
#include <QGridLayout>
#include <QMap>
#include <QVariant>
#include "toolkit.h"

enum
{
    BIT_HIDE = -2,
    BIT_SHOW = -1,
};

enum
{
    LIST_ADD = 0,
    LIST_MOD = 1,
    REG_ADD  = 2,
    REG_MOD  = 3,
    REG_DROP = 4,
};

namespace Ui {
class addTtlv;
}

class addTtlv : public QWidget
{
    Q_OBJECT

public:
    explicit addTtlv(QObject *obj,int type,QString name,QMap<QString,QVariant> info,int bitInfo,QWidget *parent = nullptr);
    ~addTtlv();

private:
    Ui::addTtlv *ui;
    QString oldName;
    QString oldEventName;
    QGridLayout *listLayout;
    QObject *sourceObj;
    QMap<QString,QVariant> regInfo;
    int bitAddr;
    bool bitEnable[16];
    int addType;
    toolKit toolkit;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void comboBox_type_currentIndexChanged(const QString &arg1);
    void comboBox_subType_currentIndexChanged(const QString &arg1);

signals:
    void addResultSignal(QString,QMap<QString,QVariant>);
    void modResultSignal(QString,QString,QMap<QString,QVariant>);
    void addRegListSignal(QObject *,QString,QMap<QString,QVariant>,int,bool);
    void modRegListSignal(QObject *,QString,QString,QMap<QString,QVariant>);
    void modBitSignal(QObject *,QString,int,int);
};

#endif // ADDTTLV_H
