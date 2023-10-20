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
#include "adddev.h"
#include "ui_adddev.h"
#include <QPushButton>
#include <QSpinBox>
#include <QDebug>
#include <QAction>
#include <QLineEdit>
#include <QMessageBox>
#include <QStringList>
#include "myspinbox.h"
#include <QJsonArray>
#include <QCloseEvent>
#include <synchapi.h>

addDev::addDev(bool isGateway,QStringList uartList,QString devName,QMap<QString,QVariant> info,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addDev)
{
    ui->setupUi(this);
    oldName = devName;
    gateway = isGateway;
    this->setWindowFlags(this->windowFlags() |Qt::Dialog);
    this->setWindowModality(Qt::ApplicationModal); //阻塞除当前窗体之外的所有的窗体
    QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5][a-zA-Z0-9_\/\.\u4e00-\u9fa5]+");
    ui->lineEdit_name->setValidator(new QRegExpValidator(rx,this));
    ui->lineEdit_name->setToolTip(tr("支持中文、大小写字母、\n数字、短划线、下划线、\n斜杠和小数点，必须以中文、\n英文或数字开头，不超过20个字符"));
    /* 禁用 最大化按钮 */
    this->setWindowFlags(this->windowFlags()&~Qt::WindowMaximizeButtonHint);
    /* 回车键确认 */
    ui->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::Key_Return);
    /* 限制输入范围 */
    ui->lineEdit_pk->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9]{1,6}"),this));
    ui->lineEdit_ps->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9]{1,16}"),this));
    /* 右键菜单 */
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    table_widget_menu = new QMenu(ui->tableWidget);
    QAction *action = new QAction(tr("删除"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotActionRemove()));
    table_widget_menu->addAction(action);

    /* 生成标题栏 */
    ui->tableWidget->setRowCount(1);
    ui->tableWidget->setColumnCount(uartList.count());
    ui->tableWidget->setHorizontalHeaderLabels(uartList);
    if(!isGateway)
    {
        ui->label_pk->hide();
        ui->label_ps->hide();
        ui->lineEdit_pk->hide();
        ui->lineEdit_ps->hide();
    }
    /* 如果是修改 */
    if(!devName.isEmpty() && !info.isEmpty())
    {
        ui->lineEdit_name->setText(devName);
        if(info.find("writeMode") != info.end())
        {
            if(info.value("writeMode").toMap().find("coil")!= info.value("writeMode").toMap().end())
            {
                int writeCoilMode = info.value("writeMode").toMap().value("coil").toInt();
                ui->comboBox_coilFunc->setCurrentIndex(writeCoilMode);
            }
            if(info.value("writeMode").toMap().find("register")!= info.value("writeMode").toMap().end())
            {
                int writeRegMode = info.value("writeMode").toMap().value("register").toInt();
                ui->comboBox_regFunc->setCurrentIndex(writeRegMode);
            }
        }
        if(isGateway && info.find("product") != info.end())
        {
            QMap<QString, QVariant> productInfo = info.find("product").value().toMap();
            if(productInfo.find("productKey") != productInfo.end())
            {
                ui->lineEdit_pk->setText(productInfo.find("productKey").value().toString());
            }
            if(productInfo.find("productSecret") != productInfo.end())
            {
                ui->lineEdit_ps->setText(productInfo.find("productSecret").value().toString());
            }
        }
        qDebug()<<"devName"<<devName;
        QMap<QString, QVariant> addrInfo = info.find("addr").value().toMap();
        for (int column = 0; column < ui->tableWidget->horizontalHeader()->count(); column++)
        {

            QMap<QString, QVariant>::iterator iter = addrInfo.begin();
            qDebug()<<"iter"<<iter.key()<<iter.value();
            while (iter != addrInfo.end())
            {
                qDebug()<<ui->tableWidget->horizontalHeaderItem(column)->text()<<iter.key();
                if(ui->tableWidget->horizontalHeaderItem(column)->text() == iter.key())
                {
                    QJsonArray devArray = iter.value().toJsonArray();
                    if(devArray.count() && devArray.count()+1 > ui->tableWidget->rowCount())
                    {
                        ui->tableWidget->setRowCount(devArray.count()+1);
                    }
                    for (int row = 0; row < devArray.count(); row++)
                    {
                        /* 增加新的控件 */
                        MySpinBox *spinBox = new MySpinBox();
                        spinBox->setRange(1,247);
                        spinBox->setValue(devArray.at(row).toInt());
                        spinBox->setContextMenuPolicy(Qt::CustomContextMenu);
                        connect(spinBox,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(on_tableWidget_customContextMenuRequested(QPoint)));
                        ui->tableWidget->setCellWidget(row,column,spinBox);
                    }
                    break;
                }
                iter++;
            }
        }
    }
    /* 生成添加按钮 */
    qDebug()<<uartList.count();
    for(int column=0;column<uartList.count();column++)
    {
        for (int row = 0; row < ui->tableWidget->rowCount(); row++)
        {
            if(ui->tableWidget->cellWidget(row,column) == nullptr)
            {
                QPushButton *pushButton = new QPushButton(tr("添加从机地址"));
                ui->tableWidget->setCellWidget(row,column,pushButton);
                connect(pushButton,SIGNAL(clicked(bool)),this,SLOT(addDevIdSlot(bool)));
                if (gateway != true && row >= 1)
                {
                    pushButton->setEnabled(false);
                }
                break;
            }
        }
    }
}

