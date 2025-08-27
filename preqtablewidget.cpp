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
#include "preqtablewidget.h"
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QMimeData>
#include <QDrag>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QAction>
#include <QMessageBox>
#include "add/addregisterform.h"
#include "add/addttlv.h"
#include "add/adduart.h"
#include "add/addproduct.h"
#include "Form.h"
#include <QJsonArray>
#include <QJsonObject>

PreQTableWidget::PreQTableWidget(QWidget *parent) :
    QTableWidget(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAcceptDrops(true);
    /* 右键菜单 */
    this->resizeRowsToContents();
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableContextMenuRequested(QPoint)));
    table_widget_menu = new QMenu(this);
    qDebug()<<"PreQTableWidget"<<parent;
    QAction *action_modify = new QAction(tr("修改"), this);
    connect(action_modify, SIGNAL(triggered()), this, SLOT(slotActionModify()));
    table_widget_menu->addAction(action_modify);
    QAction *action_remove = new QAction(tr("删除"), this);
    connect(action_remove, SIGNAL(triggered()), this, SLOT(slotActionRemove()));
    table_widget_menu->addAction(action_remove);

    table_row_menu = new QMenu(this);
    QAction *action_removeFront = new QAction(tr("删除至顶部"), this);
    connect(action_removeFront, SIGNAL(triggered()), this, SLOT(slotActionRemoveFront()));
    table_row_menu->addAction(action_removeFront);
    QAction *action_removeBack = new QAction(tr("删除至底部"), this);
    connect(action_removeBack, SIGNAL(triggered()), this, SLOT(slotActionRemoveBack()));
    table_row_menu->addAction(action_removeBack);

}

/*
    func:鼠标按下
*/
void PreQTableWidget::mousePressEvent(QMouseEvent *event)
{
    QObject *child = static_cast<QLabel*>(childAt(event->pos()));
    if(nullptr == child || "QLabel" != QString(child->metaObject()->className()))
    {
        return;
    }

    if (event->button() == Qt::RightButton)
    {
        currentLabel = child;
        return;
    }
    else if(tr("添加功能") != child->parent()->parent()->objectName())
    {
        return;
    }
    QLabel *label = (QLabel *)child;
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(label->text());
    mimeData->setImageData(this->ttlvList.find(label->text()).value());
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    mimeData->setData("style",label->styleSheet().toLatin1());
    drag->setHotSpot(event->pos()-label->pos());
    if(drag->exec(Qt::CopyAction|Qt::MoveAction,Qt::CopyAction) == Qt::MoveAction)
    {
        label->close();
    }
    else
    {
        label->show();
    }
}

/*
    func:鼠标进入
*/
void PreQTableWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    event->accept();
}

