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
#include "Form.h"
#include "ui_Form.h"
#include <QPushButton>
#include <QDebug>
#include <QAction>
#include "add/addproduct.h"
#include "add/adduart.h"
#include "add/addttlv.h"
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
#include <QLabel>
#include <QMessageBox>

Form::Form(QString title,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    this->setObjectName(title);
    ui->setupUi(this);

    /* 添加按钮 */
    ui->pushButton->setText(title);
    ui->tableWidget->setObjectName(title);
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);
    if(tr("添加产品") == title)
    {
        ui->tableWidget->setAcceptDrops(false);
        connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(addProductSlot(bool)));
        QMap<QString,QVariant> info = toolkit.readFormConfig("product");
        QMap<QString, QVariant>::iterator iter = info.begin();
        while (iter != info.end())
        {
            recoverProductResultSlot(iter.key(),iter.value().toMap());
            iter++;
        }
    }
    else if(tr("添加串口") == title)
    {
        ui->tableWidget->setAcceptDrops(false);
        connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(addUartSlot(bool)));
        QMap<QString,QVariant> info = toolkit.readFormConfig("uart");
        QMap<QString, QVariant>::iterator iter = info.begin();
        while (iter != info.end())
        {
            addUartResultSlot(iter.key(),iter.value().toMap());
            iter++;
        }
    }
    else if(tr("添加功能") == title)
    {
        connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(addTtlvSlot(bool)));
        QMap<QString,QVariant> info = toolkit.readFormConfig("ttlv");
        QMap<QString, QVariant>::iterator iter = info.begin();
        while (iter != info.end())
        {
            recoverTtlvResultSlot(iter.key(),iter.value().toMap());
            iter++;
        }
    }
}

Form::~Form()
{
    delete ui;
}

/*
    func:弹出添加产品页面
*/
void Form::addProductSlot(bool flag)
{
    Q_UNUSED(flag);
    addProduct *page = new addProduct("",QMap<QString,QVariant>());
    connect(page,SIGNAL(addResultSignal(QString,QMap<QString,QVariant>)),this,SLOT(addProductResultSlot(QString,QMap<QString,QVariant>)));
    page->show();
}

/*
    func:弹出添加串口页面
*/
void Form::addUartSlot(bool flag)
{
    Q_UNUSED(flag);
    addUart *page = new addUart(ui->tableWidget,"",QMap<QString,QVariant>());
    connect(page,SIGNAL(addResultSignal(QString,QMap<QString,QVariant>)),this,SLOT(addUartResultSlot(QString,QMap<QString,QVariant>)));
    page->show();
}

/*
    func:弹出添加功能页面
*/
void Form::addTtlvSlot(bool flag)
{
    Q_UNUSED(flag);
    addTtlv *page = new addTtlv(ui->tableWidget,LIST_ADD,"",QMap<QString,QVariant>(),BIT_HIDE);
    connect(page,SIGNAL(addResultSignal(QString,QMap<QString,QVariant>)),this,SLOT(addTtlvResultSlot(QString,QMap<QString,QVariant>)));
    page->show();
}

/*
    func:创建新的产品标签
*/
void Form::addProductResultSlot(QString name,QMap<QString,QVariant> info)
{
    QLabel *label = new QLabel(ui->tableWidget);
    label->setAlignment(Qt::AlignCenter);
    label->setText(name);
    ui->tableWidget->setRowCount(1);
    ui->tableWidget->setCellWidget(0,0,label);
    ui->pushButton->setEnabled(false);
    ui->tableWidget->productList.remove(name);
    ui->tableWidget->productList.insert(name,info);
    toolkit.writeFormConfig("product/"+name,info);
    if(info.value("devType").toString().indexOf("网关版") < 0)
    {
        toolkit.removeSubDevInfoConfig();
    }
}

/*
    func:恢复产品标签
*/
void Form::recoverProductResultSlot(QString name,QMap<QString,QVariant> info)
{
    QLabel *label = new QLabel(ui->tableWidget);
    label->setAlignment(Qt::AlignCenter);
    label->setText(name);
    ui->tableWidget->setRowCount(1);
    ui->tableWidget->setCellWidget(0,0,label);
    ui->pushButton->setEnabled(false);
    ui->tableWidget->productList.remove(name);
    ui->tableWidget->productList.insert(name,info);
    if(info.value("devType").toString().indexOf("网关版") < 0)
    {
        toolkit.removeSubDevInfoConfig();
    }
}
/*
    func:修改旧的产品标签
*/
void Form::modProductResultSlot(QString oldName,QString newName,QMap<QString,QVariant> info)
{
    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        QLabel *label = (QLabel*)ui->tableWidget->cellWidget(row,0);
        if(label->text() == oldName)
        {
            label->setText(newName);
            ui->tableWidget->productList.remove(oldName);
            toolkit.removeFormConfig("product/"+oldName);
            ui->tableWidget->productList.insert(newName,info);
            toolkit.writeFormConfig("product/"+newName,info);
            break;
        }
    }
    if(info.value("devType").toString().indexOf("网关版") < 0)
    {
        toolkit.removeSubDevInfoConfig();
    }
    emit modifySignal();
}

/*
    func:创建新的串口标签
*/
void Form::addUartResultSlot(QString name,QMap<QString,QVariant> info)
{
    if(info.isEmpty())
    {
        toolkit.removeFormConfig("uart/"+name);
        return;
    }
    QLabel *label = new QLabel(ui->tableWidget);
    label->setAlignment(Qt::AlignCenter);
    label->setText(name);
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(row+1);
    ui->tableWidget->setCellWidget(row,0,label);
    ui->tableWidget->uartList.insert(name,info);
    toolkit.writeFormConfig("uart/"+name,info);
}

