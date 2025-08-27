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
#include "addregisterform.h"
#include "ui_addregisterform.h"
#include <QPushButton>
#include <QLabel>
#include "add/addttlv.h"
#include "addregister.h"
#include <qtablewidget.h>
#include <QHeaderView>
#include <QDebug>
#include <QAction>
#include "preqtablewidget.h"
#include "Form.h"
#include <QMessageBox>
#include <QMouseEvent>
#include <QTableWidgetItem>
#include <QPainter>
#include <QTabWidget>
addRegisterForm::addRegisterForm(QMap<QString,QVariant> info,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addRegisterForm)
{
    ui->setupUi(this);
    initCompleted = false;
    listToolBox = new ToolBox(this);
    QGridLayout *listLayout = new QGridLayout(ui->widget);
    listLayout->addWidget(listToolBox);
    listLayout->setContentsMargins(0,0,0,0);

    /* 右键菜单 */
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableContextMenuRequested(QPoint)));
    table_widget_menu = new QMenu(this);
    QAction *action_modify = new QAction(tr("修改"), this);
    connect(action_modify, SIGNAL(triggered()), this, SLOT(slotActionModify()));
    table_widget_menu->addAction(action_modify);
    QAction *action_remove = new QAction(tr("删除"), this);
    connect(action_remove, SIGNAL(triggered()), this, SLOT(slotActionRemove()));
    table_widget_menu->addAction(action_remove);

    if(info.count())
    {
        QMap<QString, QVariant>::iterator iter = info.begin();
        while (iter != info.end())
        {
            QString name = iter.key();
            QMap<QString, QVariant> registerMap = iter.value().toMap();
            qDebug()<<"恢复寄存器列表 name"<<name;
            QRegExp rx("0x(.*)-0x(.*)");
            if(-1 != name.indexOf(rx))
            {
                int startAddr = rx.cap(1).toInt(nullptr,16);
                int endAddr = rx.cap(2).toInt(nullptr,16);
                int num = endAddr-startAddr+1;
                qDebug()<<"恢复寄存器列表 startAddr"<<startAddr<<endAddr<<num;
                QMap<QString,QVariant> formInfo;
                formInfo.insert("startAddr",startAddr);
                formInfo.insert("num",num);
                addRegListResultSlot(name,formInfo);
            }
            addRegisterFormForTtlv(name,registerMap);
            iter++;
        }
    }
    initCompleted = true;
}

addRegisterForm::~addRegisterForm()
{
    delete ui;
}

/*
    func:恢复配置文件中的功能标签
*/
#include <QJsonArray>
#include <QJsonObject>
void addRegisterForm::addRegisterFormForTtlv(QString name,QMap<QString,QVariant> info)
{
    QList<PreQTableWidget *> tableList = this->findChildren<PreQTableWidget *>();
    PreQTableWidget *table;
    foreach (table, tableList)
    {
        if(name == table->objectName())
        {
            break;
        }
    }
    if(table == nullptr)
    {
        qDebug()<<tr("找不到列表");
        return;
    }
    qDebug()<<"addRegisterFormForTtlv table"<<table<<table->rowCount()<<table->columnCount();
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end())
    {
        QString ttlvName = iter.key();
        if(!iter.value().toMap().isEmpty())
        {
            QJsonArray posArray = iter.value().toMap().find("pos").value().toJsonArray();
            QMap<QString,QVariant> attrInfo = iter.value().toMap().find("attr").value().toMap();
            qDebug()<<ttlvName<<attrInfo;
            int lastRow = -1;
            int lastBit = -1;
            for (int iPos = 0; iPos < posArray.count(); iPos++)
            {
                QJsonObject posObj = posArray.at(iPos).toObject();
                int row = posObj.value("row").toInt();
                int column = posObj.value("column").toInt();
                int len = posObj.value("span").toInt();
                int bit = posObj.value("bit").toInt();
                if(lastRow == row && lastBit == bit)
                {
                    QMessageBox::information(0,tr("警告"),tr("功能标签地址出现重复：")+ttlvName+"_"+QString::number(row)+"_"+QString::number(bit),"确认");
                }
                lastRow = row;
                lastBit = bit;
                qDebug()<<row<<column<<len;
                QObject *obj = table->cellWidget(row,column);
                qDebug()<<"addRegisterFormForTtlv obj"<<obj<<ttlvName<<attrInfo;
                this->addRegResultSlot(obj,ttlvName,attrInfo,bit,false);
            }
        }
        iter++;
    }
}

