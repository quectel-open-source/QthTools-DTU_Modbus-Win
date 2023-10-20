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
#include "filedown.h"
#include "ui_filedown.h"
#include <QDebug>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QCryptographicHash>

fileDown::fileDown(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::fileDown)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->lineEdit->setReadOnly(true);
    ui->textEdit->setReadOnly(true);
    this->setWindowFlags(this->windowFlags() |Qt::Dialog);
    this->setWindowModality(Qt::ApplicationModal); //阻塞除当前窗体之外的所有的窗体
    this->qserial = new qSerial();
    this->qserial->portListPeriodSet(1000);
    connect(this->qserial,SIGNAL(portListNoticSignal(QList<QString>)),this,SLOT(portListNoticSlot(QList<QString>)));
    connect(this->qserial,SIGNAL(dataReadNoticSignal(QByteArray)),this,SLOT(dataReadNoticSlot(QByteArray)));
}

fileDown::~fileDown()
{
    delete ui;
}

void fileDown::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    this->qserial->SerialClose();
}

uint16_t quos_modbus_crc16(QByteArray pData)
{
    uint16_t crc16 = 0xFFFF;
    uint32_t i,j;
    for(i = 0; i < (uint32_t)pData.length(); i++)
    {
        uint8_t tmpData = pData.at(i);
        crc16 = tmpData ^ crc16;
        for(j = 0; j < 8; j++)
        {
            if(crc16 & 0x0001)
            {
                crc16 >>= 1;
                crc16 ^= 0xA001;
            }
            else
            {
                crc16 >>= 1;
            }
        }
    }
    pData.clear();
    pData.squeeze();
    return (uint16_t)((crc16 >> 8) | (crc16 << 8));
}


void fileDown::portListNoticSlot(QList<QString> portList)
{
    qDebug()<<"串口列表发生变化"<<portList;
    QString currentPort = ui->comboBox_port->currentText();
    ui->comboBox_port->clear();
    ui->comboBox_port->addItems(portList);

    if(this->qserial->serialIsOpen() && portList.contains(currentPort) == false)
    {
        this->qserial->SerialClose();
        ui->pushButton_uartOpen->setText(tr("打开串口"));
        ui->comboBox_port->setEnabled(true);
        ui->comboBox_baudrate->setEnabled(true);
        ui->comboBox_dataBits->setEnabled(true);
        ui->comboBox_stopBits->setEnabled(true);
        ui->comboBox_parity->setEnabled(true);
    }
    else
    {
        ui->comboBox_port->setCurrentText(currentPort);
    }

    //调整下拉列表宽度，完整显示
    int max_len=0;
    for(int idx=0;idx<ui->comboBox_port->count();idx++)
    {
        if(max_len<ui->comboBox_port->itemText(idx).toLocal8Bit().length())
        {
            max_len = ui->comboBox_port->itemText(idx).toLocal8Bit().length();
        }
    }
    if(max_len*9*0.75 < ui->comboBox_port->width())
    {
        ui->comboBox_port->view()->setFixedWidth(ui->comboBox_port->width());
    }
    else
    {
        ui->comboBox_port->view()->setFixedWidth(max_len*9*0.75);
    }
}

