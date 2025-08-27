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
#ifndef ADDUART_H
#define ADDUART_H

#include <QWidget>
#include <QMap>
#include <QVariant>

namespace Ui {
class addUart;
}

class addUart : public QWidget
{
    Q_OBJECT

public:
    explicit addUart(QObject *obj,QString name,QMap<QString,QVariant> info,QWidget *parent = nullptr);
    ~addUart();

private:
    Ui::addUart *ui;
    QString oldName;
    QObject *sourceObj;
    QString m_baudrate;

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();
    void slot_baudrateIndexChange(int baudrateIndex);
    void slot_baudrateChange(QString baudrateText);

signals:
    void addResultSignal(QString,QMap<QString,QVariant>);
    void modResultSignal(QString,QString,QMap<QString,QVariant>);
};

#endif // ADDUART_H