/*
    func:鼠标移动
*/
void PreQTableWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QObject *obj = childAt(event->pos());
    if(obj && "QPushButton" == QString(obj->metaObject()->className()) && tr("添加功能") != ((QPushButton *)obj)->text())
    {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

/*
    func:鼠标放下
*/
void PreQTableWidget::dropEvent(QDropEvent *event)
{
    qDebug()<<"mimeData"<<event->mimeData()->text()<<event->mimeData()->imageData().toMap();
    qDebug()<<"dropEvent"<<childAt(event->pos())->metaObject()->className();
    QObject *obj = childAt(event->pos());
    if(nullptr == obj || "QPushButton" != QString(obj->metaObject()->className()))
    {
        qDebug()<<"obj is null";
        event->ignore();
        return;
    }
    QWidget *box = (QWidget *)toolkit.findParent(obj,nullptr,"ttlvWidget");//这里可以优化的
    if(nullptr == box)
    {
        qDebug()<<"box is null";
        event->ignore();
        return;
    }
    QTabWidget *tab = (QTabWidget *)toolkit.findParent(this,"QTabWidget",nullptr);
    QMap<QString,QVariant> idValue = event->mimeData()->imageData().toMap();
    if(tab->tabText(tab->currentIndex()).indexOf(tr("线圈")) >= 0 && idValue.find("type").value().toString() != "布尔值")
    {
        QMessageBox::information(0,tr("警告"),tr("线圈仅支持布尔类型的功能标签"),tr("确认"));
        event->ignore();
        return;
    }
    if(tab->tabText(tab->currentIndex()).indexOf(tr("输入寄存器")) >= 0 && idValue.find("subType").value().toString() != "只读")
    {
        QMessageBox::information(0,tr("警告"),tr("输入寄存器仅支持只读类型的功能标签"),tr("确认"));
        event->ignore();
        return;
    }
    int x = box->frameGeometry().x();
    int y = box->frameGeometry().y();
    QModelIndex index = this->indexAt(QPoint(x, y));
    int row = index.row();
    int column = index.column();
    if(idValue.find("type").value().toString() == "布尔值")/* 数据功能类型为布尔值 */
    {
        if(tab->tabText(tab->currentIndex()).indexOf(tr("寄存器")) >= 0)/* 如果把布尔值类型功能拖动到寄存器界面，需要选择比特地址 */
        {
            if(box->layout()->count() > 16)
            {
                QMessageBox::information(0,tr("警告"),tr("单个寄存器中最多存放16个布尔值类型的功能标签"),tr("确认"));
                event->ignore();
                return;
            }
            QString title = event->mimeData()->text();
            addTtlv *page = new addTtlv(box,REG_DROP,title,idValue,BIT_SHOW);
            QObject *regForm = toolkit.findParent(this,"addRegisterForm",nullptr);
            connect(page,SIGNAL(addRegListSignal(QObject *,QString,QMap<QString,QVariant>,int,bool)),regForm,SLOT(addRegResultSlot(QObject *,QString,QMap<QString,QVariant>,int,bool)));
//            connect(page,SIGNAL(modBitSignal(QObject*,QString,int)),regForm,SLOT(modBitResultSlot(QObject*,QString,int)));
            page->show();
            return;
        }
    }
    else if(this->cellWidget(row,column)->layout()->count() > 1)
    {
        QMessageBox::information(0,tr("警告"),tr("该寄存器地址已添加布尔值功能，不可再添加其他类型的功能标签"),tr("确认"));
        event->ignore();
        return;
    }
    int len = 1;
    if((idValue.find("type").value().toString() == "字节流" && idValue.find("len").value().toInt() > 2)
            || ((idValue.find("type").value().toString() == "数值" || idValue.find("type").value().toString() == "枚举") && idValue.find("numType").value().toString().indexOf("16位") == -1))
    {
        if(idValue.find("type").value().toString() == "字节流")
        {
            len = idValue.find("len").value().toInt();
            len = len/2 + len%2;
        }
        else if(idValue.find("type").value().toString() == "数值" || idValue.find("type").value().toString() == "枚举")
        {
            if(idValue.find("numType").value().toString().indexOf("32位") >= 0)
            {
                len = 2;
            }
            else if(idValue.find("numType").value().toString().indexOf("64位") >= 0)
            {
                len = 4;
            }
        }
        /* 可用长度判断 */
        if(this->rowCount()-1 < len)
        {
            QMessageBox::information(0,tr("警告"),tr("该功能长度大于寄存器列表长度"),tr("确认"));
            return;
        }
        int usableLen = 0;
        for(usableLen=0;usableLen<this->rowCount()-row;usableLen++)
        {
            if(this->cellWidget(row+usableLen,column)->layout()->count() > 1)
            {
                break;
            }
            QLayoutItem *item = this->cellWidget(row+usableLen,column)->layout()->itemAt(0);
            QWidget *box_check = item->widget();
            if("QPushButton" !=  QString(box_check->metaObject()->className()))
            {
                break;
            }
        }
        qDebug()<<"usableLen"<<usableLen;
        if(usableLen < len)
        {
            QMessageBox::information(0,tr("警告"),tr("该功能长度大于当前寄存器可用长度"),tr("确认"));
            return;
        }
        /* 设置合并单元格 */
        this->setSpan(row,column,len,1);
        /* 删除旧控件 */
        for(int i=0;i<len;i++)
        {
            this->removeCellWidget(row+i,column);
        }
        /* 增加新控件 */
        QWidget *box_new = new QWidget();
        box_new->setObjectName("ttlvWidget");
        QLabel *label = new QLabel(event->mimeData()->text());
        label->setStyleSheet(event->mimeData()->data("style"));
        label->setAlignment(Qt::AlignCenter);
        QHBoxLayout *listLayout = new QHBoxLayout(box_new);
        listLayout->addWidget(label);
        listLayout->setContentsMargins(0,0,0,0);
        box_new->setLayout(listLayout);
        this->setCellWidget(row,column,box_new);
    }
    else
    {
        toolkit.clearLayout(box->layout());
        QLabel *label = new QLabel(event->mimeData()->text());
        label->setStyleSheet(event->mimeData()->data("style"));
        label->setAlignment(Qt::AlignCenter);
        box->layout()->addWidget(label);
    }
    event->setDropAction(Qt::CopyAction);
    event->accept();

    /* 保存到配置文件 */
    QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab,"QTabWidget",nullptr);
    PreQTableWidget *table = (PreQTableWidget *)toolkit.findParent(box,"PreQTableWidget",nullptr);
    qDebug()<<tab->tabText(tab->currentIndex());
    qDebug()<<tab2->tabText(tab2->currentIndex());
    QString key;
    if (tab->tabText(tab->currentIndex()) == tr("线圈"))
    {
        key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/线圈/"+table->objectName()+"/"+event->mimeData()->text();
    }
    else if (tab->tabText(tab->currentIndex()) == tr("保持寄存器"))
    {
        key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/保持寄存器/"+table->objectName()+"/"+event->mimeData()->text();
    }
    else if (tab->tabText(tab->currentIndex()) == tr("输入寄存器"))
    {
        key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/输入寄存器/"+table->objectName()+"/"+event->mimeData()->text();
    }
    else if (tab->tabText(tab->currentIndex()) == tr("离散量"))
    {
        key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/离散量/"+table->objectName()+"/"+event->mimeData()->text();
    }

//    QString key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/"+tab->tabText(tab->currentIndex())+"/"+table->objectName()+"/"+event->mimeData()->text();
    toolkit.writeFormConfig(key+"/attr",idValue);
    toolkit.addPosInfo(key,-1,row,column,len);
}

/*
    func:弹出右键菜单
*/
void PreQTableWidget::tableContextMenuRequested(const QPoint &pos)
{
    actionPos = pos;
    QWidget *box = childAt(pos);
    if(nullptr == box)
    {
        return;
    }
    if(QString(box->metaObject()->className()) == "QLabel")
    {
        table_widget_menu->exec(QCursor::pos());
    }
    else if(QString(box->metaObject()->className()) == "QWidget")
    {
        QModelIndex index = this->indexAt(pos);
        if(0 == index.column() && 0 != index.row() && this->objectName().indexOf(tr("添加")) < 0)
        {
            table_row_menu->exec(QCursor::pos());
        }
    }
}

