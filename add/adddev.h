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
#ifndef ADDDEV_H
#define ADDDEV_H

#include <QWidget>
#include <QMenu>
#include <QMap>
#include "toolkit.h"

namespace Ui {
class addDev;
}

class addDev : public QWidget
{
    Q_OBJECT

public:
    explicit addDev(bool isGateway,QStringList uartList,QString devName,QMap<QString,QVariant> info,QWidget *parent = nullptr);
    ~addDev();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void addDevIdSlot(bool flag);
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void slotActionRemove();

private:
    Ui::addDev *ui;
    QString oldName;
    QMenu *table_widget_menu;
    toolKit toolkit;
    bool gateway;
    bool IsContainRepeat(QJsonArray arr);
    bool IsContainRepeat(QString oldName,QString port,int addr);
    bool IsContainRepeat(QString name);
    bool pkIsContainRepeat(QString oldName,QString pk);

signals:
    void addResultSignal(QString,QMap<QString,QVariant>,bool);
    void modResultSignal(QString,QString,QMap<QString,QVariant>);
    void unselect();

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // ADDDEV_H