addDev::~addDev()
{
    delete ui;
}

/*
    func:增加从机设备
*/
void addDev::addDevIdSlot(bool flag)
{
    Q_UNUSED(flag);
    QPushButton *button_tmp=qobject_cast<QPushButton *>(sender());
    for (int row = 0; row< ui->tableWidget->rowCount(); row++)
    {
        for (int column = 0; column < ui->tableWidget->columnCount(); column++)
        {
            QWidget *box = ui->tableWidget->cellWidget(row,column);
            if(box && "QPushButton" == QString(box->metaObject()->className()) && button_tmp == (QPushButton *)box)
            {
                if(row == ui->tableWidget->rowCount()-1)
                {
                    /* 增加新行 */
                    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
                }
                /* 增加新的控件 */
                MySpinBox *spinBox = new MySpinBox();
                if(row > 0)
                {
                    MySpinBox *box_last = (MySpinBox *)ui->tableWidget->cellWidget(row-1,column);
                    spinBox->setValue(box_last->value()+1);
                }
                spinBox->setRange(1,247);
                spinBox->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(spinBox,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(on_tableWidget_customContextMenuRequested(QPoint)));
                ui->tableWidget->setCellWidget(row,column,spinBox);
                /* 添加按钮下移 */
                QPushButton *pushButton = new QPushButton(tr("添加从机地址"));
                ui->tableWidget->setCellWidget(row+1,column,pushButton);
                connect(pushButton,SIGNAL(clicked(bool)),this,SLOT(addDevIdSlot(bool)));
                QModelIndex index = ui->tableWidget->selectionModel()->currentIndex();
                if (gateway != true && index.row() >= 0)
                {
                    pushButton->setEnabled(false);
                }
                return;
            }
        }
    }

}

/*
    func:判断数据是否有重复
*/
bool addDev::IsContainRepeat(QJsonArray arr)
{
    for ( int i = 0 ; i < arr.count(); i ++ )
    {
        for ( int j = 0 ; j < arr.count(); j ++ )
        {
            if (j != i)
            {
                if (arr.at(j) == arr.at(i))
                return true;
            }
        }
    }
    return false;
}


/*
    func:检查是否和已添加的地址重复
*/
bool addDev::IsContainRepeat(QString oldName,QString port,int addr)
{
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev");
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end())
    {
        if(iter.key() != oldName)
        {
            QMap<QString,QVariant> addrInfo = iter.value().toMap().value("addr").toMap();
            QJsonArray slaveInfo = addrInfo.value(port).toJsonArray();
            foreach (QJsonValue currAddr, slaveInfo)
            {
                if(currAddr.toInt() == addr)
                {
                    return true;
                }
            }
        }
        iter++;
    }
    return false;
}

/*
    func:检查名称是否重复
*/
bool addDev::IsContainRepeat(QString name)
{
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev");
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end())
    {
        if(iter.key() == name)
        {
            return true;
        }
        iter++;
    }
    return false;
}

/*
    func:检查产品PK是否重复(网关版)
*/
bool addDev::pkIsContainRepeat(QString oldName,QString pk)
{
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev");
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end() && pk.length())
    {
        QMap<QString,QVariant> devInfo = iter.value().toMap();
        QMap<QString,QVariant> productInfo = devInfo.find("product").value().toMap();
        if(!productInfo.isEmpty())
        {
            QString productKey = productInfo.find("productKey").value().toString();
            if(oldName != iter.key() && productKey == pk)
            {
                return true;
            }
        }
        iter++;
    }
    return false;
}