/*
    func:右键菜单Reomve前面行
*/
void PreQTableWidget::slotActionRemoveFront()
{
    QMessageBox box(QMessageBox::Warning,tr("警告"),tr("该操作会删除已添加的功能标签，请确认是否继续"));
    box.setStandardButtons (QMessageBox::Yes|QMessageBox::No);
    box.setButtonText (QMessageBox::Yes,QString(tr("确定")));
    box.setButtonText (QMessageBox::No,QString(tr("取消")));
    if(QMessageBox::No == box.exec())
    {
        return;
    }
    QModelIndex index = this->indexAt(actionPos);
    QObject *page = toolkit.findParent(this,"ToolPage",nullptr);
    QPushButton *button = page->findChild<QPushButton *>();
    addRegisterForm *regForm = (addRegisterForm *)toolkit.findParent(this,"addRegisterForm",nullptr);
    int delRow = index.row();
    if(delRow == this->rowCount()-1)
    {
        QMessageBox box(QMessageBox::Warning,tr("警告"),tr("已选中当前列表中所有行，该操作将删除整个列表，请确认是否继续"));
        box.setStandardButtons (QMessageBox::Yes|QMessageBox::No);
        box.setButtonText (QMessageBox::Yes,QString(tr("确定")));
        box.setButtonText (QMessageBox::No,QString(tr("取消")));
        if(QMessageBox::No == box.exec())
        {
            return;
        }
        regForm->removeRegList(button);
        return;
    }
    QTabWidget *tab_func = (QTabWidget *)toolkit.findParent(this,"QTabWidget",nullptr);
    QTabWidget *tab_dev = (QTabWidget *)toolkit.findParent(tab_func,"QTabWidget",nullptr);
    QString path;
    if (tab_func->tabText(tab_func->currentIndex()) == tr("线圈"))
    {
        path = "dev/"+tab_dev->tabText(tab_dev->currentIndex())+"/register/线圈/";
    }
    else if (tab_func->tabText(tab_func->currentIndex()) == tr("保持寄存器"))
    {
        path = "dev/"+tab_dev->tabText(tab_dev->currentIndex())+"/register/保持寄存器/";
    }
    else if (tab_func->tabText(tab_func->currentIndex()) == tr("输入寄存器"))
    {
        path = "dev/"+tab_dev->tabText(tab_dev->currentIndex())+"/register/输入寄存器/";
    }
    else if (tab_func->tabText(tab_func->currentIndex()) == tr("离散量"))
    {
        path = "dev/"+tab_dev->tabText(tab_dev->currentIndex())+"/register/离散量/";
    }

//    QString path = "dev/"+tab_dev->tabText(tab_dev->currentIndex())+"/register/"+tab_func->tabText(tab_func->currentIndex())+"/";
    QMap<QString,QVariant> oldInfo = toolkit.readFormConfig(path+this->objectName());
    int row;
    QList<QString> labelCount;
    for(row=1;row<=delRow;row++)
    {
        int rowSpan = this->rowSpan(1,1);
        qDebug()<<"功能跨行检查"<<rowSpan<<row<<delRow;
        if(1 < rowSpan)
        {
            this->setSpan(1,1,1,1);
            this->removeCellWidget(1,1);
            for (int i = 0; i < rowSpan; i++)
            {
                QPushButton *pushButton = new QPushButton("+");
                connect(pushButton,SIGNAL(clicked(bool)),regForm,SLOT(addTtlvSlot(bool)));
                QWidget *box_new = new QWidget();
                box_new->setObjectName("ttlvWidget");
                QHBoxLayout *listLayout = new QHBoxLayout(box_new);
                listLayout->addWidget(pushButton);
                listLayout->setContentsMargins(0,0,0,0);
                box_new->setLayout(listLayout);
                this->setCellWidget(1+i,1,box_new);
            }
        }
        this->removeRow(1);
        /* 根据行数遍历所有物模型 */
        QMutableMapIterator<QString, QVariant> oldInfoIt(oldInfo);//读写迭代器
        while (oldInfoIt.hasNext())
        {
            oldInfoIt.next();
            QMap<QString,QVariant> ttlvInfo = oldInfoIt.value().toMap();
            QJsonArray posArray = ttlvInfo.value("pos").toJsonArray();
            for (int iPos = 0; iPos < posArray.count(); iPos++)
            {
                QJsonObject posObj = posArray.at(iPos).toObject();
                if(posObj.value("row").toInt() == row)
                {
                    labelCount.append(oldInfoIt.key());
                    posArray.removeAt(iPos);
                    iPos--;
                    qDebug()<<"删除行"<<iPos;
                }
            }
            if(posArray.isEmpty())
            {
                oldInfoIt.remove();
            }
            else
            {
                ttlvInfo.insert("pos",posArray);
                oldInfoIt.setValue(ttlvInfo);
            }
        }
    }
    toolkit.removeFormConfig(path+this->objectName());
    QString newName = this->item(1,0)->text()+"-"+this->item(this->rowCount()-1,0)->text();
    qDebug()<<this->objectName()<<newName<<oldInfo;
    QMutableMapIterator<QString, QVariant> oldInfoIt(oldInfo);//读写迭代器
    while (oldInfoIt.hasNext())
    {
        oldInfoIt.next();
        QMap<QString,QVariant> ttlvInfo = oldInfoIt.value().toMap();
        QJsonArray posArray = ttlvInfo.value("pos").toJsonArray();
        for (int iPos = 0; iPos < posArray.count(); iPos++)
        {
            QJsonObject posObj = posArray.at(iPos).toObject();
            int tmpRow = posObj.value("row").toInt()-delRow;
            posObj.insert("row",tmpRow);
            posArray.replace(iPos,posObj);
        }
        ttlvInfo.insert("pos",posArray);
        oldInfoIt.setValue(ttlvInfo);
    }
    this->setObjectName(newName);
    button->setText(newName);
    toolkit.writeFormConfig(path+newName,oldInfo);
    QMap<QString,QVariant> oldTtlv = toolkit.readFormConfig("ttlv");
    QMap<QString,QVariant>::iterator ttlvIter = oldTtlv.begin();
    while (ttlvIter != oldTtlv.end())
    {
        if (labelCount.contains(ttlvIter.key()) == true)
        {
            toolkit.removeFormConfig("ttlv/" + ttlvIter.key());
        }
        ttlvIter ++;
    }
}

