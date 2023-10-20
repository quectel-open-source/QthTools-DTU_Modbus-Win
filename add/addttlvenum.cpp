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
#include "addttlvenum.h"
#include "ui_addttlvenum.h"
#include <QDebug>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>

void setSpinBoxRange(QString text,QSpinBox *spinBox)
{
    if(text.indexOf(QObject::tr("16位有符号整形")) >= 0)
    {
        spinBox->setRange(-32768,32767);
    }
    else if(text.indexOf(QObject::tr("16位无符号整形")) >= 0)
    {
        spinBox->setRange(0,65535);
    }
    else if(text.indexOf(QObject::tr("32位有符号整形")) >= 0)
    {
        spinBox->setRange(-2147483648,2147483647);
    }
    else if(text.indexOf(QObject::tr("32位无符号整形")) >= 0)
    {
        spinBox->setRange(0,2147483647);
    }
}

addTtlvEnum::addTtlvEnum(QMap<QString,QVariant> info,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addTtlvEnum)
{
    ui->setupUi(this);
    qhBoxLayout = new QHBoxLayout(ui->scrollAreaWidgetContents_2);
    qhBoxLayout->setContentsMargins(0,0,0,0);
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableContextMenuRequested(QPoint)));
    table_widget_menu = new QMenu(ui->tableWidget);
    QAction *action_remove = new QAction(tr("删除行"), ui->tableWidget);
    connect(action_remove, SIGNAL(triggered()), this, SLOT(slotActionRemove()));
    table_widget_menu->addAction(action_remove);

    QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5][a-zA-Z0-9_\/\.\u4e00-\u9fa5]+");
    ui->event_name->setValidator(new QRegExpValidator(rx,this));
    ui->event_name->setToolTip(tr("支持中文、大小写字母、\n数字、短划线、下划线、\n斜杠和小数点，必须以中文、\n英文或数字开头，不超过64个字符"));
    ui->checkBox_event->setChecked(false);
    on_checkBox_event_toggled(false);
    ui->event_type->setCurrentText(QObject::tr("告警"));

    if(info.count())
    {
        qDebug()<<"addTtlvEnum"<<info;
        ui->comboBox_numType->setEnabled(false);

        if (info.value("numType").toString() == "16位有符号整形")
        {
            ui->comboBox_numType->setCurrentText(QObject::tr("16位有符号整形"));
        }
        else if (info.value("numType").toString() == "16位无符号整形")
        {
            ui->comboBox_numType->setCurrentText(QObject::tr("16位无符号整形"));
        }
        else if (info.value("numType").toString() == "32位有符号整形")
        {
            ui->comboBox_numType->setCurrentText(QObject::tr("32位有符号整形"));
        }
        else if (info.value("numType").toString() == "32位无符号整形")
        {
            ui->comboBox_numType->setCurrentText(QObject::tr("32位无符号整形"));
        }
//        ui->comboBox_numType->setCurrentText(info.value("numType").toString());
        if (info.value("byte-order").toString() == "大端模式")
        {
            ui->comboBox_order->setCurrentText(QObject::tr("大端模式"));
        }
        else if (info.value("byte-order").toString() == "小端模式")
        {
            ui->comboBox_order->setCurrentText(QObject::tr("小端模式"));
        }
        else if (info.value("byte-order").toString() == "大端模式字节交换")
        {
            ui->comboBox_order->setCurrentText(QObject::tr("大端模式字节交换"));
        }
        else if (info.value("byte-order").toString() == "小端模式字节交换")
        {
            ui->comboBox_order->setCurrentText(QObject::tr("小端模式字节交换"));
        }
//        ui->comboBox_order->setCurrentText(info.value("byte-order").toString());
        QMap<QString,QVariant> enumInfo = info.value("enum").toMap();
        ui->tableWidget->setRowCount(enumInfo.count()+1);
        QMap<QString, QVariant>::iterator iter = enumInfo.begin();
        int row = 0;
        while (iter != enumInfo.end())
        {
            QSpinBox *spinBox = new QSpinBox();
            setSpinBoxRange(ui->comboBox_numType->currentText(),spinBox);
            spinBox->setValue(iter.key().toInt());
            ui->tableWidget->setCellWidget(row,0,spinBox);
            spinBox->setContextMenuPolicy(Qt::NoContextMenu);
            connect(spinBox, SIGNAL(valueChanged(QString)), this, SLOT(isValueChanged(QString)));
            QCheckBox *checkBox = new QCheckBox();
            checkBox->setText(QString::number(spinBox->value()));
            qhBoxLayout->addWidget(checkBox);
            QLineEdit *lineEdit = new QLineEdit(iter.value().toString());
            QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5][a-zA-Z0-9_\u4e00-\u9fa5]+");
            lineEdit->setMaxLength(20);
            lineEdit->setValidator(new QRegExpValidator(rx,this));
            lineEdit->setToolTip(QObject::tr("支持中文、英文大小写、\n数字、下划线，必须以中文、\n英文或数字开头，不超过20个字符"));
            ui->tableWidget->setCellWidget(row,1,lineEdit);
            iter++;
            row++;
        }

        QMap<QString,QVariant> eventInfo = info.value("event").toMap();
        if (eventInfo.value("addEvent").toBool() == true)
        {
            ui->checkBox_event->setChecked(true);
            on_checkBox_event_toggled(true);
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
            QMap<QString, QVariant>::iterator event_iter = conditionsInfo.begin();
            QList<QCheckBox *> checkBoxList = ui->scrollArea->findChildren<QCheckBox *>();
            while (event_iter != conditionsInfo.end())
            {
                if (event_iter.value().toBool() == true)
                {
                    for (int i = 0; i <= (checkBoxList.count() - 1); i++)
                    {
                        if (checkBoxList[i]->text() == event_iter.key())
                        {
                            checkBoxList[i]->setChecked(true);
                        }
                    }
                }
                event_iter ++;
            }
        }
        else
        {
            ui->checkBox_event->setChecked(false);
            on_checkBox_event_toggled(false);
        }
        QPushButton *button = new QPushButton("+");
        connect(button,SIGNAL(clicked(bool)),this,SLOT(addEnumSlot(bool)));
        ui->tableWidget->setCellWidget(row,0,button);
    }
    else
    {
        ui->tableWidget->setRowCount(1);
        QPushButton *button = new QPushButton("+");
        connect(button,SIGNAL(clicked(bool)),this,SLOT(addEnumSlot(bool)));
        ui->tableWidget->setCellWidget(0,0,button);
    }
}

