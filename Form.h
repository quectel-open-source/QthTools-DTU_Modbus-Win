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
#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QMenu>
#include <QMap>
#include <QLabel>
#include "toolkit.h"

namespace Ui {
class Form;
} 

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QString title,QWidget *parent = nullptr);
    ~Form();

private:
    Ui::Form *ui;
    toolKit toolkit;

private slots:
    void addProductSlot(bool flag);
    void addUartSlot(bool flag);
    void addTtlvSlot(bool flag);
    void addProductResultSlot(QString name,QMap<QString,QVariant> info);
    void recoverProductResultSlot(QString name,QMap<QString,QVariant> info);
    void modProductResultSlot(QString oldName,QString newName,QMap<QString,QVariant> info);
    void addUartResultSlot(QString name,QMap<QString,QVariant> info);
    void recoverUartResultSlot(QString name,QMap<QString,QVariant> info);
    void modUartResultSlot(QString oldName,QString newName,QMap<QString,QVariant> info);
    void addTtlvResultSlot(QString name,QMap<QString,QVariant> info);
    void recoverTtlvResultSlot(QString name,QMap<QString,QVariant> info);
    void modTtlvResultSlot(QString oldName,QString newName,QMap<QString,QVariant> info);

signals:
    void addRegisterTtlv(QLabel *);
    void modifySignal();

};

#endif // FORM_H