/*
    func:右键菜单Reomve后面行
*/
void PreQTableWidget::slotActionRemoveBack()
{
    QMessageBox box(QMessageBox::Warning,tr("警告"),tr("该操作会删除已添加的功能标签，请确认是否继续"));
    box.setStandardButtons (QMessageBox::Yes|QMessageBox::No);
    box.setButtonText (QMessageBox::Yes,QString(tr("确定")));
    box.setButtonText (QMessageBox::No,QString(tr("取消")));
    if(QMessageBox::No == box.exec())
    {
        return;
    }
    QModelIndex index = this->indexAt(actionPos);
    QObject *page = toolkit.findParent(this,"ToolPage",nullptr);
    QPushButton *button = page->findChild<QPushButton *>();
    addRegisterForm *regForm = (addRegisterForm *)toolkit.findParent(this,"addRegisterForm",nullptr);
    int delRow = index.row();
    if(delRow == 1)
    {
        QMessageBox box(QMessageBox::Warning,tr("警告"),tr("已选中当前列表中所有行，该操作将删除整个列表，请确认是否继续"));
        box.setStandardButtons (QMessageBox::Yes|QMessageBox::No);
        box.setButtonText (QMessageBox::Yes,QString(tr("确定")));
        box.setButtonText (QMessageBox::No,QString(tr("取消")));
        if(QMessageBox::No == box.exec())
        {
            return;
        }
        regForm->removeRegList(button);
        return;
    }
    QTabWidget *tab_func = (QTabWidget *)toolkit.findParent(this,"QTabWidget",nullptr);
    QTabWidget *tab_dev = (QTabWidget *)toolkit.findParent(tab_func,"QTabWidget",nullptr);

    QString path;
    if (tab_func->tabText(tab_func->currentIndex()) == tr("线圈"))
    {
        path = "dev/"+tab_dev->tabText(tab_dev->currentIndex())+"/register/线圈/";
    }
    else if (tab_func->tabText(tab_func->currentIndex()) == tr("保持寄存器"))
    {
        path = "dev/"+tab_dev->tabText(tab_dev->currentIndex())+"/register/保持寄存器/";
    }
    else if (tab_func->tabText(tab_func->currentIndex()) == tr("输入寄存器"))
    {
        path = "dev/"+tab_dev->tabText(tab_dev->currentIndex())+"/register/输入寄存器/";
    }
    else if (tab_func->tabText(tab_func->currentIndex()) == tr("离散量"))
    {
        path = "dev/"+tab_dev->tabText(tab_dev->currentIndex())+"/register/离散量/";
    }

//    QString path = "dev/"+tab_dev->tabText(tab_dev->currentIndex())+"/register/"+tab_func->tabText(tab_func->currentIndex())+"/";
    QMap<QString,QVariant> oldInfo = toolkit.readFormConfig(path+this->objectName());
    int row;
    QList<QString> labelCount;
    for(row=this->rowCount()-1;row>=delRow;row--)
    {
        int rowSpan = this->rowSpan(row,1);
        qDebug()<<"功能跨行检查"<<rowSpan<<row<<delRow;
        if(1 < rowSpan)
        {
            int startRow = row-rowSpan+1;
            this->setSpan(startRow,1,1,1);
            this->removeCellWidget(startRow,1);
            for (int i = 0; i < rowSpan; i++)
            {
                QPushButton *pushButton = new QPushButton("+");
                connect(pushButton,SIGNAL(clicked(bool)),regForm,SLOT(addTtlvSlot(bool)));
                QWidget *box_new = new QWidget();
                box_new->setObjectName("ttlvWidget");
                QHBoxLayout *listLayout = new QHBoxLayout(box_new);
                listLayout->addWidget(pushButton);
                listLayout->setContentsMargins(0,0,0,0);
                box_new->setLayout(listLayout);
                this->setCellWidget(startRow+i,1,box_new);
            }
        }
        this->removeRow(row);
        /* 根据行数遍历所有物模型 */
        QMutableMapIterator<QString, QVariant> oldInfoIt(oldInfo);//读写迭代器
        while (oldInfoIt.hasNext())
        {
            oldInfoIt.next();
            QMap<QString,QVariant> ttlvInfo = oldInfoIt.value().toMap();
            QJsonArray posArray = ttlvInfo.value("pos").toJsonArray();
            for (int iPos = 0; iPos < posArray.count(); iPos++)
            {
                QJsonObject posObj = posArray.at(iPos).toObject();
                if(posObj.value("row").toInt() == row)
                {
                    labelCount.append(oldInfoIt.key());
                    posArray.removeAt(iPos);
                    iPos--;
                    qDebug()<<"删除行"<<iPos;
                }
            }
            if(posArray.isEmpty())
            {
                oldInfoIt.remove();
            }
            else
            {
                ttlvInfo.insert("pos",posArray);
                oldInfoIt.setValue(ttlvInfo);
            }
        }
    }

    toolkit.removeFormConfig(path+this->objectName());
    QString newName = this->item(1,0)->text()+"-"+this->item(this->rowCount()-1,0)->text();
    qDebug()<<this->objectName()<<newName;
    this->setObjectName(newName);
    button->setText(newName);
    toolkit.writeFormConfig(path+newName,oldInfo);
    QMap<QString,QVariant> oldTtlv = toolkit.readFormConfig("ttlv");
    QMap<QString,QVariant>::iterator ttlvIter = oldTtlv.begin();
    while (ttlvIter != oldTtlv.end())
    {
        if (labelCount.contains(ttlvIter.key()) == true)
        {
            toolkit.removeFormConfig("ttlv/" + ttlvIter.key());
        }
        ttlvIter ++;
    }
}