#define CONFIG_FILE_INDEX 1
#define MODBUS_WRITE_SIZE 240
void fileDown::dataReadNoticSlot(QByteArray recvData)
{
    mutex.lock();
    serialData.append(recvData);
    qDebug()<<"recvBuf"<<serialData.toHex();
    if(serialData[0] != (char)0x01)
    {
        qDebug()<<"从机地址异常";
        serialData.clear();
        mutex.unlock();
        return;
    }
    else if(serialData.length() < 5)
    {
        qDebug()<<"接收不完整";
        mutex.unlock();
        return;
    }
    else if(serialData[1] != (char)0x41 && serialData[1] != (char)0x42 && serialData[1] != (char)0xC1 && serialData[1] != (char)0xC2)
    {
        serialData.clear();
        qDebug()<<"未知的命令";
        mutex.unlock();
        return;
    }
    if(false == downFlag)
    {
        qDebug()<<"当前不处于下载模式";
        serialData.clear();
        mutex.unlock();
        return;
    }
    char cmd = serialData.at(1);
    qDebug()<<"cmd"<<cmd;
    if(cmd == (char)0x41 )
    {
        if(serialData.length() < 6)
        {
            qDebug()<<"接收不完整";
            mutex.unlock();
            return;
        }
        uint16_t crc1 = quos_modbus_crc16(serialData.mid(0,4));
        uint16_t crc2 = serialData.mid(4,2).toHex().toInt(nullptr,16);
        if(crc1 != crc2)
        {
            qDebug()<<"CRC校验和错误";
            ui->textEdit->append(tr("CRC校验和错误"));
            mutex.unlock();
            return;
        }
        qDebug()<<"发起成功ACK";
        serialTimer.stop();
        ui->textEdit->append(tr("文件传输中:")+QString::number(fileOffset)+"/"+QString::number(fileSize));
        fileDownSerialTransmit(CONFIG_FILE_INDEX,fileOffset,fileValue.mid(fileOffset,MODBUS_WRITE_SIZE));
    }
    else if(cmd == (char)0x42)
    {
        if(serialData.length() < 11)
        {
            qDebug()<<"接收不完整";
            mutex.unlock();
            return;
        }
        uint16_t crc1 = quos_modbus_crc16(serialData.mid(0,9));
        uint16_t crc2 = serialData.mid(9,2).toHex().toInt(nullptr,16);
        if(crc1 != crc2)
        {
            qDebug()<<"CRC校验和错误";
            ui->textEdit->append(tr("CRC校验和错误"));
            mutex.unlock();
            return;
        }
        fileOffset += MODBUS_WRITE_SIZE;
        qDebug()<<"传输成功ACK"<<fileOffset<<fileValue.length();
        if(fileOffset >= fileValue.length())
        {
            ui->textEdit->append(tr("文件传输中:")+QString::number(fileValue.length())+"/"+QString::number(fileSize));
            ui->textEdit->append(tr("文件传输完成"));
//            QMessageBox::information(this,"提示","下载完成","ok");
            ui->pushButton_down->setText(tr("开始下载"));
            downFlag = false;
            ui->textEdit->append(tr("停止文件传输"));
            disconnect(&serialTimer, SIGNAL(timeout()), this, SLOT(recvTimeout()));
            serialTimer.stop();
        }
        else if(fileValue.length() - fileOffset < MODBUS_WRITE_SIZE)
        {
            ui->textEdit->append(tr("文件传输中:")+QString::number(fileOffset)+"/"+QString::number(fileSize));
            fileDownSerialTransmit(CONFIG_FILE_INDEX,fileOffset,fileValue.mid(fileOffset,fileValue.length() - fileOffset));
        }
        else
        {
            ui->textEdit->append(tr("文件传输中:")+QString::number(fileOffset)+"/"+QString::number(fileSize));
            fileDownSerialTransmit(CONFIG_FILE_INDEX,fileOffset,fileValue.mid(fileOffset,MODBUS_WRITE_SIZE));
        }
    }
    else
    {
        qDebug()<<"接收错误通知";
        if(serialData.length() < 5)
        {
            qDebug()<<"接收不完整";
            mutex.unlock();
            return;
        }
        uint16_t crc1 = quos_modbus_crc16(serialData.mid(0,3));
        uint16_t crc2 = serialData.mid(3,2).toHex().toInt(nullptr,16);
        if(crc1 != crc2)
        {
            qDebug()<<"CRC校验和错误";
            ui->textEdit->append(tr("CRC校验和错误"));
            mutex.unlock();
            return;
        }
        int code = serialData.at(2);
        qDebug()<<"cmd"<<(int)cmd<<"code"<<code;
        if(cmd == (char)0xc1 || cmd == (char)0xc2)
        {
            switch (code) {
            case 1:
                ui->textEdit->append(tr("设备当前未处于文件传输模式下，请重启设备"));
                break;
            case 2:
                ui->textEdit->append(tr("文件号异常"));
                break;
            case 3:
                ui->textEdit->append(tr("文件大小异常"));
                break;
            case 4:
                ui->textEdit->append(tr("文件偏移量异常"));
                break;
            case 5:
                ui->textEdit->append(tr("文件打开异常"));
                break;
            case 6:
                ui->textEdit->append(tr("文件MD5校验异常"));
                break;
            case 7:
                ui->textEdit->append(tr("文件拷贝异常"));
                break;
            default:
                break;
            }
        }
    }
    serialData.clear();
    mutex.unlock();
    recvData.clear();
    recvData.squeeze();
}

void fileDown::on_pushButton_uartOpen_clicked()
{
    if(ui->pushButton_uartOpen->text() == tr("打开串口"))
    {
        int stopBits;
        int parity;
        switch (ui->comboBox_stopBits->currentIndex())
        {
        case 0:
            stopBits = 1;
            break;
        case 1:
            stopBits = 3;
            break;
        case 2:
            stopBits = 2;
            break;
        default:
            stopBits = -1;
            break;
        }
        switch (ui->comboBox_parity->currentIndex())
        {
        case 0:
            parity = 0;
            break;
        case 1:
            parity = 2;
            break;
        case 2:
            parity = 3;
            break;
        case 3:
            parity = 4;
            break;
        case 4:
            parity = 5;
            break;
        default:
            parity = -1;
            break;
        }
        if( false == this->qserial->serialOpen(ui->comboBox_port->currentText(),ui->comboBox_baudrate->currentText().toInt(),ui->comboBox_dataBits->currentText().toInt(),parity,stopBits,QSerialPort::NoFlowControl))
        {
            QMessageBox::information(this,tr("警告"),tr("串口打开失败，请检查是否被占用"),tr("ok"));
            return;
        }
        ui->pushButton_uartOpen->setText(tr("关闭串口"));
        ui->comboBox_port->setEnabled(false);
        ui->comboBox_baudrate->setEnabled(false);
        ui->comboBox_dataBits->setEnabled(false);
        ui->comboBox_stopBits->setEnabled(false);
        ui->comboBox_parity->setEnabled(false);
        ui->textEdit->append(tr("串口打开成功"));

    }
    else
    {
        this->qserial->SerialClose();
        ui->pushButton_uartOpen->setText(tr("打开串口"));
        ui->comboBox_port->setEnabled(true);
        ui->comboBox_baudrate->setEnabled(true);
        ui->comboBox_dataBits->setEnabled(true);
        ui->comboBox_stopBits->setEnabled(true);
        ui->comboBox_parity->setEnabled(true);
        ui->textEdit->append(tr("串口关闭成功"));
        if(ui->pushButton_down->text() == tr("停止下载"))
        {
            ui->pushButton_down->setText(tr("开始下载"));
            downFlag = false;
            ui->textEdit->append(tr("停止文件传输"));
            disconnect(&serialTimer, SIGNAL(timeout()), this, SLOT(recvTimeout()));
            serialTimer.stop();
        }
    }
}


