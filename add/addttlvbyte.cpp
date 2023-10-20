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
#include "addttlvbyte.h"
#include "ui_addttlvbyte.h"
#include <QDebug>

addTtlvByte::addTtlvByte(QMap<QString,QVariant> info,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addTtlvByte)
{
    ui->setupUi(this);
    QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5][a-zA-Z0-9_\/\.\u4e00-\u9fa5]+");
    ui->event_name->setValidator(new QRegExpValidator(rx,this));
    ui->event_name->setToolTip(tr("支持中文、大小写字母、\n数字、短划线、下划线、\n斜杠和小数点，必须以中文、\n英文或数字开头，不超过64个字符"));

    ui->comboBox->setCurrentText(QObject::tr("字符串"));
    QRegExp rx1("^[ A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/]+$");
    ui->register_value->setValidator(new QRegExpValidator(rx1,this));
    ui->register_value->setToolTip(tr("支持大小写字母、\n数字、英文字符"));
    ui->event_type->setCurrentText(QObject::tr("告警"));

    if(info.count())
    {
        ui->spinBox_len->setValue(info.value("len").toInt());
        ui->spinBox_len->setEnabled(false);

        QMap<QString,QVariant> eventInfo = info.value("event").toMap();
        if (eventInfo.value("addEvent").toBool() == true)
        {
            ui->checkBox_event->setChecked(true);
            on_checkBox_event_clicked(true);
            ui->event_name->setText(eventInfo.value("eventName").toString());
            if (eventInfo.value("eventType").toString() == "故障")
            {
                ui->event_type->setCurrentText(QObject::tr("故障"));
            }
            else if (eventInfo.value("eventType").toString() == "告警")
            {
                ui->event_type->setCurrentText(QObject::tr("告警"));
            }
            else if (eventInfo.value("eventType").toString() == "信息")
            {
                ui->event_type->setCurrentText(QObject::tr("信息"));
            }

            if (eventInfo.value("eventConditions").toMap().value("byteType").toString() == "字节流")
            {
                ui->comboBox->setCurrentText(QObject::tr("字节流"));
                ui->register_value->clear();
                QRegExp rx1("^[A-Fa-f0-9]+$");
                ui->register_value->setValidator(new QRegExpValidator(rx1,this));
                ui->register_value->setToolTip(tr("支持A-F，a-f，以及数字"));
                ui->register_value->setMaxLength(info.value("len").toInt() * 2);
                ui->register_value->setText(eventInfo.value("eventConditions").toMap().value("byteStream").toString());
            }
            else if (eventInfo.value("eventConditions").toMap().value("byteType").toString() == "字符串")
            {
                ui->comboBox->setCurrentText(QObject::tr("字符串"));
                QString text = eventInfo.value("eventConditions").toMap().value("byteStream").toString();
                ui->register_value->clear();
                QRegExp rx1("^[ A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/]+$");
                ui->register_value->setValidator(new QRegExpValidator(rx1,this));
                ui->register_value->setToolTip(tr("支持大小写字母、\n数字、英文字符"));
                if (text.length() % 2)
                {
                    text = text + "0";
                }
                int str = 0;
                QString byteHexToString;
                QString hex;
                for (int i = 0; i < text.length(); i ++)
                {
                    str ++;
                    hex = hex + text.at(i);
                    if (str == 2)
                    {
                        str = 0;
                        bool ok;
                        byteHexToString = byteHexToString + QChar(hex.toInt(&ok, 16));
                        hex = "";
                    }
                }
                ui->register_value->setMaxLength(info.value("len").toInt());
                ui->register_value->setText(byteHexToString);
            }
        }
        else
        {
            ui->checkBox_event->setChecked(false);
            on_checkBox_event_clicked(false);
        }
    }
    else
    {
        ui->checkBox_event->setChecked(false);
        on_checkBox_event_clicked(false);
    }
}

addTtlvByte::~addTtlvByte()
{
    delete ui;
}

void addTtlvByte::on_checkBox_event_clicked(bool checked)
{
    if (checked == true)
    {
        ui->label_condition->show();
        ui->label_name->show();
        ui->label_type->show();
        ui->event_name->show();
        ui->event_type->show();
        ui->comboBox->show();
        ui->register_value->show();
    }
    else if (checked == false)
    {
        ui->label_condition->hide();
        ui->label_name->hide();
        ui->label_type->hide();
        ui->event_name->hide();
        ui->event_type->hide();
        ui->comboBox->hide();
        ui->register_value->hide();
    }
}

void addTtlvByte::on_comboBox_currentIndexChanged(const QString &arg1)
{
    if (arg1 == tr("字符串"))
    {
        QString text = ui->register_value->text();
        ui->register_value->clear();
        QRegExp rx1("^[ A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/]+$");
        ui->register_value->setValidator(new QRegExpValidator(rx1,this));
        ui->register_value->setToolTip(tr("支持大小写字母、\n数字、英文字符"));
        if (text.length() % 2)
        {
            text = text + "0";
        }
        int str = 0;
        QString byteHexToString;
        QString hex;
        for (int i = 0; i < text.length(); i ++)
        {
            str ++;
            hex = hex + text.at(i);
            if (str == 2)
            {
                str = 0;
                bool ok;
                byteHexToString = byteHexToString + QChar(hex.toInt(&ok, 16));
                hex = "";
            }
        }
        ui->register_value->setMaxLength(ui->spinBox_len->value());
        ui->register_value->setText(byteHexToString);
//        qDebug()<<"字符串"<<text<<"->"<<byteHexToString;
    }
    else
    {
        QString text = ui->register_value->text();
        ui->register_value->clear();
        QRegExp rx1("^[A-Fa-f0-9]+$");
        ui->register_value->setValidator(new QRegExpValidator(rx1,this));
        ui->register_value->setToolTip(tr("支持A-F，a-f，以及数字"));
        QByteArray byte = text.toUtf8();
        QString stringToByteHex;
        for(int i = 0; i < text.size(); i++)
        {
            stringToByteHex = stringToByteHex + QString("%1").arg(int(byte.at(i)),0,16);
        }
        ui->register_value->setMaxLength(ui->spinBox_len->value() * 2);
        ui->register_value->setText(stringToByteHex);
//        qDebug()<<"字节流"<<text<<"->"<<stringToByteHex;
        byte.clear();
        byte.squeeze();
    }
}

void addTtlvByte::on_spinBox_len_valueChanged(int arg1)
{
    if (ui->comboBox->currentText() == tr("字节流"))
    {
        ui->register_value->setMaxLength(arg1 * 2);
    }
    else if (ui->comboBox->currentText() == tr("字符串"))
    {
        ui->register_value->setMaxLength(arg1);
    }
}
