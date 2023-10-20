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
#include "addttlvnum.h"
#include "ui_addttlvnum.h"
#include <qlineedit.h>
#include <QDebug>

addTtlvNum::addTtlvNum(QMap<QString,QVariant> info,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addTtlvNum)
{
    ui->setupUi(this);
    QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5][a-zA-Z0-9_\/\.\u4e00-\u9fa5]+");
    ui->event_name->setValidator(new QRegExpValidator(rx,this));
    ui->event_name->setToolTip(tr("支持中文、大小写字母、\n数字、短划线、下划线、\n斜杠和小数点，必须以中文、\n英文或数字开头，不超过64个字符"));
    ui->event_type->setCurrentText(QObject::tr("告警"));

    if(info.count())
    {
        qDebug()<<"addTtlvNum"<<info;
        ui->comboBox_numType->setEnabled(false);
        ui->doubleSpinBox_increment->setValue(info.value("increment").toDouble());
        ui->doubleSpinBox_multiple->setValue(info.value("multiple").toDouble());
        if (info.value("numType").toString() == "16位有符号整形")
        {
            ui->comboBox_numType->setCurrentText(tr("16位有符号整形"));
            ui->register_value_min->setMinimum(-32768 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_min->setMaximum(32767 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMinimum(ui->register_value_min->value());
            ui->register_value_max->setMaximum(32767 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
        else if (info.value("numType").toString() == "16进制有符号A.B型")
        {
            ui->comboBox_numType->setCurrentText(tr("16进制有符号A.B型"));
            ui->register_value_min->setMinimum(-32768 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_min->setMaximum(32767 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMinimum(ui->register_value_min->value());
            ui->register_value_max->setMaximum(32767 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
        else if (info.value("numType").toString() == "16位无符号整形")
        {
            ui->comboBox_numType->setCurrentText(tr("16位无符号整形"));
            ui->register_value_min->setMinimum(ui->doubleSpinBox_increment->value());
            ui->register_value_min->setMaximum(65535 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMinimum(ui->register_value_min->value());
            ui->register_value_max->setMaximum(65535 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
        else if (info.value("numType").toString() == "32位有符号整形")
        {
            ui->comboBox_numType->setCurrentText(tr("32位有符号整形"));
            if (-2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() < -2147483648)
            {
                ui->register_value_min->setMinimum(-2147483648);
            }
            else
            {
                ui->register_value_min->setMinimum(-2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            }
            ui->register_value_max->setMinimum(ui->register_value_min->value());
            if (2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 2147483648)
            {
                ui->register_value_min->setMaximum(2147483647);
                ui->register_value_max->setMaximum(2147483647);
            }
            else
            {
                ui->register_value_min->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
                ui->register_value_max->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            }
        }
        else if (info.value("numType").toString() == "32位无符号整形")
        {
            ui->comboBox_numType->setCurrentText(tr("32位无符号整形"));
            ui->register_value_min->setMinimum(ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMinimum(ui->register_value_min->value());
            if (2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 2147483648)
            {
                ui->register_value_min->setMaximum(2147483647);
                ui->register_value_max->setMaximum(2147483647);
            }
            else
            {
                ui->register_value_min->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
                ui->register_value_max->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            }
        }
        else if (info.value("numType").toString() == "64位有符号整形")
        {
            ui->comboBox_numType->setCurrentText(tr("64位有符号整形"));
            if (-2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() < -2147483648)
            {
                ui->register_value_min->setMinimum(-2147483648);
            }
            else
            {
                ui->register_value_min->setMinimum(-2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            }
            ui->register_value_max->setMinimum(ui->register_value_min->value());
            if (2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 2147483648)
            {
                ui->register_value_min->setMaximum(2147483647);
                ui->register_value_max->setMaximum(2147483647);
            }
            else
            {
                ui->register_value_min->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
                ui->register_value_max->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            }
        }
        else if (info.value("numType").toString() == "64位无符号整形")
        {
            ui->comboBox_numType->setCurrentText(tr("64位无符号整形"));
            ui->register_value_min->setMinimum(ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMinimum(ui->register_value_min->value());
            if (4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 4294967295)
            {
                ui->register_value_min->setMaximum(4294967295);
                ui->register_value_max->setMaximum(4294967295);
            }
            else
            {
                ui->register_value_min->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
                ui->register_value_max->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            }
        }
        else if (info.value("numType").toString() == "32位单精度浮点型")
        {
            ui->comboBox_numType->setCurrentText(tr("32位单精度浮点型"));
            if (-4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() < -4294967295)
            {
                ui->register_value_min->setMinimum(-4294967295);
            }
            else
            {
                ui->register_value_min->setMinimum(-4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            }
            ui->register_value_max->setMinimum(ui->register_value_min->value());
            if (4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 4294967295)
            {
                ui->register_value_min->setMaximum(4294967295);
                ui->register_value_max->setMaximum(4294967295);
            }
            else
            {
                ui->register_value_min->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
                ui->register_value_max->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            }
        }
        else if (info.value("numType").toString() == "64位双精度浮点型")
        {
            ui->comboBox_numType->setCurrentText(tr("64位双精度浮点型"));
            if (-4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() < -4294967295)
            {
                ui->register_value_min->setMinimum(-4294967295);
            }
            else
            {
                ui->register_value_min->setMinimum(-4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            }
            ui->register_value_max->setMinimum(ui->register_value_min->value());
            if (4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 4294967295)
            {
                ui->register_value_min->setMaximum(4294967295);
                ui->register_value_max->setMaximum(4294967295);
            }
            else
            {
                ui->register_value_min->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
                ui->register_value_max->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            }
        }
        if (info.value("byte-order").toString() == "大端模式")
        {
            ui->comboBox_order->setCurrentText(tr("大端模式"));
        }
        else if (info.value("byte-order").toString() == "小端模式")
        {
            ui->comboBox_order->setCurrentText(tr("小端模式"));
        }
        else if (info.value("byte-order").toString() == "大端模式字节交换")
        {
            ui->comboBox_order->setCurrentText(tr("大端模式字节交换"));
        }
        else if (info.value("byte-order").toString() == "小端模式字节交换")
        {
            ui->comboBox_order->setCurrentText(tr("小端模式字节交换"));
        }

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
            QMap<QString,QVariant> conditionsInfo = eventInfo.value("eventConditions").toMap();
            ui->register_value_min->setValue(conditionsInfo.value("miniMum").toDouble());
            ui->register_value_max->setValue(conditionsInfo.value("maxMum").toDouble());
        }
        else
        {
            ui->checkBox_event->setChecked(false);
            on_checkBox_event_clicked(false);
        }
    }
    else
    {
        ui->comboBox_numType->setCurrentText(tr("16位有符号整形"));
        ui->register_value_min->setMinimum(-32768 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        ui->register_value_min->setMaximum(32767 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        ui->register_value_max->setMinimum(-32768 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        ui->register_value_max->setMaximum(32767 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        ui->checkBox_event->setChecked(false);
        on_checkBox_event_clicked(false);
    }
    if (stringIsInt(ui->doubleSpinBox_increment->text()) && stringIsInt(ui->doubleSpinBox_multiple->text()))
    {
        if (ui->comboBox_numType->currentText() == tr("32位单精度浮点型") || ui->comboBox_numType->currentText() == tr("64位双精度浮点型"))
        {
            ui->register_value_min->setDecimals(6);
            ui->register_value_max->setDecimals(6);
        }
        else
        {
            ui->register_value_min->setDecimals(0);
            ui->register_value_max->setDecimals(0);
        }
    }
    else
    {
        ui->register_value_min->setDecimals(6);
        ui->register_value_max->setDecimals(6);
    }
}

addTtlvNum::~addTtlvNum()
{
    delete ui;
}

void addTtlvNum::on_register_value_min_valueChanged(double arg1)
{
    ui->register_value_max->setMinimum(arg1);
}

void addTtlvNum::on_checkBox_event_clicked(bool checked)
{
    if (checked == true)
    {
        ui->label_condition->show();
        ui->label_name->show();
        ui->label_type->show();
        ui->event_name->show();
        ui->event_type->show();
        ui->label_value->show();
        ui->label_10->show();
        ui->register_value_max->show();
        ui->register_value_min->show();
        ui->label_condition->adjustSize();
        ui->label_condition->setAlignment(Qt::AlignVCenter);
    }
    else if (checked == false)
    {
        ui->label_condition->hide();
        ui->label_name->hide();
        ui->label_type->hide();
        ui->event_name->hide();
        ui->event_type->hide();
        ui->label_value->hide();
        ui->label_10->hide();
        ui->register_value_max->hide();
        ui->register_value_min->hide();
    }
}

void addTtlvNum::numericalLimits()
{
    if (ui->comboBox_numType->currentText() == tr("16位有符号整形"))
    {
        ui->register_value_min->setMinimum(-32768 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        ui->register_value_min->setMaximum(32767 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        ui->register_value_max->setMinimum(ui->register_value_min->value());
        ui->register_value_max->setMaximum(32767 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
    }
    else if (ui->comboBox_numType->currentText() == tr("16位无符号整形"))
    {
        ui->register_value_min->setMinimum(ui->doubleSpinBox_increment->value());
        ui->register_value_min->setMaximum(65535 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        ui->register_value_max->setMinimum(ui->register_value_min->value());
        ui->register_value_max->setMaximum(65535 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
    }
    else if (ui->comboBox_numType->currentText() == tr("32位有符号整形"))
    {
        if (-2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() < -2147483648)
        {
            ui->register_value_min->setMinimum(-2147483648);
        }
        else
        {
            ui->register_value_min->setMinimum(-2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
        ui->register_value_max->setMinimum(ui->register_value_min->value());
        if (2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 2147483648)
        {
            ui->register_value_min->setMaximum(2147483647);
            ui->register_value_max->setMaximum(2147483647);
        }
        else
        {
            ui->register_value_min->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
    }
    else if (ui->comboBox_numType->currentText() == tr("32位无符号整形"))
    {
        ui->register_value_min->setMinimum(ui->doubleSpinBox_increment->value());
        ui->register_value_max->setMinimum(ui->register_value_min->value());
        if (2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 2147483648)
        {
            ui->register_value_min->setMaximum(2147483647);
            ui->register_value_max->setMaximum(2147483647);
        }
        else
        {
            ui->register_value_min->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
    }
    else if (ui->comboBox_numType->currentText() == tr("64位有符号整形"))
    {
        if (-2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() < -2147483648)
        {
            ui->register_value_min->setMinimum(-2147483648);
        }
        else
        {
            ui->register_value_min->setMinimum(-2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
        ui->register_value_max->setMinimum(ui->register_value_min->value());
        if (2147483648 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 2147483648)
        {
            ui->register_value_min->setMaximum(2147483647);
            ui->register_value_max->setMaximum(2147483647);
        }
        else
        {
            ui->register_value_min->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMaximum(2147483647 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
    }
    else if (ui->comboBox_numType->currentText() == tr("64位无符号整形"))
    {
        ui->register_value_min->setMinimum(ui->doubleSpinBox_increment->value());
        ui->register_value_max->setMinimum(ui->register_value_min->value());
        if (4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 4294967295)
        {
            ui->register_value_min->setMaximum(4294967295);
            ui->register_value_max->setMaximum(4294967295);
        }
        else
        {
            ui->register_value_min->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
    }
    else if (ui->comboBox_numType->currentText() == tr("32位单精度浮点型"))
    {
        if (-4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() < -4294967295)
        {
            ui->register_value_min->setMinimum(-4294967295);
        }
        else
        {
            ui->register_value_min->setMinimum(-4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
        ui->register_value_max->setMinimum(ui->register_value_min->value());
        if (4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 4294967295)
        {
            ui->register_value_min->setMaximum(4294967295);
            ui->register_value_max->setMaximum(4294967295);
        }
        else
        {
            ui->register_value_min->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
    }
    else if (ui->comboBox_numType->currentText() == tr("64位双精度浮点型"))
    {
        if (-4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() < -4294967295)
        {
            ui->register_value_min->setMinimum(-4294967295);
        }
        else
        {
            ui->register_value_min->setMinimum(-4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
        ui->register_value_max->setMinimum(ui->register_value_min->value());
        if (4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value() > 4294967295)
        {
            ui->register_value_min->setMaximum(4294967295);
            ui->register_value_max->setMaximum(4294967295);
        }
        else
        {
            ui->register_value_min->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
            ui->register_value_max->setMaximum(4294967295 * ui->doubleSpinBox_multiple->value() + ui->doubleSpinBox_increment->value());
        }
    }
}

void addTtlvNum::on_comboBox_numType_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    numericalLimits();
    if (stringIsInt(ui->doubleSpinBox_increment->text()) && stringIsInt(ui->doubleSpinBox_multiple->text()))
    {
        if (ui->comboBox_numType->currentText() == tr("32位单精度浮点型") || ui->comboBox_numType->currentText() == tr("64位双精度浮点型"))
        {
            ui->register_value_min->setDecimals(6);
            ui->register_value_max->setDecimals(6);
        }
        else
        {
            ui->register_value_min->setDecimals(0);
            ui->register_value_max->setDecimals(0);
        }
    }
    else
    {
        ui->register_value_min->setDecimals(6);
        ui->register_value_max->setDecimals(6);
    }
}

void addTtlvNum::on_doubleSpinBox_multiple_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    numericalLimits();
    if (stringIsInt(ui->doubleSpinBox_increment->text()) && stringIsInt(ui->doubleSpinBox_multiple->text()))
    {
        if (ui->comboBox_numType->currentText() == tr("32位单精度浮点型") || ui->comboBox_numType->currentText() == tr("64位双精度浮点型"))
        {
            ui->register_value_min->setDecimals(6);
            ui->register_value_max->setDecimals(6);
        }
        else
        {
            ui->register_value_min->setDecimals(0);
            ui->register_value_max->setDecimals(0);
        }
    }
    else
    {
        ui->register_value_min->setDecimals(6);
        ui->register_value_max->setDecimals(6);
    }
}

void addTtlvNum::on_doubleSpinBox_increment_valueChanged(double arg1)
{
    Q_UNUSED(arg1);
    numericalLimits();
    if (stringIsInt(ui->doubleSpinBox_increment->text()) && stringIsInt(ui->doubleSpinBox_multiple->text()))
    {
        if (ui->comboBox_numType->currentText() == tr("32位单精度浮点型") || ui->comboBox_numType->currentText() == tr("64位双精度浮点型"))
        {
            ui->register_value_min->setDecimals(6);
            ui->register_value_max->setDecimals(6);
        }
        else
        {
            ui->register_value_min->setDecimals(0);
            ui->register_value_max->setDecimals(0);
        }
    }
    else
    {
        ui->register_value_min->setDecimals(6);
        ui->register_value_max->setDecimals(6);
    }
}

/*
    func:判断字符串是否是整数
*/
bool addTtlvNum::stringIsInt(QString value)
{
    int decimal = value.indexOf(".");
    if(decimal < 0)
    {
        return true;
    }
    for (decimal+=1; decimal < value.length(); decimal++)
    {
        if(value.at(decimal) != '0')
        {
            return false;
        }
    }
    return true;
}