addTtlvEnum::~addTtlvEnum()
{
    delete ui;
}

void addTtlvEnum::tableContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->tableWidget->indexAt(pos);
    removeRow = index.row();
    if (removeRow == ui->tableWidget->rowCount() - 1)
    {
        return;
    }

    if (index.isValid())
    {
        table_widget_menu->exec(QCursor::pos());
    }
}

void addTtlvEnum::slotActionRemove()
{
    ui->tableWidget->removeRow(removeRow);
    QList<QCheckBox *> checkBoxList = ui->scrollArea->findChildren<QCheckBox *>();
    if (removeRow <= (checkBoxList.count() - 1))
    {
        delete checkBoxList[removeRow];
    }
}

void addTtlvEnum::isValueChanged(QString value)
{
    QModelIndex index = ui->tableWidget->selectionModel()->currentIndex();
    int modifyRow = index.row();
    QList<QCheckBox *> checkBoxList = ui->scrollArea->findChildren<QCheckBox *>();
    if (modifyRow <= (checkBoxList.count() - 1))
    {
        checkBoxList[modifyRow]->setText(value);
    }
}

void addTtlvEnum::addEnumSlot(bool flag)
{
    Q_UNUSED(flag);
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(row+1);
    ui->tableWidget->removeCellWidget(row-1,0);
    QSpinBox *spinBox = new QSpinBox();
    setSpinBoxRange(ui->comboBox_numType->currentText(),spinBox);
    int value;
    if(row >= 2)
    {
        QSpinBox *lastBox = (QSpinBox *)ui->tableWidget->cellWidget(row-2,0);
        value = lastBox->value();
        spinBox->setValue(value+1);
    }
    ui->tableWidget->setCellWidget(row-1,0,spinBox);
    spinBox->setContextMenuPolicy(Qt::NoContextMenu);
    connect(spinBox, SIGNAL(valueChanged(QString)), this, SLOT(isValueChanged(QString)));
    QCheckBox *checkBox = new QCheckBox();
    checkBox->setText(QString::number(spinBox->value()));
    qhBoxLayout->addWidget(checkBox);
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5][a-zA-Z0-9_\u4e00-\u9fa5]+");
    lineEdit->setMaxLength(20);
    lineEdit->setValidator(new QRegExpValidator(rx,this));
    lineEdit->setToolTip(QObject::tr("支持中文、英文大小写、\n数字、下划线，必须以中文、\n英文或数字开头，不超过20个字符"));
    ui->tableWidget->setCellWidget(row-1,1,lineEdit);
    QPushButton *button = new QPushButton("+");
    connect(button,SIGNAL(clicked(bool)),this,SLOT(addEnumSlot(bool)));
    ui->tableWidget->setCellWidget(row,0,button);
}

//void addTtlvEnum::on_comboBox_numType_currentTextChanged(const QString &arg1)
//{
//    for (int i = 0; i < ui->tableWidget->rowCount()-1; i++)
//    {
//        QSpinBox *spinBox = (QSpinBox *)ui->tableWidget->cellWidget(i,0);
//        setSpinBoxRange(ui->comboBox_numType->currentText(),spinBox);
//    }
//}

void addTtlvEnum::on_checkBox_event_toggled(bool checked)
{
    if (checked == true)
    {
        ui->label_condition->show();
        ui->label_name->show();
        ui->label_type->show();
        ui->event_name->show();
        ui->event_type->show();
        ui->scrollArea->show();
    }
    else if (checked == false)
    {
        ui->label_condition->hide();
        ui->label_name->hide();
        ui->label_type->hide();
        ui->event_name->hide();
        ui->event_type->hide();
        ui->scrollArea->hide();
    }
}
