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
#include "addttlv.h"
#include "ui_addttlv.h"
#include "add/addttlvbool.h"
#include "add/addttlvbyte.h"
#include "add/addttlvnum.h"
#include "add/addttlvenum.h"
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QSpinBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QRadioButton>
#include <QScrollArea>
#include "preqtablewidget.h"

addTtlv::addTtlv(QObject *obj,int type,QString name,QMap<QString,QVariant> info,int bitInfo,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addTtlv)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() |Qt::Dialog);
    this->setWindowModality(Qt::ApplicationModal); //阻塞除当前窗体之外的所有的窗体
    /* 禁用 最大化按钮 */
    this->setWindowFlags(this->windowFlags()&~Qt::WindowMaximizeButtonHint);
    /* 回车键确认 */
    ui->buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::Key_Return);
    /* 限制由数字、26个英文字母或者下划线组成的字符串 */
    QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5][a-zA-Z0-9_\/\.\u4e00-\u9fa5]+");
    ui->lineEdit_name->setValidator(new QRegExpValidator(rx,this));
    ui->lineEdit_name->setToolTip(tr("支持中文、大小写字母、\n数字、短划线、下划线、\n斜杠和小数点，必须以中文、\n英文或数字开头，不超过64个字符"));

    /*
     * 描述                          obj          tab      bit      info          type
    1)在列表添加时隐藏                 tableWidget  null     隐藏     infoIsEmtry   type可选
    2)在线圈列表中添加时隐藏            ttlvWidget   线圈      隐藏    infoIsEmtry   type不可选
    3)在寄存器列表中添加时显示          ttlvWidget   寄存器     显示    infoIsEmtry   type可选
    4)拖放寄存器界面添加时显示          ttlvWidget   寄存器     显示    infoHasValue  type不可选
    5)在寄存器列表中添加多个功能时显示  ttlvWidget   寄存器     显示   infoIsEmtry    type不可选
    6)修改功能                      tableWidget  null      隐藏   infoHasValue   type不可选
    7)从寄存器列表中修改功能          ttlvWidget  寄存器      显示   infoHasValue   type不可选
    */

    QTabWidget *tab = (QTabWidget *)toolkit.findParent(obj,"QTabWidget",nullptr);
    bitAddr = bitInfo;
    addType = type;
    if((tab && tab->tabText(tab->currentIndex()).indexOf(tr("线圈")) >= 0) || (tab && tab->tabText(tab->currentIndex()).indexOf(tr("离散量")) >= 0) || info.count() || (obj->objectName() == "ttlvWidget" && ((QWidget*)obj)->layout()->count() > 1))
    {
        /* type不可选 */
        ui->comboBox_type->setEnabled(false);
    }
    else
    {
        /* type可选 */
        ui->comboBox_type->setEnabled(true);
    }
    if((tab && tab->tabText(tab->currentIndex()).indexOf(tr("输入寄存器")) >= 0) || (tab && tab->tabText(tab->currentIndex()).indexOf(tr("离散量")) >= 0))
    {
        ui->comboBox_subType->setEnabled(false);
    }
    else if(type == REG_DROP)
    {
        ui->comboBox_subType->setEnabled(false);
    }
    sourceObj = obj;
    listLayout = new QGridLayout(ui->widget);
    listLayout->setContentsMargins(0,0,0,0);
    ui->widget->setLayout(listLayout);

    memset(bitEnable,true,sizeof(bitEnable));
    if(BIT_HIDE != bitAddr)
    {
        PreQTableWidget *regTable = (PreQTableWidget *)toolkit.findParent(obj,"PreQTableWidget",nullptr);
        QTabWidget *tab1 = (QTabWidget *)toolkit.findParent(regTable,"QTabWidget",nullptr);
        QTabWidget *tab2 = (QTabWidget *)toolkit.findParent(tab1,"QTabWidget",nullptr);
        QMap<QString,QVariant> devInfo = toolkit.readFormConfig("dev");
        qDebug()<<tab1->tabText(tab1->currentIndex())<<tab2->tabText(tab2->currentIndex());

        QMap<QString,QVariant> regInfo;
        if (tab1->tabText(tab1->currentIndex()) == tr("线圈"))
        {
            regInfo = devInfo.value(tab2->tabText(tab2->currentIndex())).toMap().value("register").toMap().value("线圈").toMap().value(regTable->objectName()).toMap();
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("保持寄存器"))
        {
            regInfo = devInfo.value(tab2->tabText(tab2->currentIndex())).toMap().value("register").toMap().value("保持寄存器").toMap().value(regTable->objectName()).toMap();
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("输入寄存器"))
        {
            regInfo = devInfo.value(tab2->tabText(tab2->currentIndex())).toMap().value("register").toMap().value("输入寄存器").toMap().value(regTable->objectName()).toMap();
        }
        else if (tab1->tabText(tab1->currentIndex()) == tr("离散量"))
        {
            regInfo = devInfo.value(tab2->tabText(tab2->currentIndex())).toMap().value("register").toMap().value("离散量").toMap().value(regTable->objectName()).toMap();
        }

//        QMap<QString,QVariant> regInfo = devInfo.value(tab2->tabText(tab2->currentIndex())).toMap().value("register").toMap().value(tab1->tabText(tab1->currentIndex())).toMap().value(regTable->objectName()).toMap();
        int x = ((QWidget *)obj)->frameGeometry().x();
        int y = ((QWidget *)obj)->frameGeometry().y();
        QModelIndex index = regTable->indexAt(QPoint(x, y));
        int currRow = index.row();
        QMap<QString, QVariant>::iterator iter = regInfo.begin();
        while (iter != regInfo.end())
        {
            QJsonArray posArray = iter.value().toMap().value("pos").toJsonArray();
            for (int iPos = 0; iPos < posArray.count(); iPos++)
            {
                QJsonObject posObj = posArray.at(iPos).toObject();
                if(posObj.value("row").toInt() == currRow && posObj.value("bit").toInt() >= 0)
                {
                    if(bitAddr != BIT_HIDE && bitAddr != posObj.value("bit").toInt())
                    {
                        bitEnable[posObj.value("bit").toInt()] = false;
                    }
                }
            }
            iter++;
        }
    }

    QWidget *page = nullptr;
    if(info.count())
    {
        oldName = name;
        ui->lineEdit_name->setText(name);

        if (info.value("type").toString() == "布尔值")
        {
            ui->comboBox_type->setCurrentText(tr("布尔值"));
        }
        else if (info.value("type").toString() == "数值")
        {
            ui->comboBox_type->setCurrentText(tr("数值"));
        }
        else if (info.value("type").toString() == "字节流")
        {
            ui->comboBox_type->setCurrentText(tr("字节流"));
        }
        else if (info.value("type").toString() == "枚举")
        {
            ui->comboBox_type->setCurrentText(tr("枚举"));
        }
//        ui->comboBox_type->setCurrentText(info.value("type").toString());
        if (info.value("subType").toString() == "只读")
        {
            ui->comboBox_subType->setCurrentText(tr("只读"));
        }
        else if (info.value("subType").toString() == "只写")
        {
            ui->comboBox_subType->setCurrentText(tr("只写"));
        }
        else if (info.value("subType").toString() == "读写")
        {
            ui->comboBox_subType->setCurrentText(tr("读写"));
        }
//        ui->comboBox_subType->setCurrentText(info.value("subType").toString());

        if(info.value("type").toString() == "布尔值")
        {
            if(BIT_SHOW == bitAddr)
            {
                ui->lineEdit_name->setEnabled(false);
            }
            page = new addTtlvBool(info,bitAddr,bitEnable);
        }
        else if(info.value("type").toString() == "数值")
        {
            page = new addTtlvNum(info);
        }
        else if(info.value("type").toString() == "字节流")
        {
            page = new addTtlvByte(info);
        }
        else if(info.value("type").toString() == "枚举")
        {
            page = new addTtlvEnum(info);
        }
        oldEventName = info.value("event").toMap().value("eventName").toString();
    }
    else
    {
        page = new addTtlvBool(info,bitAddr,bitEnable);
        if (ui->comboBox_subType->currentText() == tr("只写"))
        {
            QCheckBox* checkBox_event = this->findChild<QCheckBox*>("checkBox_event");
            checkBox_event->setChecked(false);
            checkBox_event->setEnabled(false);
        }
    }
    listLayout->addWidget(page);
    connect(ui->comboBox_type,SIGNAL(currentIndexChanged(QString)),this,SLOT(comboBox_type_currentIndexChanged(QString)));
    connect(ui->comboBox_subType,SIGNAL(currentIndexChanged(QString)),this,SLOT(comboBox_subType_currentIndexChanged(QString)));
    comboBox_subType_currentIndexChanged(ui->comboBox_subType->currentText());
}