/*
    func:右键菜单Modify处理
*/
void PreQTableWidget::slotActionModify()
{
    qDebug() << __FUNCTION__;
    QModelIndex index = this->indexAt(actionPos);
    int row = index.row();
    int column = index.column();
    QWidget *box = this->cellWidget(row,column);
    qDebug()<<row<<column<<box<<box->metaObject()->className();
    if(box && "QLabel" == QString(box->metaObject()->className()))
    {
        if(this->objectName() == tr("添加产品"))
        {
            QString name = ((QLabel *)box)->text();
            addProduct *page = new addProduct(name,this->productList.find(name).value().toMap());
            connect(page,SIGNAL(modResultSignal(QString,QString,QMap<QString,QVariant>)),this->parent(),SLOT(modProductResultSlot(QString,QString,QMap<QString,QVariant>)));
            page->show();
        }
        else if(this->objectName() == tr("添加串口"))
        {
            QString name = ((QLabel *)box)->text();
            addUart *page = new addUart(this,name,this->uartList.find(name).value().toMap());
            connect(page,SIGNAL(modResultSignal(QString,QString,QMap<QString,QVariant>)),this->parent(),SLOT(modUartResultSlot(QString,QString,QMap<QString,QVariant>)));
            page->show();
        }
        else if(this->objectName() == tr("添加功能"))
        {
            QString name = ((QLabel *)box)->text();
            addTtlv *page = new addTtlv(this,LIST_MOD,name,this->ttlvList.find(name).value().toMap(),BIT_HIDE);
            connect(page,SIGNAL(modResultSignal(QString,QString,QMap<QString,QVariant>)),this->parent(),SLOT(modTtlvResultSlot(QString,QString,QMap<QString,QVariant>)));
            page->show();
        }
    }
    else if(box && "ttlvWidget" == box->objectName())
    {
        QString name = ((QLabel *)currentLabel)->text();
        QTabWidget *tab1 = (QTabWidget *)toolkit.findParent(box,"QTabWidget",nullptr);
        QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab1,"QTabWidget",nullptr);
        PreQTableWidget *regTable = (PreQTableWidget *)toolkit.findParent(box,"PreQTableWidget",nullptr);
        addRegisterForm *regForm = (addRegisterForm *)toolkit.findParent(regTable,"addRegisterForm",nullptr);
        qDebug()<<tab1->tabText(tab1->currentIndex());
        qDebug()<<tab2->tabText(tab2->currentIndex());

        QString key;
        if (tab1->tabText(tab1->currentIndex()) == tr("线圈"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/线圈/"+regTable->objectName()+"/"+name+"/attr";
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("保持寄存器"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/保持寄存器/"+regTable->objectName()+"/"+name+"/attr";
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("输入寄存器"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/输入寄存器/"+regTable->objectName()+"/"+name+"/attr";
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("离散量"))
        {
            key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/离散量/"+regTable->objectName()+"/"+name+"/attr";
        }

//        QString key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/"+tab1->tabText(tab1->currentIndex())+"/"+regTable->objectName()+"/"+name+"/attr";
        QMap<QString,QVariant> info = toolkit.readFormConfig(key);
        qDebug()<<"寄存器修改"<<name<<key<<info;
        QObject *obj = toolkit.findParent(this,nullptr,"MainWindow");
        Form *ttlvForm = obj->findChild<Form*>(tr("添加功能"));
        addTtlv *page;
        if(tab1->tabText(tab1->currentIndex()).indexOf(tr("寄存器")) >= 0)
        {
            bool ok;
            int bit = ((QLabel *)currentLabel)->toolTip().toInt(&ok);
            if(ok)
            {
                page = new addTtlv(box,REG_MOD,name,info,bit);
            }
            else
            {
                page = new addTtlv(box,REG_MOD,name,info,BIT_SHOW);
            }
        }
        else
        {
            page = new addTtlv(box,REG_MOD,name,info,BIT_HIDE);
        }
        qDebug()<<">>>>>>>>"<<regTable<<regForm;
        connect(page,SIGNAL(modResultSignal(QString,QString,QMap<QString,QVariant>)),ttlvForm,SLOT(modTtlvResultSlot(QString,QString,QMap<QString,QVariant>)));
        connect(page,SIGNAL(modBitSignal(QObject*,QString,int,int)),regForm,SLOT(modBitResultSlot(QObject*,QString,int,int)));
        page->show();
    }
}

/*
    func:右键菜单Reomve处理
*/
void PreQTableWidget::slotActionRemove()
{
    QModelIndex index = this->indexAt(actionPos);
    int row = index.row();
    int column = index.column();
    QWidget *box = this->cellWidget(row,column);
    if(box && "QWidget" == QString(box->metaObject()->className()))/* 删除寄存器列表中的功能 */
    {
        QString name = ((QLabel *)currentLabel)->text();
        int bit = ((QLabel *)currentLabel)->toolTip().toInt();
        QString key;
        for (int i = 0; i < box->layout()->count(); i++)
        {
            QLayoutItem *item = box->layout()->itemAt(i);
            if(currentLabel == item->widget())
            {
                /* 保存到配置文件 */
                qDebug()<<" remove 11111111111 ";
                QTabWidget *tab1 = (QTabWidget *)toolkit.findParent(box,"QTabWidget",nullptr);
                QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab1,"QTabWidget",nullptr);
                PreQTableWidget *table = (PreQTableWidget *)toolkit.findParent(box,"PreQTableWidget",nullptr);
                qDebug()<<tab1->tabText(tab1->currentIndex());
                qDebug()<<tab2->tabText(tab2->currentIndex());
                if (tab1->tabText(tab1->currentIndex()) == tr("线圈"))
                {
                    key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/线圈/"+table->objectName()+"/"+name;
                }
                else if (tab1->tabText(tab1->currentIndex()) == tr("保持寄存器"))
                {
                    key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/保持寄存器/"+table->objectName()+"/"+name;
                }
                else if (tab1->tabText(tab1->currentIndex()) == tr("输入寄存器"))
                {
                    key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/输入寄存器/"+table->objectName()+"/"+name;
                }
                else if (tab1->tabText(tab1->currentIndex()) == tr("离散量"))
                {
                    key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/离散量/"+table->objectName()+"/"+name;
                }

//                QString key = "dev/"+tab2->tabText(tab2->currentIndex())+"/register/"+tab1->tabText(tab1->currentIndex())+"/"+table->objectName()+"/"+name;
                QMap<QString,QVariant> ttlvInfo = toolkit.readFormConfig(key);
                QJsonArray posArray = ttlvInfo.value("pos").toJsonArray();
                for (int iPos = 0; iPos < posArray.count(); iPos++)
                {
                    if(posArray.at(iPos).toObject().value("row").toInt() == row)
                    {
                        if(posArray.at(iPos).toObject().value("bit").toInt() < 0 || posArray.at(iPos).toObject().value("bit").toInt() == bit)
                        {
                            posArray.removeAt(iPos);
                            break;
                        }
                    }
                }
                if(posArray.count())
                {
                    ttlvInfo.insert("pos",posArray);
                    toolkit.writeFormConfig(key,ttlvInfo);
                }
                else
                {
                    qDebug()<<" remove key "<<key;
                    toolkit.removeFormConfig(key);
                    QString removeTtlv = "ttlv/" + name;
                    qDebug()<<" remove removeTtlv "<<removeTtlv;
                    toolkit.removeFormConfig(removeTtlv);
                }
                /* 删除控件 */
                item = box->layout()->takeAt(i);
                delete item->widget();
                break;
            }
        }
        int rowSpan = this->rowSpan(row,column);
        if(1 == rowSpan)
        {
            QObject *obj = toolkit.findParent(this,"addRegisterForm",nullptr);
            if(1 == box->layout()->count())
            {
                QPushButton *pushButton = nullptr;
                pushButton = box->findChild<QPushButton *>();
                if(pushButton == nullptr)
                {
                    pushButton = new QPushButton("+");
                    connect(pushButton,SIGNAL(clicked(bool)),obj,SLOT(addTtlvNumBit8Slot(bool)));
                    box->layout()->addWidget(pushButton);
                }
                else
                {
                  disconnect(pushButton, SIGNAL(clicked(bool)), nullptr, nullptr);
                  connect(pushButton,SIGNAL(clicked(bool)),obj,SLOT(addTtlvSlot(bool)));
                }
            }
            else if(0 == box->layout()->count())
            {
                QPushButton *pushButton = new QPushButton("+");
                connect(pushButton,SIGNAL(clicked(bool)),obj,SLOT(addTtlvSlot(bool)));
                box->layout()->addWidget(pushButton);
            }
        }
        else
        {
            this->setSpan(row,column,1,1);
            this->removeCellWidget(row,column);
            for (int i = 0; i < rowSpan; i++)
            {
                QPushButton *pushButton = new QPushButton("+");
                QObject *obj = toolkit.findParent(this,"addRegisterForm",nullptr);
                connect(pushButton,SIGNAL(clicked(bool)),obj,SLOT(addTtlvSlot(bool)));
                QWidget *box_new = new QWidget();
                box_new->setObjectName("ttlvWidget");
                QHBoxLayout *listLayout = new QHBoxLayout(box_new);
                listLayout->addWidget(pushButton);
                listLayout->setContentsMargins(0,0,0,0);
                box_new->setLayout(listLayout);
                this->setCellWidget(row+i,1,box_new);
            }
        }
    }
    else if(box && "QLabel" == QString(box->metaObject()->className()))/* 删除功能列表中的功能 */
    {
        QString name = ((QLabel *)box)->text();
        if(this->objectName() == tr("添加功能"))
        {
            QMessageBox box(QMessageBox::Warning,tr("警告"),tr("该操作将删除寄存器列表已添加的功能，请确认是否继续"));
            box.setStandardButtons (QMessageBox::Yes|QMessageBox::No);
            box.setButtonText (QMessageBox::Yes,QString(tr("确定")));
            box.setButtonText (QMessageBox::No,QString(tr("取消")));
            if(QMessageBox::No == box.exec())
            {
                return;
            }
            this->ttlvList.remove(name);
            /* 删除功能的时候，遍历所有寄存器列表，把列表中的相同功能全部删掉 */
            removeRegisterTtlv(name);
            toolkit.removeFormConfig("ttlv/"+name);
        }
        else if(this->objectName() == tr("添加串口"))
        {
            /* 删除串口的时候，遍历所有设备列表 */
            /* 如果该串口已添加设备，则不允许删除该串口 */
            if(findDevForUart(name))
            {
                QMessageBox::information(0,tr("警告"),tr("该串口已经被添加到设备列表中，请先把对应设备删除再删除串口"),tr("确认"));
                return;
            }
            this->delRegisterUart(name);
            this->uartList.remove(name);
            toolkit.removeFormConfig("uart/"+name);
        }
        else if(this->objectName() == tr("添加产品"))
        {
            this->productList.remove(name);
            toolkit.removeFormConfig("product/"+name);
            QObject* obj = toolkit.findParent(this,"Form",nullptr);
            QPushButton *button = obj->findChild<QPushButton *>();
            button->setEnabled(true);

        }
        this->removeRow(row);
    }
}

/*
    func:判断是否串口已被使用
*/
bool PreQTableWidget::findDevForUart(QString name)
{
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev");
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end())
    {
        QMap<QString,QVariant> portInfo = iter.value().toMap().value("addr").toMap();
        qDebug()<<"findDevForUart"<<portInfo<<portInfo.value(name).toJsonArray();
        if(portInfo.value(name).toJsonArray().count())
        {
            return true;
        }
        iter++;
    }
    return false;
}

