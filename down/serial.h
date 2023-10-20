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
#ifndef SERIAL_H
#define SERIAL_H
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QList>
#include <QTimer>

typedef struct
{
    uint8_t *buf;
    uint16_t offset;
    uint16_t bufLen;
    uint8_t lastData;
}localRecvCache_t;

class qSerial : public QSerialPort, QSerialPortInfo
{
    Q_OBJECT

public:
    qSerial();
    ~qSerial();
    QSerialPort serial;
    void portListPeriodSet(int32_t period);
    bool serialOpen(QString portName,int baundrate,int dataBits,int parity,int stopBits,int control);
    void SerialClose();
    bool serialIsOpen();
    qint64 SerialSend(QByteArray data);

private:
    QTimer portGetTimer;
    QList<QString> oldPortList;
    bool isFirst;
private slots:
    void portListGetHandle(void);
    void dataReadHandle();

signals:
    void portListNoticSignal(QList<QString>);
    void dataReadNoticSignal(QByteArray);
};
#endif // GIZSERIAL_H
