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
#include "addproduct.h"
#include "ui_addproduct.h"
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>


addProduct::addProduct(QString name,QMap<QString,QVariant> info,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addProduct)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(this->windowFlags() |Qt::Dialog);
    this->setWindowModality(Qt::ApplicationModal); //阻塞除当前窗体之外的所有的窗体
    /* 禁用 最大化按钮 */
    this->setWindowFlags(this->windowFlags()&~Qt::WindowMaximizeButtonHint);
    /* 回车键确认 */
    ui->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::Key_Return);
    /* 限制输入范围 */
    ui->lineEdit_pk->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9]{1,6}"),this));
    ui->lineEdit_ps->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9]{1,16}"),this));
    if(info.count())
    {
        oldName = name;
        ui->lineEdit_name->setText(name);
        ui->lineEdit_pk->setText(info.find("productKey").value().toString());
        ui->lineEdit_ps->setText(info.find("productSecret").value().toString());
        /* 如果不存在otaInvt, 设置默认值24 */
        if (info.find("otaInvt") == info.end())
        {
            ui->spinBox_otaPollInterval->setValue(24);
        }
        else
        {
            ui->spinBox_otaPollInterval->setValue(info.find("otaInvt").value().toInt());
        }
        if (info.find("report").value().toString() == "变化上报")
        {
            ui->comboBox_report->setCurrentText(tr("变化上报"));
        }
        else if (info.find("report").value().toString() == "全部上报")
        {
            ui->comboBox_report->setCurrentText(tr("全部上报"));
        }

        if (info.find("devType").value().toString() == "直连版")
        {
            ui->comboBox_devType->setCurrentText(tr("直连版"));
        }
        else if (info.find("devType").value().toString() == "网关版")
        {
            ui->comboBox_devType->setCurrentText(tr("网关版"));
        }
    }
}

addProduct::~addProduct()
{
    delete ui;
}

void addProduct::on_buttonBox_accepted()
{
    if(0 == ui->lineEdit_name->text().length())
    {
        QMessageBox::information(0,tr("添加产品失败"),tr("请输入产品名称"),tr("确认"));
        return;
    }
    if(6 != ui->lineEdit_pk->text().length())
    {
        QMessageBox::information(0,tr("添加产品失败"),tr("请检查ProductKey输入内容长度"),tr("确认"));
        return;
    }
    if(16 != ui->lineEdit_ps->text().length())
    {
        QMessageBox::information(0,tr("添加产品失败"),tr("请检查ProductSecert输入内容长度"),tr("确认"));
        return;
    }
    QMap<QString,QVariant> info;
    info.insert("productKey",ui->lineEdit_pk->text());
    info.insert("productSecret",ui->lineEdit_ps->text());
    info.insert("otaInvt", ui->spinBox_otaPollInterval->value());
    if (ui->comboBox_report->currentText() == tr("变化上报"))
    {
        info.insert("report","变化上报");

    }
    else if (ui->comboBox_report->currentText() == tr("全部上报"))
    {
        info.insert("report","全部上报");

    }
    if (ui->comboBox_devType->currentText() == tr("直连版"))
    {
        QMap<QString,QVariant> checkInfo = toolkit.readFormConfig("dev");
        QMap<QString, QVariant>::iterator iter = checkInfo.begin();
        int productNumber = 0;
        while (iter != checkInfo.end())
        {
            productNumber ++;
            if (productNumber >= 2)
            {
                QMessageBox::information(0,tr("修改产品失败"),tr("直连版只能添加一个设备型号，请删除后再试"),tr("确认"));
                return;
            }
            QMap<QString,QVariant> checkSlaveProduct = toolkit.readFormConfig("dev/" + iter.key() + "/addr");
            QMap<QString, QVariant>::iterator checkiter = checkSlaveProduct.begin();
            qDebug()<<checkSlaveProduct;
            while (checkiter != checkSlaveProduct.end())
            {
                QList<QVariant> arrayList = checkiter.value().toList();
                if (arrayList.count() >= 2)
                {
                    QMessageBox::information(0,tr("修改产品失败"),tr("直连版只能添加一个从机地址，请删除后再试"),tr("确认"));
                    return;
                }
                checkiter ++;
            }
            iter ++;
        }
        info.insert("devType","直连版");
    }
    else if (ui->comboBox_devType->currentText() == tr("网关版"))
    {
        info.insert("devType","网关版");
    }

    if(oldName.length())
    {
        emit modResultSignal(oldName,ui->lineEdit_name->text(),info);
    }
    else
    {
        emit addResultSignal(ui->lineEdit_name->text(),info);
    }
    this->close();
}

void addProduct::on_buttonBox_rejected()
{
    this->close();
}