char intGetChar(uint64_t Number,int position,int NumSize)
{
    uint64_t highByte = ((uint64_t)0xff << (NumSize-position)*8)&Number;
    return (uchar)(highByte>>((NumSize-position)*8));
}

QByteArray IntToByte(uint64_t Number, int NumSize)
{
    QByteArray abyte0;
    for (int i=1;i<=NumSize;i++)
    {
        abyte0.append(intGetChar(Number,i,NumSize));
    }
    return abyte0;
}

bool fileDown::fileDownSerialNotifi(int fileIndex,int fileSize,QByteArray md5)
{
    QByteArray sendBuf;
    sendBuf.append(0x01);
    sendBuf.append(0x41);
    sendBuf.append(0x16);
    sendBuf.append(IntToByte(fileIndex,2));
    sendBuf.append(IntToByte(fileSize,4));
    sendBuf.append(md5);
    sendBuf.append(IntToByte(quos_modbus_crc16(sendBuf),2));
    qDebug()<<"sendBuf"<<sendBuf.toHex();
    this->qserial->SerialSend(sendBuf);
    md5.clear();
    md5.squeeze();
    sendBuf.clear();
    sendBuf.squeeze();
    return true;
}

bool fileDown::fileDownSerialTransmit(int fileIndex,int offset,QByteArray value)
{
    QByteArray sendBuf;
    sendBuf.append(0x01);
    sendBuf.append(0x42);
    sendBuf.append(value.length()+7);
    sendBuf.append(IntToByte(fileIndex,2));
    sendBuf.append(IntToByte(offset,4));
    sendBuf.append(IntToByte(value.length(),1));
    sendBuf.append(value);
    sendBuf.append(IntToByte(quos_modbus_crc16(sendBuf),2));
    qDebug()<<"sendBuf"<<sendBuf.toHex();
    this->qserial->SerialSend(sendBuf);
    value.clear();
    value.squeeze();
    sendBuf.clear();
    sendBuf.squeeze();
    return true;
}

void fileDown::on_pushButton_fileOpen_clicked()
{
    QString file = QFileDialog::getOpenFileName(NULL,tr("选取文件"),".","*.bin");
    if(file.isEmpty())
    {
        QMessageBox::information(0,tr("警告"),tr("请正确选取文件"),tr("确认"));
        return;
    }
    QFile myFile;
    myFile.setFileName(file);
    if(false == myFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(0,tr("警告"),tr("无法打开文件"),tr("确认"));
        return;
    }
    ui->lineEdit->setText(file);
    fileValue = myFile.readAll();
    myFile.close();
    fileSize = fileValue.length();
    QCryptographicHash md(QCryptographicHash::Md5);
    md.addData(fileValue);
    fileMd5 = md.result();
    ui->textEdit->append(tr("导入文件成功"));
}

void fileDown::recvTimeout()
{
    ui->textEdit->append(tr("请求文件传输"));
    fileDownSerialNotifi(CONFIG_FILE_INDEX,fileSize,fileMd5);
}

void fileDown::on_pushButton_down_clicked()
{
    if(ui->pushButton_down->text() == tr("开始下载"))
    {
        if(ui->lineEdit->text().isEmpty())
        {
            QMessageBox::information(0,tr("警告"),tr("请先打开需要传输的文件"),tr("确认"));
            return;
        }
        if(false == qserial->serialIsOpen())
        {
            QMessageBox::information(0,tr("警告"),tr("请先打开串口"),tr("确认"));
            return;
        }
        ui->pushButton_down->setText(tr("停止下载"));
        downFlag = true;
        serialData.clear();
        fileOffset = 0;
        connect(&serialTimer, SIGNAL(timeout()), this, SLOT(recvTimeout()));
        serialTimer.start(1000);
    }
    else
    {
        ui->pushButton_down->setText(tr("开始下载"));
        downFlag = false;
        ui->textEdit->append(tr("停止文件传输"));
        disconnect(&serialTimer, SIGNAL(timeout()), this, SLOT(recvTimeout()));
        serialTimer.stop();
    }
}