/*
    func:弹出添加寄存器页面
*/
void addRegisterForm::on_pushButton_clicked()
{
    addRegister *page = new addRegister("",QMap<QString,QVariant>(),this);
    connect(page,SIGNAL(addResultSignal(QString,QMap<QString,QVariant>)),this,SLOT(addRegListResultSlot(QString,QMap<QString,QVariant>)));
    page->show();
}

/*
    func:增加寄存器列表
*/
void addRegisterForm::addRegListResultSlot(QString name,QMap<QString,QVariant> info)
{
    qDebug()<<info<<info.find("num").value();
    int startAddr = info.find("startAddr").value().toInt();
    int num = info.find("num").value().toInt();
    qDebug()<<"addRegListResultSlot startAddr"<<startAddr<<num;

    PreQTableWidget *table = new PreQTableWidget(this);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setFocusPolicy(Qt::NoFocus);
    table->setObjectName(name);
    table->setMinimumHeight(200);
    listToolBox->addWidget(name,table);
    table->setRowCount(num+1);
    table->setColumnCount(2);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setVisible(false);
    table->setColumnWidth(0,130);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setAcceptDrops(true);
    QTableWidgetItem *item1 = new QTableWidgetItem(tr("寄存器地址"));
    item1->setTextAlignment(Qt::AlignCenter);
    table->setItem(0,0,item1);
    QTableWidgetItem *item2 = new QTableWidgetItem(tr("功能标签"));
    item2->setTextAlignment(Qt::AlignCenter);
    table->setItem(0,1,item2);
    table->setRowHidden(0,true);
    for(int i = 0;i<num;i++)
    {
        QString itemAddr;
        itemAddr.sprintf("%04x",startAddr+i);
        QTableWidgetItem *item3 = new QTableWidgetItem("0x"+itemAddr);
        item3->setTextAlignment(Qt::AlignCenter);
        table->setItem(i+1,0,item3);
        QHBoxLayout *vLayout = new QHBoxLayout(table->cellWidget(i+1,1));
        QWidget *Widget_btn = new QWidget;
        Widget_btn->setObjectName("ttlvWidget");
        QPushButton * addTtvlButton = new QPushButton("+");
        connect(addTtvlButton,SIGNAL(clicked(bool)),this,SLOT(addTtlvSlot(bool)));
        vLayout->addWidget(addTtvlButton);
        vLayout->setContentsMargins(0,0,0,0);
        Widget_btn->setLayout(vLayout);
        table->setCellWidget(i+1,1,Widget_btn);
    }
    /* 保存到配置文件 */
    if(initCompleted)
    {
        QTabWidget *tab1 = (QTabWidget *)toolkit.findParent(this,"QTabWidget",nullptr);
        QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab1,"QTabWidget",nullptr);
        QString key;
        if (tab1->tabText(tab1->currentIndex()) == tr("线圈"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/线圈/"+table->objectName();
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("保持寄存器"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/保持寄存器/"+table->objectName();
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("输入寄存器"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/输入寄存器/"+table->objectName();
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("离散量"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/离散量/"+table->objectName();
        }

//        QString key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/"+tab1->tabText(tab1->currentIndex())+"/"+table->objectName();
        toolkit.writeFormConfig(key,QMap<QString,QVariant>());
    }
}

/*
    func:修改寄存器列表
*/
void addRegisterForm::modRegListResultSlot(QString oldName,QString newName,QMap<QString,QVariant> info)
{
    QList<PreQTableWidget *> tableList = this->listToolBox->findChildren<PreQTableWidget *>();
    QList<QPushButton *> buttonList = this->listToolBox->findChildren<QPushButton *>("pushButtonFold");
    for (int i = 0; i < tableList.count(); i++)
    {
        PreQTableWidget *table = tableList.at(i);
        if(table->objectName() == oldName)
        {
            qDebug()<<"修改寄存器列表内容信息"<<oldName<<newName;
            int startAddr = info.find("startAddr").value().toInt();
            int num = info.find("num").value().toInt();
            int oldRow = table->rowCount();
            table->setObjectName(newName);
            table->setRowCount(num+1);
            for(int i = 0;i<num;i++)
            {
                QString itemAddr;
                itemAddr.sprintf("%04x",startAddr+i);
                table->setItem(i+1,0,new QTableWidgetItem("0x"+itemAddr));
            }
            if(num+1 > oldRow)
            {
                int addRow = num+1-oldRow;//6-2=4
                qDebug()<<"增加新行"<<addRow<<oldRow;
                for(int i = oldRow;i<num+1;i++)
                {
                    QHBoxLayout *vLayout = new QHBoxLayout(table->cellWidget(i,1));
                    QWidget *Widget_btn = new QWidget;
                    Widget_btn->setObjectName("ttlvWidget");
                    QPushButton * addTtvlButton = new QPushButton("+");
                    connect(addTtvlButton,SIGNAL(clicked(bool)),this,SLOT(addTtlvSlot(bool)));
                    vLayout->addWidget(addTtvlButton);
                    vLayout->setContentsMargins(0,0,0,0);
                    Widget_btn->setLayout(vLayout);
                    table->setCellWidget(i,1,Widget_btn);
                }
            }
            break;
        }
    }
    for (int i = 0; i < buttonList.count(); i++)
    {
        QPushButton *button = buttonList.at(i);
        if(button->text() == oldName)
        {
            qDebug()<<"修改寄存器列表按钮信息"<<oldName<<newName;
            button->setText(newName);
            break;
        }
    }
    /* 保存到配置文件 */
    if(initCompleted)
    {
        QTabWidget *tab1 = (QTabWidget *)toolkit.findParent(this,"QTabWidget",nullptr);
        QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab1,"QTabWidget",nullptr);
        QString key;
        if (tab1->tabText(tab1->currentIndex()) == tr("线圈"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/线圈/";
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("保持寄存器"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/保持寄存器/";
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("输入寄存器"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/输入寄存器/";
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("离散量"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/离散量/";
        }
//      QString key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/"+tab1->tabText(tab1->currentIndex())+"/";
        QMap<QString,QVariant> oldInfo = toolkit.readFormConfig(key+oldName);
        toolkit.removeFormConfig(key+oldName);
        toolkit.writeFormConfig(key+newName,oldInfo);    
    }
}

void addRegisterForm::addTtlvNumBit8Slot(bool flag)
{
    qDebug() << __FUNCTION__;
    Q_UNUSED(flag);
    QObject *obj = toolkit.findParent(this,nullptr,"MainWindow");
    Form *ttlvForm = (Form *)obj->findChild<Form *>(tr("添加功能"));
    QObject *currBox = toolkit.findParent(sender(),nullptr,"ttlvWidget");
    addTtlv *page;
    QMap<QString,QVariant> info;
    QLabel *label = currBox->findChild<QLabel *>();
    QString key;
    if(label)
    {
        info.insert("type","数值");
        info.insert("numType","8位有符号整形");
        info.insert("increment",0);
        info.insert("multiple",1);
        QString name = label->text();
        QTabWidget *tab1 = (QTabWidget *)toolkit.findParent(currBox,"QTabWidget",nullptr);
        QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab1,"QTabWidget",nullptr);
        PreQTableWidget *table = (PreQTableWidget *)toolkit.findParent(currBox,"PreQTableWidget",nullptr);
        if (tab1->tabText(tab1->currentIndex()) == tr("保持寄存器"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/保持寄存器/"+table->objectName()+"/"+name;
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("输入寄存器"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/输入寄存器/"+table->objectName()+"/"+name;
        }
    }
    QMap<QString,QVariant> peerInfo = toolkit.readFormConfig(key);
    QMap<QString,QVariant> attrInfo = peerInfo.value("attr").toMap();
    if(attrInfo.isEmpty())
    {
        return;
    }
    if(attrInfo.find("HALBytes").value() == "低8位")
    {
        info.insert("HALBytes","高8位");
    }
    else if(attrInfo.find("HALBytes").value() == "高8位")
    {
        info.insert("HALBytes","低8位");
    }
    page = new addTtlv(currBox,REG_ADD,"",info,BIT_SHOW);
    /* 把数据添加到功能列表 */
    connect(page,SIGNAL(addResultSignal(QString,QMap<QString,QVariant>)),ttlvForm,SLOT(addTtlvResultSlot(QString,QMap<QString,QVariant>)));
    connect(page,SIGNAL(addRegListSignal(QObject *,QString,QMap<QString,QVariant>,int,bool)),this,SLOT(addRegResultSlot(QObject *,QString,QMap<QString,QVariant>,int,bool)));
    page->show();
}


/*
    func:弹出添加功能页面
*/
void addRegisterForm::addTtlvSlot(bool flag)
{
    qDebug() << __FUNCTION__;
    Q_UNUSED(flag);
    QObject *obj = toolkit.findParent(this,nullptr,"MainWindow");
    Form *ttlvForm = (Form *)obj->findChild<Form *>(tr("添加功能"));
    QObject *currBox = toolkit.findParent(sender(),nullptr,"ttlvWidget");
    QTabWidget *currTab = (QTabWidget*)toolkit.findParent(currBox,"QTabWidget",nullptr);
    addTtlv *page;
    if(currTab->tabText(currTab->currentIndex()).indexOf(tr("寄存器")) >= 0)
    {
        page = new addTtlv(currBox,REG_ADD,"",QMap<QString,QVariant>(),BIT_SHOW);
    }
    else
    {
        page = new addTtlv(currBox,REG_ADD,"",QMap<QString,QVariant>(),BIT_HIDE);
    }

    if(((QWidget *)currBox)->layout()->count() > 16)
    {
        QMessageBox::information(0,tr("警告"),tr("单个寄存器中最多存放16个布尔值类型的功能标签"),tr("确认"));
        return;
    }
    /* 把数据添加到功能列表 */
    connect(page,SIGNAL(addResultSignal(QString,QMap<QString,QVariant>)),ttlvForm,SLOT(addTtlvResultSlot(QString,QMap<QString,QVariant>)));
    connect(page,SIGNAL(addRegListSignal(QObject *,QString,QMap<QString,QVariant>,int,bool)),this,SLOT(addRegResultSlot(QObject *,QString,QMap<QString,QVariant>,int,bool)));
//    connect(page,SIGNAL(modBitSignal(QObject*,QString,int)),this,SLOT(modBitResultSlot(QObject*,QString,int)));
    page->show();
}

/*
    func:创建新的功能标签
*/
void addRegisterForm::addRegResultSlot(QObject *obj,QString name,QMap<QString,QVariant> info,int bitAddr,bool isSave)
{
    if(nullptr == obj)
    {
        qDebug()<<"obj is null";
        return ;
    }
    QWidget *box = (QWidget *)obj;
    qDebug()<<"addRegisterForm"<<name<<info;
    qDebug()<<"从寄存器页面中添加功能"<<box;
    QTabWidget *tab = (QTabWidget *)toolkit.findParent(box,"QTabWidget",nullptr);
    QLabel *labelNew = new QLabel();
    labelNew->setText(name);
    labelNew->setAlignment(Qt::AlignCenter);
    qDebug()<<"tab"<<tab;
    qDebug()<<"labelNew"<<labelNew;
    if("布尔值" == info.find("type").value())
    {
        labelNew->setMinimumWidth(20);
        labelNew->setStyleSheet("background-color:rgb(69,137,148);");
        if(bitAddr >= 0)
        {
            labelNew->setToolTip(QString::number(bitAddr));
        }
    }
    else if("数值" == info.find("type").value())
    {
        qDebug()<<"数值numType"<<info.find("numType").value();
        if(info.find("numType").value().toString().indexOf("8位")!=-1)
        {
            labelNew->setMinimumWidth(20);
        }
        labelNew->setStyleSheet("background-color:rgb(117,121,74);");
    }
    else if("字节流" == info.find("type").value())
    {
        labelNew->setStyleSheet("background-color:rgb(114,83,52);");
    }
    else if("枚举" == info.find("type").value())
    {
        labelNew->setStyleSheet("background-color:rgb(205,133,63);");
    }

    QMap<QString,QVariant> eventInfo = info.value("event").toMap();

    QLabel *point = new QLabel();
    QHBoxLayout *pHBLayout = new QHBoxLayout();
    QWidget *stretch = new QWidget();
    stretch->setStyleSheet("background: transparent;");
    stretch->setMaximumHeight(0);
    pHBLayout->addWidget(stretch);
    pHBLayout->addWidget(point);
    pHBLayout->setMargin(0);
    labelNew->setLayout(pHBLayout);
    point->setFixedSize(20, 20);
    QPixmap *pixmap = new QPixmap(":img/event.png");
    pixmap->scaled(point->size(), Qt::KeepAspectRatio);
    point->setScaledContents(true);
    point->setPixmap(*pixmap);
    point->hide();

    if(eventInfo.count())
    {
        if (eventInfo.value("addEvent").toBool() == true)
        {
            point->show();
        }
        else
        {
            point->hide();
        }
    }

    PreQTableWidget *table = (PreQTableWidget *)toolkit.findParent(box,"PreQTableWidget",nullptr);
    table->indexAt(QPoint(0, 0));/* 不知道为什么要先这样调用一次，否则第一次调用会失败 */
    qDebug()<<"table"<<table<<table->indexAt(QPoint(0, 0));
    qDebug()<<"重新计算坐标"<<box->pos();
    int x = box->frameGeometry().x();
    int y = box->frameGeometry().y();
    QModelIndex index = table->indexAt(QPoint(x, y));
    int row = index.row();
    int column = index.column();
    int len = 1;
    qDebug()<<"重新计算坐标"<<x<<y<<row<<column<<table;
    if((info.find("type").value().toString() == "字节流" && info.find("len").value().toInt() > 2)
            || ((info.find("type").value().toString() == "数值" || info.find("type").value().toString() == "枚举") && info.find("numType").value().toString().indexOf("16位") == -1))
    {
        if(info.find("type").value().toString() == "字节流")
        {
            len = info.find("len").value().toInt();
            len = len/2 + len%2;
        }
        else if(info.find("type").value().toString() == "数值" || info.find("type").value().toString() == "枚举")
        {
            if(info.find("numType").value().toString().indexOf("32位") >= 0)
            {
                len = 2;
            }
            else if(info.find("numType").value().toString().indexOf("64位") >= 0)
            {
                len = 4;
            }
        }            
        /* 设置合并单元格 */

        if(info.find("numType").value().toString().indexOf("8位") >= 0)
        {
            qDebug()<<"8位控件"<<box->pos();
            QWidget *box_old =table->cellWidget(row,column);
            if(box_old && box_old->layout() && box_old->layout()->count() > 1)
            {
                QHBoxLayout *hboxLayout = qobject_cast<QHBoxLayout *>(box_old->layout());
                hboxLayout->addWidget(labelNew);
                QPushButton *button = box_old->findChild<QPushButton *>();
                if(button)
                {
                    hboxLayout->removeWidget(button);
                    button->setParent(nullptr);
                    button->deleteLater();
                }
            }
            else
            {
                /* 增加新控件 */
                QWidget *box_new = new QWidget();
                box_new->setObjectName("ttlvWidget");
                QHBoxLayout *listLayout = new QHBoxLayout(box_new);
                QString bitname = info.find("HALBytes").value().toString();
                listLayout->addWidget(labelNew);
                QPushButton * addTtvlButton = new QPushButton("+");
                connect(addTtvlButton,SIGNAL(clicked(bool)),this,SLOT(addTtlvNumBit8Slot(bool)));
                listLayout->addWidget(addTtvlButton);
                listLayout->setContentsMargins(0,0,0,0);
                box_new->setLayout(listLayout);
                table->setCellWidget(row,column,box_new);
            }
        }
        else
        {
            table->setSpan(row,column,len,1);
            /* 删除旧控件 */
            for(int i=0;i<len;i++)
            {
                table->removeCellWidget(row+i,column);
            }
            /* 增加新控件 */
            QWidget *box_new = new QWidget();
            box_new->setObjectName("ttlvWidget");
            QHBoxLayout *listLayout = new QHBoxLayout(box_new);
            listLayout->addWidget(labelNew);
            listLayout->setContentsMargins(0,0,0,0);
            box_new->setLayout(listLayout);
            table->setCellWidget(row,column,box_new);
            qDebug()<<"恢复到列表中"<<row<<column<<box_new;
        }
    }
    else
    {
        if("布尔值" != info.find("type").value().toString() || tab->tabText(tab->currentIndex()).indexOf(tr("线圈")) >= 0 ||tab->tabText(tab->currentIndex()).indexOf(tr("离散量")) >= 0)
        {
            qDebug()<<"清空列表中的控件"<<info.find("type").value().toString()<<tab->tabText(tab->currentIndex()).indexOf(tr("线圈"));
            toolkit.clearLayout(box->layout());
        }
        qDebug()<<"增加新的控件";
        box->layout()->addWidget(labelNew);
    }
    /* 保存到配置文件 */
    if (isSave)
    {
        if(initCompleted)
        {
            QTabWidget *tab1 = (QTabWidget *)toolkit.findParent(this,"QTabWidget",nullptr);
            QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab1,"QTabWidget",nullptr);

            QString path;
            if (tab1->tabText(tab1->currentIndex()) == tr("线圈"))
            {
                path = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/线圈/"+table->objectName()+"/"+name;
            }
            else if (tab1->tabText(tab1->currentIndex()) == tr("保持寄存器"))
            {
                path = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/保持寄存器/"+table->objectName()+"/"+name;
            }
            else if (tab1->tabText(tab1->currentIndex()) == tr("输入寄存器"))
            {
                path = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/输入寄存器/"+table->objectName()+"/"+name;
            }
            else if (tab1->tabText(tab1->currentIndex()) == tr("离散量"))
            {
                path = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/离散量/"+table->objectName()+"/"+name;
            }
    //        QString path = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/"+tab1->tabText(tab1->currentIndex())+"/"+table->objectName()+"/"+name;
            if (path != NULL)
            {
                toolkit.writeFormConfig(path+"/attr",info);
                toolkit.addPosInfo(path,bitAddr,row,column,len);
            }
        }
    }
}


/*
    func:在寄存器列表中修改布尔值比特地址
*/
void addRegisterForm::modBitResultSlot(QObject *obj,QString name,int oldBit,int newBit)
{
    QTabWidget *tab1 = (QTabWidget *)toolkit.findParent(this,"QTabWidget",nullptr);
    QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab1,"QTabWidget",nullptr);
    PreQTableWidget *table = (PreQTableWidget *)toolkit.findParent(obj,"PreQTableWidget",nullptr);

    QString path;
    if (tab1->tabText(tab1->currentIndex()) == tr("线圈"))
    {
        path = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/线圈/"+table->objectName()+"/"+name;
    }
    else if (tab1->tabText(tab1->currentIndex()) == tr("保持寄存器"))
    {
        path = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/保持寄存器/"+table->objectName()+"/"+name;
    }
    else if (tab1->tabText(tab1->currentIndex()) == tr("输入寄存器"))
    {
        path = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/输入寄存器/"+table->objectName()+"/"+name;
    }
    else if (tab1->tabText(tab1->currentIndex()) == tr("离散量"))
    {
        path = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/离散量/"+table->objectName()+"/"+name;
    }

//    QString path = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/"+tab1->tabText(tab1->currentIndex())+"/"+table->objectName()+"/"+name;
    QMap<QString,QVariant> ttlvInfo = toolkit.readFormConfig(path);
    QJsonArray posArray = ttlvInfo.value("pos").toJsonArray();
    for (int iPos = 0; iPos < posArray.count(); iPos++)
    {
        QJsonObject posObj = posArray.at(iPos).toObject();
        if(posObj.value("bit").toInt() == oldBit)
        {
            posObj.insert("bit",newBit);
            posArray.replace(iPos,posObj);
            break;
        }
    }
    ttlvInfo.insert("pos",posArray);
    toolkit.writeFormConfig(path,ttlvInfo);
    QList<QLabel *> labelList = obj->findChildren<QLabel *>();
    foreach (QLabel *lable, labelList)
    {
        if(lable->toolTip().toInt() == oldBit)
        {
            lable->setToolTip(QString::number(newBit));
            break;
        }
    }
    qDebug()<<"需要修改比特地址>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"<<oldBit<<newBit;
}

/*
    func:弹出右键菜单
*/
void addRegisterForm::tableContextMenuRequested(const QPoint &pos)
{
    if(childAt(pos) && QString(childAt(pos)->metaObject()->className()) == "QPushButton" && ((QPushButton *)childAt(pos))->text() != tr("添加寄存器地址"))
    {
        actionPos = pos;
        table_widget_menu->exec(QCursor::pos());
    }
}

/*
    func:右键菜单Reomve处理
*/
void addRegisterForm::slotActionRemove()
{
    QMessageBox box(QMessageBox::Warning,tr("警告"),tr("该操作将删除该列表下所有信息，请确认是否继续"));
    box.setStandardButtons (QMessageBox::Yes|QMessageBox::No);
    box.setButtonText (QMessageBox::Yes,QString(tr("确定")));
    box.setButtonText (QMessageBox::No,QString(tr("取消")));
    if(QMessageBox::No == box.exec())
    {
        return;
    }
    QPushButton *button = static_cast<QPushButton*>(childAt(actionPos));
    this->removeRegList(button);
}

/*
    func:根据列表头信息删除寄存器列表
*/
void addRegisterForm::removeRegList(QPushButton *button)
{

    QList<QPushButton *> buttonList = this->listToolBox->findChildren<QPushButton *>("pushButtonFold");
    for (int i = 0; i < buttonList.count(); i++)
    {
        qDebug()<<"寻找列表"<<buttonList.at(i)<<button;
        if(buttonList.at(i) == button)
        {
            qDebug()<<"找到对应的列表";
            /* 保存到配置文件 */
            QTabWidget *tab1 = (QTabWidget *)toolkit.findParent(this,"QTabWidget",nullptr);
            QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab1,"QTabWidget",nullptr);
            QWidget *box = (QWidget *)this->listToolBox->getWidget(i);
            PreQTableWidget *table = box->findChild<PreQTableWidget *>();
            qDebug()<<tab1->tabText(tab1->currentIndex());
            qDebug()<<tab2->tabText(tab2->currentIndex());
            QString key;
            if (tab1->tabText(tab1->currentIndex()) == tr("线圈"))
            {
                key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/线圈/"+table->objectName();
            }
            else if (tab1->tabText(tab1->currentIndex()) == tr("保持寄存器"))
            {
                key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/保持寄存器/"+table->objectName();
            }
            else if (tab1->tabText(tab1->currentIndex()) == tr("输入寄存器"))
            {
                key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/输入寄存器/"+table->objectName();
            }
            else if (tab1->tabText(tab1->currentIndex()) == tr("离散量"))
            {
                key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/离散量/"+table->objectName();
            }

//            QString key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/"+tab1->tabText(tab1->currentIndex())+"/"+table->objectName();
            qDebug()<<"key"<<key;
            QMap<QString,QVariant> removeInfo = toolkit.readFormConfig(key);
            QMap<QString,QVariant>::iterator iter = removeInfo.begin();
            while (iter != removeInfo.end())
            {
                qDebug()<<"删除功能标签："<<iter.key();
                toolkit.removeFormConfig("ttlv/" + iter.key());
                iter ++;
            }
            toolkit.removeFormConfig(key);
            /* 删除列表 */
            this->listToolBox->delWidget(i);
            break;
        }
    }
}

/*
    func:右键菜单Modify处理
*/
void addRegisterForm::slotActionModify()
{
    QPushButton *button = static_cast<QPushButton*>(childAt(actionPos));
    QList<QPushButton *> buttonList = this->listToolBox->findChildren<QPushButton *>("pushButtonFold");
    for (int i = 0; i < buttonList.count(); i++)
    {
        if(buttonList.at(i) == button)
        {
            QTabWidget *tab1 = (QTabWidget *)toolkit.findParent(this,"QTabWidget",nullptr);
            QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab1,"QTabWidget",nullptr);
            QWidget *box = (QWidget *)this->listToolBox->getWidget(i);
            PreQTableWidget *table = box->findChild<PreQTableWidget *>();

            QString key;
            if (tab1->tabText(tab1->currentIndex()) == tr("线圈"))
            {
                key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/线圈/"+table->objectName();
            }
            else if (tab1->tabText(tab1->currentIndex()) == tr("保持寄存器"))
            {
                key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/保持寄存器/"+table->objectName();
            }
            else if (tab1->tabText(tab1->currentIndex()) == tr("输入寄存器"))
            {
                key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/输入寄存器/"+table->objectName();
            }
            else if (tab1->tabText(tab1->currentIndex()) == tr("离散量"))
            {
                key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/离散量/"+table->objectName();
            }
//          key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/"+tab1->tabText(tab1->currentIndex())+"/"+table->objectName();
            QMap<QString,QVariant> info = toolkit.readFormConfig(key);
            qDebug()<<"找到对应的列表"<<table->objectName()<<info;
            addRegister *page = new addRegister(table->objectName(),info,table);
            connect(page,SIGNAL(modResultSignal(QString,QString,QMap<QString,QVariant>)),this,SLOT(modRegListResultSlot(QString,QString,QMap<QString,QVariant>)));
            page->show();
            break;
        }
    }
}