addTtlv::~addTtlv()
{
    delete ui;
}

void addTtlv::comboBox_subType_currentIndexChanged(const QString &arg1)
{
    if (arg1 == tr("只写"))
    {
        QCheckBox* checkBox_event = this->findChild<QCheckBox*>("checkBox_event");
        checkBox_event->setChecked(false);
        checkBox_event->setEnabled(false);

        this->findChild<QLabel*>("label_condition")->hide();
        this->findChild<QLabel*>("label_type")->hide();
        this->findChild<QLabel*>("label_name")->hide();
        this->findChild<QLineEdit*>("event_name")->hide();
        this->findChild<QComboBox*>("event_type")->hide();
        if(ui->comboBox_type->currentText() == tr("布尔值"))
        {
            this->findChild<QRadioButton*>("radioButton_false")->hide();
            this->findChild<QRadioButton*>("radioButton_true")->hide();
        }
        else if(ui->comboBox_type->currentText() == tr("数值"))
        {
            this->findChild<QLabel*>("label_value")->hide();
            this->findChild<QLabel*>("label_10")->hide();
            this->findChild<QDoubleSpinBox*>("register_value_min")->hide();
            this->findChild<QDoubleSpinBox*>("register_value_max")->hide();
        }
        else if(ui->comboBox_type->currentText() == tr("字节流"))
        {
            this->findChild<QComboBox*>("comboBox")->hide();
            this->findChild<QLineEdit*>("register_value")->hide();
        }
        else if(ui->comboBox_type->currentText() == tr("枚举"))
        {
            this->findChild<QScrollArea*>("scrollArea")->hide();
        }
    }
    else
    {
        QCheckBox* checkBox_event = this->findChild<QCheckBox*>("checkBox_event");
        checkBox_event->setEnabled(true);
    }
}

