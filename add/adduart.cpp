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
#include "adduart.h"
#include "ui_adduart.h"
#include <QPushButton>
#include <qDebug>
#include <preqtablewidget.h>
#include <QMessageBox>

addUart::addUart(QObject *obj,QString name,QMap<QString,QVariant> info,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addUart)
{
    ui->setupUi(this);
    sourceObj = obj;
    this->setWindowFlags(this->windowFlags() |Qt::Dialog);
    this->setWindowModality(Qt::ApplicationModal); //阻塞除当前窗体之外的所有的窗体
    /* 禁用 最大化按钮 */
    this->setWindowFlags(this->windowFlags()&~Qt::WindowMaximizeButtonHint);
    /* 回车键确认 */
    ui->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::Key_Return);
    if(info.count())
    {
        oldName = name;
        qDebug()<<"uartInfo"<<info;
        ui->spinBox_port->setValue(name.toInt());
        ui->comboBox_baudrate->setCurrentText(info.find("baudrate").value().toString());
        ui->comboBox_dataBits->setCurrentText(info.find("dataBits").value().toString());
        ui->comboBox_stopBits->setCurrentText(info.find("stopBits").value().toString());
        ui->comboBox_parity->setCurrentText(info.find("parity").value().toString());
        ui->spinBox_pollingInterval->setValue(info.find("pollingInterval").value().toInt());
        ui->spinBox_cmdInterval->setValue(info.find("cmdInterval").value().toInt());
        if(info.find("resendCount") != info.end())
        {
            ui->spinBox_resendCount->setValue(info.find("resendCount").value().toInt());
        }
    }
}

addUart::~addUart()
{
    delete ui;
}

void addUart::on_buttonBox_accepted()
{
    QString newName = QString::number(ui->spinBox_port->value());
    if(newName != oldName)
    {
        PreQTableWidget *table = (PreQTableWidget *)sourceObj;
        for (int row = 0; row < table->rowCount(); row++)
        {
            QLabel *label = (QLabel *)table->cellWidget(row,0);
            if(label->text() == newName)
            {
                QMessageBox::information(0,tr("添加串口失败"),tr("该端口已存在"),tr("确认"));
                return;
            }
        }
    }
    QMap<QString,QVariant> info;
    info.insert("baudrate",ui->comboBox_baudrate->currentText());
    info.insert("dataBits",ui->comboBox_dataBits->currentText());
    info.insert("stopBits",ui->comboBox_stopBits->currentText());
    info.insert("parity",ui->comboBox_parity->currentText());
    info.insert("pollingInterval",ui->spinBox_pollingInterval->value());
    info.insert("cmdInterval",ui->spinBox_cmdInterval->value());
    info.insert("resendCount",ui->spinBox_resendCount->value());
    if(oldName.length())
    {
        emit modResultSignal(oldName,newName,info);
    }
    else
    {
        emit addResultSignal(newName,info);
    }


    this->close();
}

void addUart::on_buttonBox_rejected()
{
    this->close();
}
