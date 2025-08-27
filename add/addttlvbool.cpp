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
#include "addttlvbool.h"
#include "ui_addttlvbool.h"
#include <QDebug>
#include <QMessageBox>

addTtlvBool::addTtlvBool(QMap<QString,QVariant> info,int bitAddr,bool bitEnable[],QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addTtlvBool)
{
    ui->setupUi(this);
    ui->label_writeToolTip->hide();
    pButtonGroup.addButton(ui->checkBox_0,0);
    pButtonGroup.addButton(ui->checkBox_1,1);
    pButtonGroup.addButton(ui->checkBox_2,2);
    pButtonGroup.addButton(ui->checkBox_3,3);
    pButtonGroup.addButton(ui->checkBox_4,4);
    pButtonGroup.addButton(ui->checkBox_5,5);
    pButtonGroup.addButton(ui->checkBox_6,6);
    pButtonGroup.addButton(ui->checkBox_7,7);
    pButtonGroup.addButton(ui->checkBox_8,8);
    pButtonGroup.addButton(ui->checkBox_9,9);
    pButtonGroup.addButton(ui->checkBox_10,10);
    pButtonGroup.addButton(ui->checkBox_11,11);
    pButtonGroup.addButton(ui->checkBox_12,12);
    pButtonGroup.addButton(ui->checkBox_13,13);
    pButtonGroup.addButton(ui->checkBox_14,14);
    pButtonGroup.addButton(ui->checkBox_15,15);
    pButtonGroup.setExclusive(true);

    radioGroup.addButton(ui->radioButton_true,1);
    radioGroup.addButton(ui->radioButton_false,0);
    QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5][a-zA-Z0-9_\/\.\u4e00-\u9fa5]+");
    ui->event_name->setValidator(new QRegExpValidator(rx,this));
    ui->event_name->setToolTip(tr("支持中文、大小写字母、\n数字、短划线、下划线、\n斜杠和小数点，必须以中文、\n英文或数字开头，不超过64个字符"));
    ui->event_type->setCurrentText(QObject::tr("告警"));

    if(info.count())
    {
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

            if (eventInfo.value("eventConditions").toMap().value("bool").toBool() == false)
            {
                ui->radioButton_false->setChecked(true);
            }
            else if (eventInfo.value("eventConditions").toMap().value("bool").toBool() == true)
            {
                ui->radioButton_true->setChecked(true);
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

    if(bitAddr >= -1)
    {
        ui->label->show();
        ui->checkBox_0->show();
        ui->checkBox_1->show();
        ui->checkBox_2->show();
        ui->checkBox_3->show();
        ui->checkBox_4->show();
        ui->checkBox_5->show();
        ui->checkBox_6->show();
        ui->checkBox_7->show();
        ui->checkBox_8->show();
        ui->checkBox_9->show();
        ui->checkBox_10->show();
        ui->checkBox_11->show();
        ui->checkBox_12->show();
        ui->checkBox_13->show();
        ui->checkBox_14->show();
        ui->checkBox_15->show();
        for(int i=15;i>=0;i--)
        {
            setBitEnable(i,bitEnable[i]);
            if(bitEnable[i])
            {
                pButtonGroup.button(i)->setChecked(true);
            }
        }
        qDebug()<<"checkedId"<<pButtonGroup.checkedId()<<bitAddr;
        if(bitAddr >= 0)
        {
            pButtonGroup.button(bitAddr)->setChecked(true);
        }
    }
    else
    {
        ui->label->hide();
        ui->checkBox_0->hide();
        ui->checkBox_1->hide();
        ui->checkBox_2->hide();
        ui->checkBox_3->hide();
        ui->checkBox_4->hide();
        ui->checkBox_5->hide();
        ui->checkBox_6->hide();
        ui->checkBox_7->hide();
        ui->checkBox_8->hide();
        ui->checkBox_9->hide();
        ui->checkBox_10->hide();
        ui->checkBox_11->hide();
        ui->checkBox_12->hide();
        ui->checkBox_13->hide();
        ui->checkBox_14->hide();
        ui->checkBox_15->hide();
    }
}

addTtlvBool::~addTtlvBool()
{
    delete ui;
}

void addTtlvBool::showToolTipSlot(void)
{
    qInfo() << __FUNCTION__;
    ui->label_writeToolTip->show();
}

void addTtlvBool::hideToolTipSlot(void)
{
    qInfo() << __FUNCTION__;
    ui->label_writeToolTip->hide();
}

void addTtlvBool::setBitEnable(int bit,bool flag)
{
    switch (bit)
    {
    case 0:
        ui->checkBox_0->setEnabled(flag);
        break;
    case 1:
        ui->checkBox_1->setEnabled(flag);
        break;
    case 2:
        ui->checkBox_2->setEnabled(flag);
        break;
    case 3:
        ui->checkBox_3->setEnabled(flag);
        break;
    case 4:
        ui->checkBox_4->setEnabled(flag);
        break;
    case 5:
        ui->checkBox_5->setEnabled(flag);
        break;
    case 6:
        ui->checkBox_6->setEnabled(flag);
        break;
    case 7:
        ui->checkBox_7->setEnabled(flag);
        break;
    case 8:
        ui->checkBox_8->setEnabled(flag);
        break;
    case 9:
        ui->checkBox_9->setEnabled(flag);
        break;
    case 10:
        ui->checkBox_10->setEnabled(flag);
        break;
    case 11:
        ui->checkBox_11->setEnabled(flag);
        break;
    case 12:
        ui->checkBox_12->setEnabled(flag);
        break;
    case 13:
        ui->checkBox_13->setEnabled(flag);
        break;
    case 14:
        ui->checkBox_14->setEnabled(flag);
        break;
    case 15:
        ui->checkBox_15->setEnabled(flag);
        break;
    default:
        break;
    }
}

void addTtlvBool::on_checkBox_event_clicked(bool checked)
{
    if (checked == true)
    {
        ui->label_condition->show();
        ui->label_name->show();
        ui->label_type->show();
        ui->event_name->show();
        ui->event_type->show();
        ui->radioButton_true->show();
        ui->radioButton_false->show();
    }
    else if (checked == false)
    {
        ui->label_condition->hide();
        ui->label_name->hide();
        ui->label_type->hide();
        ui->event_name->hide();
        ui->event_type->hide();
        ui->radioButton_true->hide();
        ui->radioButton_false->hide();
    }
}
