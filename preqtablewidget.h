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
#ifndef PREQTABLEWIDGET_H
#define PREQTABLEWIDGET_H

#include <QTableWidget>
#include <QMenu>
#include "toolkit.h"

class PreQTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit PreQTableWidget(QWidget *parent = 0);
    QMap<QString,QVariant> ttlvList;
    QMap<QString,QVariant> uartList;
    QMap<QString,QVariant> productList;
    void modRegisterTtlv(QString oldName,QString newName,QMap<QString,QVariant> info);
    void modRegisterUart(QString oldName,QString newName);
    
private:
    QMenu *table_widget_menu;
    QMenu *table_row_menu;
    QPoint actionPos;
    QObject *currentLabel;
    toolKit toolkit;
    void removeRegisterTtlv(QString name);
    bool findDevForUart(QString name);
    void delRegisterUart(QString oldName);

protected:
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void tableContextMenuRequested(const QPoint &pos);
    void slotActionRemove();
    void slotActionModify();
    void slotActionRemoveFront();
    void slotActionRemoveBack();

signals:

    
};

#endif // PREQTABLEWIDGET_H