/*
    func:遍历所有设备列表删除串口
*/
void PreQTableWidget::delRegisterUart(QString oldName)
{
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev");
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end())
    {
        qDebug()<<"delRegisterUart"<<oldName<<"dev/"+iter.key()+"/addr/"+oldName;
        toolkit.removeFormConfig("dev/"+iter.key()+"/addr/"+oldName);
        iter++;
    }
}

/*
    func:遍历所有设备列表修改串口
*/
void PreQTableWidget::modRegisterUart(QString oldName,QString newName)
{
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev");
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end())
    {
        QMap<QString,QVariant> oldInfo = info.value(iter.key()).toMap().value("addr").toMap();
        qDebug()<<"delRegisterUart"<<oldName<<newName<<oldInfo;
        QJsonArray slaveInfo = oldInfo.value(oldName).toJsonArray();
        oldInfo.remove(oldName);
        oldInfo.insert(newName,slaveInfo);
        toolkit.writeFormConfig("dev/"+iter.key()+"/addr/",oldInfo);
        iter++;
    }
}

/*
    func:遍历所有寄存器列表删除功能
*/
void PreQTableWidget::removeRegisterTtlv(QString name)
{
    qDebug()<<"removeRegisterTtlv"<<this;
    QObject *obj = toolkit.findParent(this,nullptr,"MainWindow");
    QTabWidget *devTab = obj->findChild<QTabWidget*>();
    qDebug()<<devTab<<devTab->count();
    for (int devIndex = 0; devIndex < devTab->count()-1; devIndex++)
    {
        QWidget *currDevTab = devTab->widget(devIndex);
        qDebug()<<"currTab"<<currDevTab<<devTab->tabText(devIndex);
        QTabWidget *regTab = currDevTab->findChild<QTabWidget*>();
        for (int regIndex = 0; regIndex < regTab->count(); regIndex++)
        {
            QWidget *currRegTab = regTab->widget(regIndex);
            qDebug()<<"currTab"<<currRegTab<<regTab->tabText(regIndex);
            QList<PreQTableWidget*> tableList = currRegTab->findChildren<PreQTableWidget*>();
            qDebug()<<tableList;
            foreach (PreQTableWidget *table, tableList)
            {
                qDebug()<<table<<table->rowCount();
                for (int row = 1; row < table->rowCount(); row += table->rowSpan(row,1))
                {
                    QWidget *box = table->cellWidget(row,1);
                    bool deleteFlag = false;
                    qDebug()<<row<<box<<table->rowSpan(row,1)<<table->rowCount();
                    for (int iLayout = 0; iLayout < box->layout()->count(); iLayout++)
                    {
                        QWidget *boxLayout = box->layout()->itemAt(iLayout)->widget();
                        if(QString(boxLayout->metaObject()->className()) == "QLabel")
                        {
                            QLabel *label = (QLabel *)boxLayout;
                            if(label->text() == name)
                            {
                                qDebug()<<"删除"<<name;
                                /* 保存到配置文件 */
                                PreQTableWidget *table = (PreQTableWidget *)toolkit.findParent(box,"PreQTableWidget",nullptr);

                                QString key;
                                if (regTab->tabText(regIndex) == tr("线圈"))
                                {
                                    key = "dev/"+devTab->tabText(devIndex)+"/register/线圈/"+table->objectName()+"/"+name;
                                }
                                else if (regTab->tabText(regIndex) == tr("保持寄存器"))
                                {
                                    key = "dev/"+devTab->tabText(devIndex)+"/register/保持寄存器/"+table->objectName()+"/"+name;
                                }
                                else if (regTab->tabText(regIndex) == tr("输入寄存器"))
                                {
                                    key = "dev/"+devTab->tabText(devIndex)+"/register/输入寄存器/"+table->objectName()+"/"+name;
                                }
                                else if (regTab->tabText(regIndex) == tr("离散量"))
                                {
                                    key = "dev/"+devTab->tabText(devIndex)+"/register/离散量/"+table->objectName()+"/"+name;
                                }

//                                QString key = "dev/"+devTab->tabText(devIndex)+"/register/"+regTab->tabText(regIndex)+"/"+table->objectName()+"/"+name;
                                toolkit.removeFormConfig(key);
                                /*删除控件*/
                                QLayoutItem *item = box->layout()->takeAt(iLayout);
                                iLayout--;
                                delete item->widget();
                                deleteFlag = true;
                            }
                        }
                    }
                    if(deleteFlag)
                    {
                        int rowSpan = table->rowSpan(row,1);
                        qDebug()<<"rowSpan"<<rowSpan;
                        if(1 == rowSpan)
                        {
                            QPushButton *pushButton = new QPushButton("+");
                            QObject *objReg = obj->findChild<addRegisterForm*>();;
                            connect(pushButton,SIGNAL(clicked(bool)),objReg,SLOT(addTtlvSlot(bool)));
                            qDebug()<<"removeRegisterTtlv"<<objReg;
                            if(0 == box->layout()->count())
                            {
                                box->layout()->addWidget(pushButton);
                            }
                        }
                        else
                        {
                            table->setSpan(row,1,1,1);
                            table->removeCellWidget(row,1);
                            for (int i = 0; i < rowSpan; i++)
                            {
                                QPushButton *pushButton = new QPushButton("+");
                                QObject *objReg = obj->findChild<addRegisterForm*>();;
                                connect(pushButton,SIGNAL(clicked(bool)),objReg,SLOT(addTtlvSlot(bool)));
                                QWidget *box_new = new QWidget();
                                box_new->setObjectName("ttlvWidget");
                                QHBoxLayout *listLayout = new QHBoxLayout(box_new);
                                listLayout->addWidget(pushButton);
                                listLayout->setContentsMargins(0,0,0,0);
                                box_new->setLayout(listLayout);
                                table->setCellWidget(row+i,1,box_new);
                            }
                        }
                    }
                }
            }
        }
    }
}