bool findInputRegister(QObject *obj,QString name)
{
    QList<QTabWidget*> devTabList = obj->findChildren<QTabWidget*>();
    foreach (QTabWidget* devTab, devTabList)
    {
        QWidget* func04Tab = devTab->findChild<QWidget*>("tab_0x04");
        QList<QLabel*>labelList = func04Tab->findChildren<QLabel*>();
        foreach (QLabel* label, labelList)
        {
            if(label->text() == name)
            {
                return true;
            }
        }
    }
    return false;
}

void addTtlv::on_buttonBox_accepted()
{
    qDebug()<<"on_buttonBox_accepted"<<sourceObj;
    QString name = ui->lineEdit_name->text();
    if(0 == ui->lineEdit_name->text().length())
    {
        QMessageBox::information(0,tr("添加功能失败"),tr("请输入功能名称"),tr("确认"));
        return;
    }
    QObject *obj = toolkit.findParent(sourceObj,nullptr,"MainWindow");

    QMap<QString,QVariant> ttlvInfo = toolkit.readFormConfig("ttlv");
    QMap<QString, QVariant>::iterator ttlvIter = ttlvInfo.begin();
    while (ttlvIter != ttlvInfo.end())
    {
        if (ui->lineEdit_name->text() != oldName)
        {
            if (ttlvIter.key() == ui->lineEdit_name->text() || ttlvIter.value().toMap().value("event").toMap().value("eventName").toString() == ui->lineEdit_name->text())
            {
                QMessageBox::information(0,tr("添加功能失败"),tr("该功能名称已存在"),tr("确认"));
                return;
            }
        }
        ttlvIter ++;
    }

    /* 如果是修改的话，需要判断是否允许修改 */
    if((addType == LIST_MOD || addType == REG_MOD) && ui->comboBox_subType->currentText().indexOf(tr("写")) >= 0 && findInputRegister(obj,name))
    {
        QMessageBox::information(0,tr("警告"),tr("该功能名称已被添加到输入寄存器中，无法修改为读写或者可写类型"),tr("确认"));
        return;
    }

    QMap<QString,QVariant> info;
    QMap<QString,QVariant> event_info;
    int newBitAddr = -1;
    if (ui->comboBox_type->currentText() == tr("布尔值"))
    {
        info.insert("type", "布尔值");
    }
    else if (ui->comboBox_type->currentText() == tr("数值"))
    {
        info.insert("type", "数值");
    }
    else if (ui->comboBox_type->currentText() == tr("字节流"))
    {
        info.insert("type", "字节流");
    }
    else if (ui->comboBox_type->currentText() == tr("枚举"))
    {
        info.insert("type", "枚举");
    }

    if (ui->comboBox_subType->currentText() == tr("只读"))
    {
        info.insert("subType", "只读");
    }
    else if (ui->comboBox_subType->currentText() == tr("只写"))
    {
        info.insert("subType", "只写");
    }
    else if (ui->comboBox_subType->currentText() == tr("读写"))
    {
        info.insert("subType", "读写");
    }
//    info.insert("type",ui->comboBox_type->currentText());
//    info.insert("subType",ui->comboBox_subType->currentText());
    if(tr("布尔值") == ui->comboBox_type->currentText())
    {
        addTtlvBool *page = (addTtlvBool *)ui->widget->layout()->itemAt(0)->widget();
        if(BIT_HIDE != bitAddr)
        {
            newBitAddr = page->pButtonGroup.checkedId();
            qDebug()<<"newBitAddr"<<newBitAddr;
        }

        QMap<QString,QVariant> eventInfo;
        QCheckBox* checkBox_event = this->findChild<QCheckBox*>("checkBox_event");
        eventInfo.insert("addEvent", checkBox_event->isChecked());
        if (checkBox_event->isChecked() ==true)
        {
            QLineEdit* event_name = this->findChild<QLineEdit*>("event_name");
            if(0 == event_name->text().length())
            {
                QMessageBox::information(0,tr("添加功能失败"),tr("请输入事件名称"),tr("确认"));
                return;
            }
            else
            {
                QMap<QString,QVariant> ttlvInfo = toolkit.readFormConfig("ttlv");
                QMap<QString, QVariant>::iterator ttlvIter = ttlvInfo.begin();
                while (ttlvIter != ttlvInfo.end())
                {
                    if (oldEventName != event_name->text())
                    {
                        if (ttlvIter.value().toMap().value("event").toMap().value("eventName").toString() == event_name->text() || ttlvIter.key() == event_name->text())
                        {
                            QMessageBox::information(0,tr("添加功能失败"),tr("该事件名称已存在"),tr("确认"));
                            return;
                        }
                    }
                    if (event_name->text() == name)
                    {
                        QMessageBox::information(0,tr("添加功能失败"),tr("功能名称不能与事件名称相同"),tr("确认"));
                        return;
                    }
                    ttlvIter ++;
                }
            }
            eventInfo.insert("eventName", event_name->text());
            QComboBox* event_type = this->findChild<QComboBox*>("event_type");
            if (event_type->currentText() == tr("信息"))
            {
                eventInfo.insert("eventType", "信息");
            }
            else if (event_type->currentText() == tr("故障"))
            {
                eventInfo.insert("eventType", "故障");
            }
            else if (event_type->currentText() == tr("告警"))
            {
                eventInfo.insert("eventType", "告警");
            }

            QMap<QString,QVariant> conditionsInfo;
            QRadioButton *radioButton_false = this->findChild<QRadioButton *>("radioButton_false");
            if (radioButton_false->isChecked() == true)
            {
                conditionsInfo.insert("bool", false);
            }
            QRadioButton *radioButton_true = this->findChild<QRadioButton *>("radioButton_true");
            if (radioButton_true->isChecked() == true)
            {
                conditionsInfo.insert("bool", true);
            }
            eventInfo.insert("eventConditions", conditionsInfo);
        }
        info.insert("event", eventInfo);
    }
    else if(tr("数值") == ui->comboBox_type->currentText())
    {
        QComboBox* comboBox1 = this->findChild<QComboBox*>("comboBox_numType");
        QComboBox* comboBox2 = this->findChild<QComboBox*>("comboBox_order");
        QDoubleSpinBox* spinBox1 = this->findChild<QDoubleSpinBox*>("doubleSpinBox_multiple");
        QDoubleSpinBox* spinBox2 = this->findChild<QDoubleSpinBox*>("doubleSpinBox_increment");
        if (comboBox1->currentText() == tr("16位有符号整形"))
        {
            info.insert("numType", "16位有符号整形");
        }
        else if (comboBox1->currentText() == tr("16位无符号整形"))
        {
            info.insert("numType", "16位无符号整形");
        }
        else if (comboBox1->currentText() == tr("32位有符号整形"))
        {
            info.insert("numType", "32位有符号整形");
        }
        else if (comboBox1->currentText() == tr("32位无符号整形"))
        {
            info.insert("numType", "32位无符号整形");
        }
        else if (comboBox1->currentText() == tr("64位有符号整形"))
        {
            info.insert("numType", "64位有符号整形");
        }
        else if (comboBox1->currentText() == tr("64位无符号整形"))
        {
            info.insert("numType", "64位无符号整形");
        }
        else if (comboBox1->currentText() == tr("32位单精度浮点型"))
        {
            info.insert("numType", "32位单精度浮点型");
        }
        else if (comboBox1->currentText() == tr("64位双精度浮点型"))
        {
            info.insert("numType", "64位双精度浮点型");
        }
        else if (comboBox1->currentText() == tr("16进制有符号A.B型"))
        {
            info.insert("numType", "16进制有符号A.B型");
        }

        if (comboBox2->currentText() == tr("大端模式"))
        {
            info.insert("byte-order", "大端模式");
        }
        else if (comboBox2->currentText() == tr("小端模式"))
        {
            info.insert("byte-order", "小端模式");
        }
        else if (comboBox2->currentText() == tr("大端模式字节交换"))
        {
            info.insert("byte-order", "大端模式字节交换");
        }
        else if (comboBox2->currentText() == tr("小端模式字节交换"))
        {
            info.insert("byte-order", "小端模式字节交换");
        }
//        info.insert("numType",comboBox1->currentText());
//        info.insert("byte-order",comboBox2->currentText());
        info.insert("multiple",QString::number(spinBox1->value(),'f'));
        info.insert("increment",QString::number(spinBox2->value(),'f'));

        QMap<QString,QVariant> eventInfo;
        QCheckBox* checkBox_event = this->findChild<QCheckBox*>("checkBox_event");
        eventInfo.insert("addEvent", checkBox_event->isChecked());
        if (checkBox_event->isChecked() ==true)
        {
            QLineEdit* event_name = this->findChild<QLineEdit*>("event_name");
            if(0 == event_name->text().length())
            {
                QMessageBox::information(0,tr("添加功能失败"),tr("请输入事件名称"),tr("确认"));
                return;
            }
            else
            {
//                QMap<QString,QVariant> ttlvInfo = toolkit.readFormConfig("ttlv");
                ttlvIter = ttlvInfo.begin();
                while (ttlvIter != ttlvInfo.end())
                {
                    if (oldEventName != event_name->text())
                    {
                        if (ttlvIter.value().toMap().value("event").toMap().value("eventName").toString() == event_name->text() || ttlvIter.key() == event_name->text())
                        {
                            QMessageBox::information(0,tr("添加功能失败"),tr("该事件名称已存在"),tr("确认"));
                            return;
                        }
                    }
                    if (event_name->text() == name)
                    {
                        QMessageBox::information(0,tr("添加功能失败"),tr("功能名称不能与事件名称相同"),tr("确认"));
                        return;
                    }
                    ttlvIter ++;
                }
            }
            eventInfo.insert("eventName", event_name->text());
            QComboBox* event_type = this->findChild<QComboBox*>("event_type");
            if (event_type->currentText() == tr("信息"))
            {
                eventInfo.insert("eventType", "信息");
            }
            else if (event_type->currentText() == tr("故障"))
            {
                eventInfo.insert("eventType", "故障");
            }
            else if (event_type->currentText() == tr("告警"))
            {
                eventInfo.insert("eventType", "告警");
            }
            QMap<QString,QVariant> conditionsInfo;
            QDoubleSpinBox *register_value_min = this->findChild<QDoubleSpinBox*>("register_value_min");
            QDoubleSpinBox *register_value_max = this->findChild<QDoubleSpinBox*>("register_value_max");
            if (register_value_min->value() > register_value_max->value())
            {
                QMessageBox::information(0,tr("添加功能失败"),tr("事件触发条件最小值不能大于最大值"),tr("确认"));
                return;
            }
            conditionsInfo.insert("miniMum", QString::number(register_value_min->value(), 'f', 6));
            conditionsInfo.insert("maxMum", QString::number(register_value_max->value(), 'f', 6));
            eventInfo.insert("eventConditions", conditionsInfo);
        }
        info.insert("event", eventInfo);
    }
    else if(tr("枚举") == ui->comboBox_type->currentText())
    {
        QComboBox* comboBox1 = this->findChild<QComboBox*>("comboBox_numType");
        QComboBox* comboBox2 = this->findChild<QComboBox*>("comboBox_order");
        if (comboBox1->currentText() == tr("16位有符号整形"))
        {
            info.insert("numType", "16位有符号整形");
        }
        else if (comboBox1->currentText() == tr("16位无符号整形"))
        {
            info.insert("numType", "16位无符号整形");
        }
        else if (comboBox1->currentText() == tr("32位有符号整形"))
        {
            info.insert("numType", "32位有符号整形");
        }
        else if (comboBox1->currentText() == tr("32位无符号整形"))
        {
            info.insert("numType", "32位无符号整形");
        }
        else if (comboBox1->currentText() == tr("64位有符号整形"))
        {
            info.insert("numType", "64位有符号整形");
        }
        else if (comboBox1->currentText() == tr("64位无符号整形"))
        {
            info.insert("numType", "64位无符号整形");
        }
        else if (comboBox1->currentText() == tr("32位单精度浮点型"))
        {
            info.insert("numType", "32位单精度浮点型");
        }
        else if (comboBox1->currentText() == tr("64位双精度浮点型"))
        {
            info.insert("numType", "64位双精度浮点型");
        }

        if (comboBox2->currentText() == tr("大端模式"))
        {
            info.insert("byte-order", "大端模式");
        }
        else if (comboBox2->currentText() == tr("小端模式"))
        {
            info.insert("byte-order", "小端模式");
        }
        else if (comboBox2->currentText() == tr("大端模式字节交换"))
        {
            info.insert("byte-order", "大端模式字节交换");
        }
        else if (comboBox2->currentText() == tr("小端模式字节交换"))
        {
            info.insert("byte-order", "小端模式字节交换");
        }
//        info.insert("numType",comboBox1->currentText());
//        info.insert("byte-order",comboBox2->currentText());
        info.insert("multiple",QString::number(1.000000,'f'));
        info.insert("increment",QString::number(0.000000,'f'));
        QTableWidget* tableEnum = this->findChild<QTableWidget*>();
        if(tableEnum->rowCount() == 1)
        {
            QMessageBox::information(0,tr("添加功能失败"),tr("枚举类型至少需要添加一个属性值"),tr("确认"));
            return;
        }
        QMap<QString,QVariant> enumInfo;
        for(int iEnum = 0;iEnum<tableEnum->rowCount()-1;iEnum++)
        {
            QSpinBox *spinBox1 = (QSpinBox *)tableEnum->cellWidget(iEnum,0);
            QLineEdit *lineEdit1 = (QLineEdit *)tableEnum->cellWidget(iEnum,1);
            if(enumInfo.find(QString::number(spinBox1->value())) != enumInfo.end())
            {
                QMessageBox::information(0,tr("添加功能失败"),tr("枚举类型的属性值存在重复"),tr("确认"));
                return;
            }
            if(lineEdit1->text().isEmpty())
            {
                QMessageBox::information(0,tr("添加功能失败"),tr("枚举类型的属性描述不能为空"),tr("确认"));
                return;
            }
            enumInfo.insert(QString::number(spinBox1->value()),lineEdit1->text());
        }
        info.insert("enum",enumInfo);

        QMap<QString,QVariant> eventInfo;
        QCheckBox* checkBox_event = this->findChild<QCheckBox*>("checkBox_event");
        eventInfo.insert("addEvent", checkBox_event->isChecked());
        if (checkBox_event->isChecked() ==true)
        {
            QLineEdit* event_name = this->findChild<QLineEdit*>("event_name");
            if(0 == event_name->text().length())
            {
                QMessageBox::information(0,tr("添加功能失败"),tr("请输入事件名称"),tr("确认"));
                return;
            }
            else
            {
                QMap<QString,QVariant> ttlvInfo = toolkit.readFormConfig("ttlv");
                QMap<QString, QVariant>::iterator ttlvIter = ttlvInfo.begin();
                while (ttlvIter != ttlvInfo.end())
                {
                    if (oldEventName != event_name->text())
                    {
                        if (ttlvIter.value().toMap().value("event").toMap().value("eventName").toString() == event_name->text() || ttlvIter.key() == event_name->text())
                        {
                            QMessageBox::information(0,tr("添加功能失败"),tr("该事件名称已存在"),tr("确认"));
                            return;
                        }
                    }
                    if (event_name->text() == name)
                    {
                        QMessageBox::information(0,tr("添加功能失败"),tr("功能名称不能与事件名称相同"),tr("确认"));
                        return;
                    }
                    ttlvIter ++;
                }
            }
            eventInfo.insert("eventName", event_name->text());
            QComboBox* event_type = this->findChild<QComboBox*>("event_type");
            if (event_type->currentText() == tr("信息"))
            {
                eventInfo.insert("eventType", "信息");
            }
            else if (event_type->currentText() == tr("故障"))
            {
                eventInfo.insert("eventType", "故障");
            }
            else if (event_type->currentText() == tr("告警"))
            {
                eventInfo.insert("eventType", "告警");
            }
            QMap<QString,QVariant> conditionsInfo;
            QWidget* scrollArea = this->findChild<QWidget*>("scrollAreaWidgetContents_2");
            QList<QCheckBox *> checkBoxList = scrollArea->findChildren<QCheckBox *>();
            int isTrue = 0;
            for (int i = 0; i <= (checkBoxList.count() - 1); i++)
            {
                if (checkBoxList[i]->isChecked() == true)
                {
                    isTrue ++;
                }
                conditionsInfo.insert(checkBoxList[i]->text(), checkBoxList[i]->isChecked());
            }
            if (isTrue == 0)
            {
                QMessageBox::information(0,tr("添加功能失败"),tr("事件触发条件至少选择一个"),tr("确认"));
                return;
            }
            eventInfo.insert("eventConditions", conditionsInfo);
        }
        info.insert("event", eventInfo);
    }
    else if(tr("字节流") == ui->comboBox_type->currentText())
    {
        QSpinBox* spinBox1 = this->findChild<QSpinBox*>("spinBox_len");
        info.insert("len",spinBox1->value());

        QMap<QString,QVariant> eventInfo;
        QCheckBox* checkBox_event = this->findChild<QCheckBox*>("checkBox_event");
        eventInfo.insert("addEvent", checkBox_event->isChecked());
        if (checkBox_event->isChecked() ==true)
        {
            QLineEdit* event_name = this->findChild<QLineEdit*>("event_name");
            if(0 == event_name->text().length())
            {
                QMessageBox::information(0,tr("添加功能失败"),tr("请输入事件名称"),tr("确认"));
                return;
            }
            else
            {
                QMap<QString,QVariant> ttlvInfo = toolkit.readFormConfig("ttlv");
                QMap<QString, QVariant>::iterator ttlvIter = ttlvInfo.begin();
                while (ttlvIter != ttlvInfo.end())
                {
                    if (oldEventName != event_name->text())
                    {
                        if (ttlvIter.value().toMap().value("event").toMap().value("eventName").toString() == event_name->text() || ttlvIter.key() == event_name->text())
                        {
                            QMessageBox::information(0,tr("添加功能失败"),tr("该事件名称已存在"),tr("确认"));
                            return;
                        }
                    }
                    if (event_name->text() == name)
                    {
                        QMessageBox::information(0,tr("添加功能失败"),tr("功能名称不能与事件名称相同"),tr("确认"));
                        return;
                    }
                    ttlvIter ++;
                }
            }
            eventInfo.insert("eventName", event_name->text());
            QComboBox* event_type = this->findChild<QComboBox*>("event_type");
            if (event_type->currentText() == tr("信息"))
            {
                eventInfo.insert("eventType", "信息");
            }
            else if (event_type->currentText() == tr("故障"))
            {
                eventInfo.insert("eventType", "故障");
            }
            else if (event_type->currentText() == tr("告警"))
            {
                eventInfo.insert("eventType", "告警");
            }
            QMap<QString,QVariant> conditionsInfo;
            QLineEdit *register_value = this->findChild<QLineEdit*>("register_value");
            QComboBox* comboBox = this->findChild<QComboBox*>("comboBox");
            if (register_value->text().length() <= 0)
            {
                QMessageBox::information(0,tr("添加功能失败"),tr("字节流类型的事件触发条件不能为空"),tr("确认"));
                return;
            }
            conditionsInfo.insert("byteType", register_value->text());
            if (comboBox->currentText() == tr("字节流"))
            {
                if (register_value->text().length() % 2)
                {
                    QMessageBox::information(0,tr("添加功能失败"),tr("触发条件字节流不可为奇数"),tr("确认"));
                    return;
                }
                conditionsInfo.insert("byteType", "字节流");
                conditionsInfo.insert("byteStream", register_value->text());
            }
            else if (comboBox->currentText() == tr("字符串"))
            {
                QString text = register_value->text();
                QByteArray byte = text.toUtf8();
                QString stringToByteHex;
                for(int i = 0; i < text.size(); i++)
                {
                    stringToByteHex = stringToByteHex + QString("%1").arg(int(byte.at(i)),0,16);
                }
                conditionsInfo.insert("byteType", "字符串");
                conditionsInfo.insert("byteStream", stringToByteHex);
                byte.clear();
                byte.squeeze();
            }
            eventInfo.insert("eventConditions",conditionsInfo);
        }
        info.insert("event", eventInfo);
    }

    if(addType == REG_ADD)
    {
        qDebug()<<"从寄存器页面添加功能";
        if((info.find("type").value().toString() == "字节流" && info.find("len").value().toInt() > 2)
                || (info.find("type").value().toString() == "数值" && info.find("numType").value().toString().indexOf("16位") == -1))
        {
            if((info.find("type").value().toString() == "字节流" && info.find("len").value().toInt() > 2)
                    || (info.find("type").value().toString() == "数值" && info.find("numType").value().toString().indexOf("16位") == -1))
            {
                QWidget *box = (QWidget *)sourceObj;
                PreQTableWidget *table = (PreQTableWidget *)toolkit.findParent(box,"PreQTableWidget",nullptr);
                int x = box->frameGeometry().x();
                int y = box->frameGeometry().y();
                QModelIndex index = table->indexAt(QPoint(x, y));
                int row = index.row();
                int column = index.column();
                int len = 0;

                if(info.find("type").value().toString() == "字节流")
                {
                    len = info.find("len").value().toInt();
                    len = len/2 + (len%2==0?0:1);
                }
                else if(info.find("type").value().toString() == "数值")
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
                /* 可用长度判断 */
                qDebug()<<"rowCount"<<table->rowCount();
                if(table->rowCount() < len)
                {
                    QMessageBox::information(0,tr("警告"),tr("该功能长度大于当前寄存器长度"),tr("确认"));
                    return;
                }
                int usableLen = 0;
                for(usableLen=0;usableLen<table->rowCount()-row;usableLen++)
                {
                    if(table->cellWidget(row+usableLen,column)->layout()->count() > 1)
                    {
                        break;
                    }
                    QLayoutItem *item = table->cellWidget(row+usableLen,column)->layout()->itemAt(0);
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
            }
        }
    }
    qDebug()<<oldName<<sourceObj;
    if(oldName.length())
    {
        emit modResultSignal(oldName,name,info);
    }
    else
    {
        emit addResultSignal(name,info);
    }
    if(nullptr != sourceObj)
    {
        emit addRegListSignal(sourceObj,name,info,newBitAddr,true);
        if(newBitAddr >= 0 && newBitAddr != bitAddr)
        {
            emit modBitSignal(sourceObj,name,bitAddr,newBitAddr);
        }
    }
    this->close();
}

void addTtlv::on_buttonBox_rejected()
{
    this->close();
}

void addTtlv::comboBox_type_currentIndexChanged(const QString &arg1)
{
    qDebug()<<"on_comboBox_type_currentIndexChanged";
    QWidget *page = NULL;
    if(arg1 == tr("布尔值"))
    {
        page = new addTtlvBool(QMap<QString,QVariant>(),bitAddr,bitEnable);
    }
    else if(arg1 == tr("数值"))
    {
        page = new addTtlvNum(QMap<QString,QVariant>());
    }
    else if(arg1 == tr("字节流"))
    {
        page = new addTtlvByte(QMap<QString,QVariant>());
    }
    else if(arg1 == tr("枚举"))
    {
        page = new addTtlvEnum(QMap<QString,QVariant>());
    }
    else
    {
        return;
    }
    toolkit.clearLayout(listLayout);
    listLayout->addWidget(page);
    comboBox_subType_currentIndexChanged(ui->comboBox_subType->currentText());
}
