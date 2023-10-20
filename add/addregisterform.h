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
#ifndef ADDREGISTERFORM_H
#define ADDREGISTERFORM_H

#include <QWidget>
#include <QMenu>
#include <QMap>
#include <QLabel>
#include <QPushButton>
#include "ToolBox.h"
#include "toolkit.h"

namespace Ui {
class addRegisterForm;
}

class addRegisterForm : public QWidget
{
    Q_OBJECT

public:
    explicit addRegisterForm(QMap<QString,QVariant> info,QWidget *parent = nullptr);
    ~addRegisterForm();
    void removeRegList(QPushButton *button);

private:
    Ui::addRegisterForm *ui;
    ToolBox *listToolBox;
    QMenu *table_widget_menu;
    QPoint actionPos;
    bool initCompleted;
    toolKit toolkit;
    void addRegisterFormForTtlv(QString name,QMap<QString,QVariant> info);

private slots:
    void addTtlvSlot(bool flag);
    void on_pushButton_clicked();
    void addRegResultSlot(QObject *currBox,QString name,QMap<QString,QVariant> info,int bitAddr,bool isSave);
    void tableContextMenuRequested(const QPoint &pos);
    void slotActionRemove();
    void slotActionModify();
    void addRegListResultSlot(QString name,QMap<QString,QVariant> info);
    void modRegListResultSlot(QString oldName,QString newName,QMap<QString,QVariant> info);
    void modBitResultSlot(QObject *obj,QString name,int oldBit,int newBit);

protected:
};

#endif // ADDREGISTERFORM_H