/*
    func:点击确定
*/
void addDev::on_buttonBox_accepted()
{
    QString newName = ui->lineEdit_name->text();
    if(0 == newName.length())
    {
        QMessageBox::information(0,tr("添加从机设备失败"),QObject::tr("请输入设备名称"),tr("确认"));
        return;
    }
    if(oldName != newName && IsContainRepeat(newName))
    {
        QMessageBox::information(0,tr("添加设备失败"),tr("该设备名称已存在"),tr("确认"));
        return;
    }
    QMap<QString,QVariant> info;
    QMap<QString,QVariant> devInfo;
    QMap<QString,QVariant> writeModeInfo;
    bool devIsEmpty = true;
    for(int column=0;column<ui->tableWidget->columnCount();column++)
    {
        QString port = ui->tableWidget->horizontalHeaderItem(column)->text();
        QJsonArray slaveArray;
        for(int row=0;row<ui->tableWidget->rowCount();row++)
        {
            QWidget *box = ui->tableWidget->cellWidget(row,column);
            if(box && "MySpinBox" == QString(box->metaObject()->className()))
            {
                int addr = ((MySpinBox*)box)->value();
                if(IsContainRepeat(oldName,port,addr))
                {
                    QMessageBox::information(0,tr("添加从机设备失败"),tr("已有设备存在于")+tr("串口")+port+tr("和从机地址%1").arg(QString::number(addr)),tr("确认"));
                    return;
                }
                slaveArray.append(addr);
                devIsEmpty = false;
            }
        }
        qDebug()<<"slaveArray";
        if(IsContainRepeat(slaveArray))
        {
            QMessageBox::information(0,tr("添加从机设备失败"),tr("同一串口下不允许存在相同的从机地址"),tr("确认"));
            return;
        }
        devInfo.insert(ui->tableWidget->horizontalHeaderItem(column)->text(),slaveArray);
    }
    if(devIsEmpty)
    {
        QMessageBox::information(0,tr("添加从机设备失败"),tr("请至少添加一个从机设备"),tr("确认"));
        return;
    }

    QMap<QString,QVariant> productInfo;
    if(!ui->lineEdit_pk->isHidden())
    {
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
        if(false == ui->lineEdit_pk->isHidden() && pkIsContainRepeat(oldName,ui->lineEdit_pk->text()))
        {
            QMessageBox::information(0,tr("添加产品失败"),tr("当前输入的ProductKey和其他子设备重复，请检查输入内容"),tr("确认"));
            return;
        }
        productInfo.insert("productKey",ui->lineEdit_pk->text());
        productInfo.insert("productSecret",ui->lineEdit_ps->text());
    }
    info.insert("product",productInfo);
    info.insert("addr",devInfo);
    writeModeInfo.insert("coil",ui->comboBox_coilFunc->currentIndex());
    writeModeInfo.insert("register",ui->comboBox_regFunc->currentIndex());
    info.insert("writeMode",writeModeInfo);
    if(oldName.length())
    {
        emit modResultSignal(oldName,newName,info);
    }
    else
    {
        emit addResultSignal(newName,info,true);
    }
    this->close();
}

/*
    func:点击取消
*/
void addDev::on_buttonBox_rejected()
{
    emit unselect();
    this->close();
}

void addDev::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    emit unselect();
}

/*
    func:弹出右键菜单
*/
void addDev::on_tableWidget_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);
    int column = ui->tableWidget->currentColumn();
    int row = ui->tableWidget->currentRow();
    QWidget *box = ui->tableWidget->cellWidget(row,column);
    if(box && QString(box->metaObject()->className()) == "MySpinBox")
    {
        table_widget_menu->exec(QCursor::pos());
    }
}

/*
    func:右键菜单Reomve处理
*/
void addDev::slotActionRemove()
{
    int column = ui->tableWidget->currentColumn();
    int row = ui->tableWidget->currentRow();
    /* 若删除的是添加按钮直接返回 */
    QWidget *box = ui->tableWidget->cellWidget(row,column);
    if(box && QString(box->metaObject()->className()) == "QPushButton")
    {
        return;
    }

    for (; row< ui->tableWidget->rowCount()-1; row++)
    {
        box = ui->tableWidget->cellWidget(row+1,column);
        if(box && QString(box->metaObject()->className()) == "QPushButton")
        {
            /* 添加按钮上移 */
            ui->tableWidget->removeCellWidget(row+1,column);
            QPushButton *pushButton = new QPushButton(tr("添加从机地址"));
            ui->tableWidget->setCellWidget(row,column,pushButton);
            connect(pushButton,SIGNAL(clicked(bool)),this,SLOT(addDevIdSlot(bool)));
            break;
        }
        else
        {
            /* 设备列表上移 */
            MySpinBox *box_new = new MySpinBox();
            box_new->setValue(((MySpinBox *)box)->value());
            box_new->setRange(1,247);
            box_new->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(box_new,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(on_tableWidget_customContextMenuRequested(QPoint)));
            ui->tableWidget->removeCellWidget(row,column);
            ui->tableWidget->setCellWidget(row,column,box_new);
        }
    }

    /* 删除空白行 */
    for (int i = 0; i < ui->tableWidget->columnCount(); i++)
    {
        box = ui->tableWidget->cellWidget(ui->tableWidget->rowCount()-1,i);
        if(box && QString(box->metaObject()->className()) == "QPushButton")
        {
            return;
        }
    }
    ui->tableWidget->removeRow(ui->tableWidget->rowCount()-1);
}

