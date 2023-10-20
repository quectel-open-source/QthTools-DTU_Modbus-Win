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
#ifndef FILEDOWN_H
#define FILEDOWN_H

#include <QWidget>
#include <QCloseEvent>
#include <QTime>
#include <QMutex>
#include "serial.h"

namespace Ui {
class fileDown;
}

class fileDown : public QWidget
{
    Q_OBJECT

public:
    explicit fileDown(QWidget *parent = nullptr);
    ~fileDown();

private:
    Ui::fileDown *ui;
    qSerial *qserial;
    QByteArray fileValue;
    int fileSize;
    int fileOffset;
    QByteArray fileMd5;
    bool downFlag;
    QByteArray serialData;
    QTimer serialTimer;
    QMutex mutex;
    bool fileDownSerialNotifi(int fileIndex,int fileSize,QByteArray md5);
    bool fileDownSerialTransmit(int fileIndex,int offset,QByteArray value);

private slots:
    void portListNoticSlot(QList<QString> portList);
    void dataReadNoticSlot(QByteArray recvData);
    void on_pushButton_uartOpen_clicked();
    void on_pushButton_fileOpen_clicked();
    void on_pushButton_down_clicked();
    void recvTimeout();

protected:
     void closeEvent(QCloseEvent *event);
};

#endif // FILEDOWN_H
