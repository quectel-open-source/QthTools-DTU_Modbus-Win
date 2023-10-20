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
#include "addregister.h"
#include "ui_addregister.h"
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QTabWidget>
#include "preqtablewidget.h"
addRegister::addRegister(QString name,QMap<QString,QVariant> info,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addRegister)
{
    Q_UNUSED(info);
    ui->setupUi(this);
    oldName = name;
    this->setWindowFlags(this->windowFlags() |Qt::Dialog);
    this->setWindowModality(Qt::ApplicationModal); //阻塞除当前窗体之外的所有的窗体
    /* 禁用 最大化按钮 */
    this->setWindowFlags(this->windowFlags()&~Qt::WindowMaximizeButtonHint);
    /* 回车键确认 */
    ui->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::Key_Return);
    /* 限制输入范围 */
    ui->lineEdit_startAddr->setValidator(new QRegExpValidator(QRegExp("[a-fA-F0-9]{1,4}"),this));
    if(!name.isEmpty())
    {
        /* 修改寄存器列表范围 */
        QRegExp rx("0x(.*)-0x(.*)");
        if(-1 != name.indexOf(rx))
        {
            int startAddr = rx.cap(1).toInt(nullptr,16);
            int endAddr = rx.cap(2).toInt(nullptr,16);
            int num = endAddr-startAddr+1;
            qDebug()<<"修改寄存器列表 startAddr"<<startAddr<<endAddr<<num;
            ui->lineEdit_startAddr->setText(rx.cap(1));
            ui->spinBox_num->setValue(num);
        }
    }
}

addRegister::~addRegister()
{
    delete ui;
}

/*
    func:检查地址是否重复
*/
bool addRegister::IsContainRepeat(int startAddr,int endAddr)
{
    QTabWidget *tab = (QTabWidget *)toolkit.findParent(this,"QTabWidget",nullptr);
    QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab,"QTabWidget",nullptr);
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev");

    QMap<QString,QVariant> addrInfo;
    if (tab->tabText(tab->currentIndex()) == tr("线圈"))
    {
        addrInfo = info.value(tab2->tabText(tab2->currentIndex())).toMap().value("register").toMap().value("线圈").toMap();
    }
    else if (tab->tabText(tab->currentIndex()) == tr("保持寄存器"))
    {
        addrInfo = info.value(tab2->tabText(tab2->currentIndex())).toMap().value("register").toMap().value("保持寄存器").toMap();
    }
    else if (tab->tabText(tab->currentIndex()) == tr("输入寄存器"))
    {
        addrInfo = info.value(tab2->tabText(tab2->currentIndex())).toMap().value("register").toMap().value("输入寄存器").toMap();
    }
    else if (tab->tabText(tab->currentIndex()) == tr("离散量"))
    {
        addrInfo = info.value(tab2->tabText(tab2->currentIndex())).toMap().value("register").toMap().value("离散量").toMap();
    }

    QMap<QString, QVariant>::iterator addrIter = addrInfo.begin();
    while (addrIter != addrInfo.end())
    {
        QString addr = addrIter.key();
        if(addr != oldName)
        {
            QRegExp rx("0x(.*)-0x(.*)");
            if(-1 != addr.indexOf(rx))
            {
                int startAddr_table = rx.cap(1).toInt(nullptr,16);
                int endAddr_table = rx.cap(2).toInt(nullptr,16);
                qDebug()<<"startAddr_table"<<addr<<startAddr_table<<endAddr_table;
                qDebug()<<"startAddr"<<startAddr<<endAddr;
                if((startAddr >= startAddr_table && startAddr <= endAddr_table)
                        || (endAddr >= startAddr_table && endAddr <= endAddr_table)
                        || (startAddr_table >= startAddr && startAddr_table <= endAddr)
                        || (endAddr_table >= startAddr && endAddr_table <= endAddr))
                {
                    return true;
                }
            }
        }
        addrIter++;
    }
    return false;
}

void addRegister::on_buttonBox_accepted()
{
    if(0 == ui->lineEdit_startAddr->text().length())
    {
        QMessageBox::information(0,tr("添加寄存器失败"),tr("输入不能为空"),tr("确认"));
        return;
    }
    int startAddr;
    if(ui->checkBox->isChecked())
    {
        startAddr = ui->lineEdit_startAddr->text().toInt(nullptr,16);
    }
    else
    {
        startAddr = ui->lineEdit_startAddr->text().toInt(nullptr,10);
        if(startAddr > 65535)
        {
            QMessageBox::information(0,tr("添加寄存器失败"),tr("超过寄存器地址范围，最大值为65535(0XFFFF)"),tr("确认"));
            return;
        }
    }
    int num = ui->spinBox_num->value();
    int endAddr = startAddr+num-1;
    if(endAddr > 0xffff)
    {
        QMessageBox::information(0,tr("添加寄存器失败"),tr("寄存器地址超过最大地址长度"),tr("确认"));
        return;
    }
    /* 检测是否存在冲突地址 */
    if(IsContainRepeat(startAddr,endAddr))
    {
        QMessageBox::information(0,tr("添加寄存器失败"),tr("寄存器地址冲突，请检查已经添加的寄存器列表"),tr("确认"));
        return;
    }
    QString name;
    name.sprintf("0x%04x-0x%04x",startAddr,endAddr);
    QMap<QString,QVariant> info;
    info.insert("startAddr",startAddr);
    info.insert("num",num);
    if(oldName.length())
    {
        /* 修改列表需要额外判断是否满足条件：修改后的长度不能小于已被占用的长度 */
        PreQTableWidget *table = (PreQTableWidget *)this->parent();
        int validRow;
        for (validRow = table->rowCount()-1; validRow > 0; validRow--)
        {
            QWidget *box = table->cellWidget(validRow,1);
            if(box)
            {
                qDebug()<<"box"<<validRow<<table->rowSpan(validRow,1)<<box->layout()->count()<<box->layout()->itemAt(0)->widget();
                if(box->layout()->count() > 1 || "QPushButton" != QString(box->layout()->itemAt(0)->widget()->metaObject()->className()))
                {
                    validRow += table->rowSpan(validRow,1)-1;
                    break;
                }
            }
        }
        qDebug()<<tr("被占用行数")<<validRow;
        if(num < validRow)
        {
            QMessageBox::information(0,tr("修改寄存器列表失败"),tr("当前列表中所占用长度大于修改长度，请删除列表中多余的功能标签"),tr("确认"));
            return;
        }
        emit modResultSignal(oldName,name,info);
    }
    else
    {
        emit addResultSignal(name,info);
    }
    this->close();
}

void addRegister::on_buttonBox_rejected()
{
    this->close();
}

void addRegister::on_checkBox_stateChanged(int arg1)
{
    if(arg1)
    {
        ui->lineEdit_startAddr->setValidator(new QRegExpValidator(QRegExp("[a-fA-F0-9]{1,4}"),this));
        ui->lineEdit_startAddr->setText(QString::number(ui->lineEdit_startAddr->text().toInt(nullptr,10),16));
    }
    else
    {
        ui->lineEdit_startAddr->setValidator(new QRegExpValidator(QRegExp("^(\\d|[1-9]\\d|([1-9][0-9][0-9])|([1-9][0-9][0-9][0-9])|([1-5][0-9][0-9][0-9][0-9]|6[0-5][0-5][0-3][0-5]))$"),this));
        ui->lineEdit_startAddr->setText(QString::number(ui->lineEdit_startAddr->text().toInt(nullptr,16),10));
    }
}
