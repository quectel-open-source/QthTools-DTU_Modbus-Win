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
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>

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
        QString baudRate =  info.find("baudrate").value().toString();
        bool isCustom = false;
        ui->spinBox_port->setValue(name.toInt());
        for (int i = 1; i < ui->comboBox_baudrate->count(); i++)
        {
            if (baudRate == ui->comboBox_baudrate->itemText(i))
            {
                isCustom = true;
            }
        }
        if (isCustom)
        {
            ui->comboBox_baudrate->setCurrentText(baudRate);
        }
        else
        {
            QLineEdit *lineEdit = new QLineEdit(this);
            QRegExpValidator *validator = new QRegExpValidator(QRegExp("^([0-9]{1,8})$"), this);
            lineEdit->setValidator(validator);
            ui->comboBox_baudrate->setLineEdit(lineEdit);
            lineEdit->setText(baudRate);
        }
        ui->comboBox_dataBits->setCurrentText(info.find("dataBits").value().toString());
        ui->comboBox_stopBits->setCurrentText(info.find("stopBits").value().toString());
        ui->comboBox_parity->setCurrentText(info.find("parity").value().toString());
        ui->spinBox_slowPollingInterval->setValue(info.find("slowPollingInterval").value().toInt());
        ui->spinBox_fastPollingInterval->setValue(info.find("fastPollingInterval").value().toInt());
        ui->spinBox_cmdInterval->setValue(info.find("cmdInterval").value().toInt());
        if(info.find("resendCount") != info.end())
        {
            ui->spinBox_resendCount->setValue(info.find("resendCount").value().toInt());
        }
    }
    connect(ui->comboBox_baudrate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &addUart::slot_baudrateIndexChange);
    connect(ui->comboBox_baudrate, &QComboBox::currentTextChanged, this, &addUart::slot_baudrateChange);
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
    if (ui->comboBox_baudrate->currentText().isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("自定义波特率不能为空"));
        msgBox.exec();
        return;
    }
    QMap<QString,QVariant> info;
    info.insert("baudrate",ui->comboBox_baudrate->currentText());
    info.insert("dataBits",ui->comboBox_dataBits->currentText());
    info.insert("stopBits",ui->comboBox_stopBits->currentText());
    info.insert("parity",ui->comboBox_parity->currentText());
    info.insert("slowPollingInterval",ui->spinBox_slowPollingInterval->value());
    info.insert("fastPollingInterval",ui->spinBox_fastPollingInterval->value());
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

void addUart::slot_baudrateIndexChange(int baudrateIndex)
{
    qInfo() << __FUNCTION__;
    if (0 == baudrateIndex)
    {
        QLineEdit *lineEdit = new QLineEdit(this);
        QRegExpValidator *validator = new QRegExpValidator(QRegExp("^([0-9]{1,8})$"), this);
        lineEdit->setValidator(validator);
        ui->comboBox_baudrate->setLineEdit(lineEdit);
        lineEdit->clear();
    }
    else
    {
        ui->comboBox_baudrate->setEditable(false);
    }
}

void addUart::slot_baudrateChange(QString baudrateText)
{
    qInfo() << __FUNCTION__;
    this->m_baudrate = baudrateText;
}