/*
    func:遍历所有寄存器列表修改功能标签
*/
void PreQTableWidget::modRegisterTtlv(QString oldName,QString newName,QMap<QString,QVariant> info)
{
    qDebug()<<"modRegisterTtlv"<<this;
    QObject *obj = toolkit.findParent(this,nullptr,"MainWindow");
    QTabWidget *devTab = obj->findChild<QTabWidget*>();
    qDebug()<<devTab<<devTab->count();
    for (int devIndex = 0; devIndex < devTab->count()-1; devIndex++)
    {
        QWidget *currDevTab = devTab->widget(devIndex);
        qDebug()<<"currTab"<<currDevTab<<devTab->tabText(devIndex);
        QTabWidget *regTab = currDevTab->findChild<QTabWidget*>();
        for (int regIndex = 0; regIndex < regTab->count(); regIndex++)
        {
            QWidget *currRegTab = regTab->widget(regIndex);
            qDebug()<<"currTab"<<currRegTab<<regTab->tabText(regIndex);
            QList<PreQTableWidget*> tableList = currRegTab->findChildren<PreQTableWidget*>();
//            qDebug()<<tableList;
            foreach (PreQTableWidget *table, tableList)
            {
//                qDebug()<<table<<table->rowCount();
                for (int row = 1; row < table->rowCount(); row += table->rowSpan(row,1))
                {
                    QWidget *box = table->cellWidget(row,1);
//                    qDebug()<<row<<box<<table->rowSpan(row,1)<<table->rowCount();
                    for (int iLayout = 0; iLayout < box->layout()->count(); iLayout++)
                    {
                        QWidget *boxLayout = box->layout()->itemAt(iLayout)->widget();
                        if(QString(boxLayout->metaObject()->className()) == "QLabel")
                        {
                            QLabel *label = (QLabel *)boxLayout;
                            if(label->text() == oldName)
                            {
                                label->setText(newName);
                                /* 保存到配置文件 */
                                PreQTableWidget *table = (PreQTableWidget *)toolkit.findParent(box,"PreQTableWidget",nullptr);
                                QString path;
                                if (regTab->tabText(regIndex) == tr("线圈"))
                                {
                                    path = "dev/"+devTab->tabText(devIndex)+"/register/线圈/"+table->objectName()+"/";
                                }
                                else if (regTab->tabText(regIndex) == tr("保持寄存器"))
                                {
                                    path = "dev/"+devTab->tabText(devIndex)+"/register/保持寄存器/"+table->objectName()+"/";
                                }
                                else if (regTab->tabText(regIndex) == tr("输入寄存器"))
                                {
                                    path = "dev/"+devTab->tabText(devIndex)+"/register/输入寄存器/"+table->objectName()+"/";
                                }
                                else if (regTab->tabText(regIndex) == tr("离散量"))
                                {
                                    path = "dev/"+devTab->tabText(devIndex)+"/register/离散量/"+table->objectName()+"/";
                                }

                                QMap<QString,QVariant> eventInfo = info.value("event").toMap();
                                if(eventInfo.count())
                                {
                                    QList<QLabel *> point = label->findChildren<QLabel *>();
                                    if (eventInfo.value("addEvent").toBool() == true)
                                    {
                                        if (point.count())
                                        {
                                            point[0]->show();
                                        }
                                    }
                                    else
                                    {
                                        if (point.count())
                                        {
                                            point[0]->hide();
                                        }
                                    }
                                }

//                              QString path = "dev/"+devTab->tabText(devIndex)+"/register/"+regTab->tabText(regIndex)+"/"+table->objectName()+"/";
                                QMap<QString,QVariant> oldInfo = toolkit.readFormConfig(path+oldName);
                                if(!oldInfo.isEmpty())
                                {
                                    qDebug()<<"修改前配置信息"<<newName<<oldInfo;
                                    oldInfo.insert("attr",info);
                                    toolkit.removeFormConfig(path+oldName);
                                    qDebug()<<"修改后配置信息"<<newName<<oldInfo;
                                    toolkit.writeFormConfig(path+newName,oldInfo);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