/*
    func:恢复串口标签
*/
void Form::recoverUartResultSlot(QString name,QMap<QString,QVariant> info)
{
    if(info.isEmpty())
    {
        toolkit.removeFormConfig("uart/"+name);
        return;
    }
    QLabel *label = new QLabel(ui->tableWidget);
    label->setAlignment(Qt::AlignCenter);
    label->setText(name);
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(row+1);
    ui->tableWidget->setCellWidget(row,0,label);
    ui->tableWidget->uartList.insert(name,info);
}

/*
    func:修改旧的串口标签
*/
void Form::modUartResultSlot(QString oldName,QString newName,QMap<QString,QVariant> info)
{
    qDebug()<<"修改旧的串口标签";
    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        QLabel *label = (QLabel*)ui->tableWidget->cellWidget(row,0);
        if(label->text() == oldName)
        {
            label->setText(newName);
            ui->tableWidget->uartList.remove(oldName);
            toolkit.removeFormConfig("uart/"+oldName);
            ui->tableWidget->uartList.insert(newName,info);
            toolkit.writeFormConfig("uart/"+newName,info);
            ui->tableWidget->modRegisterUart(oldName,newName);
            break;
        }
    }
}


/*
    func:创建新的功能标签
*/
void Form::addTtlvResultSlot(QString name,QMap<QString,QVariant> info)
{
    qDebug()<<"addTtlvResultSlot "<<"name: " << name <<" info: " << info;
    if(info.isEmpty())
    {
        toolkit.removeFormConfig("ttlv/"+name);
        return;
    }
    QLabel *label = new QLabel(ui->tableWidget);
    /* 不同类型的功能用颜色区分, 红(bool)绿(num)蓝(byte) */
    if("布尔值" == info.find("type").value())
    {
        label->setStyleSheet("background-color:rgb(69,137,148);");
    }
    else if("数值" == info.find("type").value())
    {
        qDebug()<<"绿色";
        label->setStyleSheet("background-color:rgb(117,121,74);");
    }
    else if("字节流" == info.find("type").value())
    {
        label->setStyleSheet("background-color:rgb(114,83,52);");
    }
    else if("枚举" == info.find("type").value())
    {
        label->setStyleSheet("background-color:rgb(205,133,63);");
    }
    if(info.value("type").toString() == "字节流" && info.value("len").toInt() > 240)
    {
        QMessageBox::information(0, tr("警告"), name+tr("长度异常，已强制修改为240，请检查该功能的定义是否正常"),tr("确认"));
        info.insert("len",240);
    }
    label->setAlignment(Qt::AlignCenter);
    int row = ui->tableWidget->rowCount();
    label->setText(name);
    ui->tableWidget->setRowCount(row+1);
    ui->tableWidget->setCellWidget(row,0,label);
    ui->tableWidget->ttlvList.insert(name,info);
    qDebug() << "ttlvList: " << ui->tableWidget->ttlvList;
    toolkit.writeFormConfig("ttlv/"+name,info);
}

/*
    func:恢复功能标签
*/
void Form::recoverTtlvResultSlot(QString name,QMap<QString,QVariant> info)
{
    qDebug()<<"addTtlvResultSlot"<<name<<info;
    if(info.isEmpty())
    {
        toolkit.removeFormConfig("ttlv/"+name);
        return;
    }
    QLabel *label = new QLabel(ui->tableWidget);
    /* 不同类型的功能用颜色区分, 红(bool)绿(num)蓝(byte) */
    if("布尔值" == info.find("type").value())
    {
        label->setStyleSheet("background-color:rgb(69,137,148);");
    }
    else if("数值" == info.find("type").value())
    {
        qDebug()<<"绿色";
        label->setStyleSheet("background-color:rgb(117,121,74);");
    }
    else if("字节流" == info.find("type").value())
    {
        label->setStyleSheet("background-color:rgb(114,83,52);");
    }
    else if("枚举" == info.find("type").value())
    {
        label->setStyleSheet("background-color:rgb(205,133,63);");
    }
    if(info.value("type").toString() == "字节流" && info.value("len").toInt() > 240)
    {
        QMessageBox::information(0, tr("警告"), name+tr("长度异常，已强制修改为240，请检查该功能的定义是否正常"),tr("确认"));
        info.insert("len",240);
    }
    label->setAlignment(Qt::AlignCenter);
    int row = ui->tableWidget->rowCount();
    label->setText(name);
    ui->tableWidget->setRowCount(row+1);
    ui->tableWidget->setCellWidget(row,0,label);
    ui->tableWidget->ttlvList.insert(name,info);
//    qDebug()<<"ttlvList"<<ui->tableWidget->ttlvList;
}

/*
    func:修改旧的功能标签
*/
void Form::modTtlvResultSlot(QString oldName,QString newName,QMap<QString,QVariant> info)
{
    qDebug()<<"替换新的功能"<<oldName<<newName;
    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        QLabel *label = (QLabel*)ui->tableWidget->cellWidget(row,0);
        if(label->text() == oldName)
        {
            label->setText(newName);
            ui->tableWidget->ttlvList.remove(oldName);
            toolkit.removeFormConfig("ttlv/"+oldName);
            ui->tableWidget->ttlvList.insert(newName,info);
            toolkit.writeFormConfig("ttlv/"+newName,info);
            ui->tableWidget->modRegisterTtlv(oldName,newName,info);
            break;
        }
    }
}

