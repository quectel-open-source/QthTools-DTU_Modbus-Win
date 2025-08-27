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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Form.h"
#include <QDebug>
#include "devtab.h"
#include "add/adddev.h"
#include <QLabel>
#include <QMessageBox>
#include <QTabBar>
#include "preqtablewidget.h"
#include "add/addregisterform.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include "help/userdoc.h"
#include "help/feedback.h"
#include "help/about.h"
#include <QJsonArray>
#include <QFileInfo>
#include <QProcess>
#include "down/filedown.h"
#include <QtGui>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    /* 右键菜单 */
    ui->tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tabWidget,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(tableContextMenuRequested(QPoint)));
    table_widget_menu = new QMenu(ui->tabWidget);
    action_modify = new QAction(tr("修改"), ui->tabWidget);
    action_remove = new QAction(tr("删除"), ui->tabWidget);
    connect(action_modify, SIGNAL(triggered()), this, SLOT(slotActionModify()));
    connect(action_remove, SIGNAL(triggered()), this, SLOT(slotActionRemove()));
    table_widget_menu->addAction(action_modify);
    table_widget_menu->addAction(action_remove);

    updateVersion = new toolUpdate();
    updateVersion->checkForUpdates();

    ui->splitter_main->setStretchFactor(0,2);
    ui->splitter_main->setStretchFactor(1,12);

    if (checkFeatureLabel())
    {
        QMessageBox::information(this,tr("警告"),tr("当前版本不支持同名功能标签，已删除部分同名功能，请检查删除后的格式"),tr("确定"));
    }

    if (checkMultiple())
    {
        QMessageBox::information(this,tr("警告"),tr("部分数值类型的倍率超出设定范围（0.000001-1.000000），已将倍率修改为1.000000"),tr("确定"));
    }

    if (toolkit.readNewConfigFile() == false)
    {
        QMap<QString,QVariant> product = toolkit.readFormConfig("product");
        QMap<QString, QVariant>::iterator iter = product.begin();
        QMap<QString,QVariant> info;
        QString name;
        while(iter != product.end())
        {
            info = iter.value().toMap();
            name = iter.key();
            iter ++;
            break;
        }

        info.insert("devType", "网关版");
        toolkit.writeFormConfig("product/" + name, info);
        qDebug()<<"直连版下只能存在一个设备";
        QMessageBox::information(this, tr("警告"), tr("直连版产品存在多个设备型号或多个从机地址，已将直连版修改为网关版"), tr("确认"));
    }
    else
    {
        QMap<QString,QVariant> product = toolkit.readFormConfig("product");
        QMap<QString, QVariant>::iterator iter = product.begin();
        QMap<QString,QVariant> info;
        QString name;
        bool isExist = false;
        if (!product.isEmpty())
        {
            while(iter != product.end())
            {
                info = iter.value().toMap();
                name = iter.key();
                if (iter.value().toMap().value("devType").toString().size() != 0)
                {
                    isExist = true;
                }
                break;
            }
            if(!isExist && !info.isEmpty())
            {
                info.insert("devType", "直连版");
            }
            toolkit.writeFormConfig("product/" + name, info);
        }
    }

    listToolBox = new ToolBox(this);
    Form *productForm = new Form(tr("添加产品"));
    listToolBox->addWidget(tr("产品信息"), productForm);
    connect(productForm,SIGNAL(modifySignal()), this, SLOT(processAddProductModifySignal()));
    listToolBox->addWidget(tr("串口列表"), new Form(tr("添加串口")));
    listToolBox->addWidget(tr("功能列表"), new Form(tr("添加功能")));
    listLayout = new QGridLayout(ui->widget_list);
    listLayout->addWidget(listToolBox);
    listLayout->setContentsMargins(0,0,0,0);

    /* 读文件 */
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev");
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end())
    {
        /* 创建设备tab */
        addResultSlot(iter.key(),iter.value().toMap(),false);
        iter++;
    }

    QMap<QString,QVariant> language = toolkit.readLanguageConfig("Language");
    if (!language.isEmpty())
    {
        if (language.value("language").toString() == "english")
        {
            languageChange(1);
        }
        else if (language.value("language").toString() == "chinese")
        {
            languageChange(0);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
    func:检查寄存器值倍率（0.000001-1）
*/
bool MainWindow::checkMultiple()
{
    bool isRepeat = false;
    QMap<QString,QVariant> devInfo = toolkit.readFormConfig("dev");
    QMap<QString,QVariant>::iterator devIter = devInfo.begin();
    QList<QString> labelCount;
    while (devIter != devInfo.end())
    {
        QString devKey = devIter.key();
        QMap<QString,QVariant> regInfo = devIter.value().toMap().value("register").toMap();
        QMap<QString,QVariant>::iterator regIter = regInfo.begin();
        while (regIter != regInfo.end())
        {
            QString regKey = regIter.key();
            QMap<QString,QVariant> addressInfo = regIter.value().toMap();
            QMap<QString,QVariant>::iterator addressIter = addressInfo.begin();
            while (addressIter != addressInfo.end())
            {
                QString addressKey = addressIter.key();
                QMap<QString,QVariant> funInfo = addressIter.value().toMap();
                QMap<QString,QVariant>::iterator funIter = funInfo.begin();
                while (funIter != funInfo.end())
                {
                    if (funIter.value().toMap().value("attr").toMap().value("type").toString() == "数值")
                    {
                        if (funIter.value().toMap().value("attr").toMap().value("multiple").toDouble() > 1 || funIter.value().toMap().value("attr").toMap().value("multiple").toDouble() < 0.000001)
                        {
                            QString key = "dev/" + devKey + "/register/" + regKey + "/" + addressKey + "/" +funIter.key() + "/attr";
                            QMap<QString,QVariant> newMultiple = funIter.value().toMap().value("attr").toMap();
                            newMultiple.insert("multiple", "1.000000");
                            toolkit.writeFormConfig(key,newMultiple);
                            isRepeat = true;
                        }
                    }
                    funIter ++;
                }
                addressIter ++;
            }
            regIter ++;
        }
        devIter ++;
    }
    return isRepeat;
}

/*
    func:检查并删除同名功能标签
*/
bool MainWindow::checkFeatureLabel()
{
    bool isRepeat = false;
    QMap<QString,QVariant> devInfo = toolkit.readFormConfig("dev");
    QMap<QString,QVariant>::iterator devIter = devInfo.begin();
    QList<QString> labelCount;
    while (devIter != devInfo.end())
    {
        QString devKey = devIter.key();
        QMap<QString,QVariant> regInfo = devIter.value().toMap().value("register").toMap();
        QMap<QString,QVariant>::iterator regIter = regInfo.begin();
        while (regIter != regInfo.end())
        {
            QString regKey = regIter.key();
            QMap<QString,QVariant> addressInfo = regIter.value().toMap();
            QMap<QString,QVariant>::iterator addressIter = addressInfo.begin();
            while (addressIter != addressInfo.end())
            {
                QString addressKey = addressIter.key();
                QMap<QString,QVariant> funInfo = addressIter.value().toMap();
                QMap<QString,QVariant>::iterator funIter = funInfo.begin();
                while (funIter != funInfo.end())
                {
                    QString key = "dev/" + devKey + "/register/" + regKey + "/" + addressKey + "/" +funIter.key();
                    if (labelCount.contains(funIter.key()) == true)
                    {
                        qDebug()<<"删除同名功能配置信息"<<key;
                        toolkit.removeFormConfig(key);
                        isRepeat = true;
                    }
                    else
                    {
                        labelCount.append(funIter.key());
                        QMap<QString,QVariant> ttlvInfo = funIter.value().toMap();
                        QJsonArray posArray = ttlvInfo.value("pos").toJsonArray();
                        if (posArray.count() > 1)
                        {
                            int arr = posArray.count();
                            for (int iPos = 0; iPos < arr; iPos++)
                            {
                                if(iPos > 0)
                                {
                                    qDebug()<<"删除地址下同名功能配置信息"<<key;
                                    posArray.removeAt(1);
                                    isRepeat = true;
                                }
                            }
                            ttlvInfo.insert("pos",posArray);
                            toolkit.writeFormConfig(key,ttlvInfo);
                        }
                    }
                    funIter ++;
                }
                addressIter ++;
            }
            regIter ++;
        }
        devIter ++;
    }

    QMap<QString,QVariant> qttlvInfo = toolkit.readFormConfig("ttlv");
    QMap<QString,QVariant>::iterator qttlvIter = qttlvInfo.begin();
    while (qttlvIter != qttlvInfo.end())
    {
        if (labelCount.contains(qttlvIter.key()) == false)
        {
            toolkit.removeFormConfig("ttlv/" + qttlvIter.key());
        }
        qttlvIter ++;
    }

    return isRepeat;
}

/*
    func:点击tab标签
*/
void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    if(index != ui->tabWidget->count()-1)
    {
        return;
    }
    PreQTableWidget *productTable = this->findChild<PreQTableWidget *>(tr("添加产品"));
    if(0 == productTable->rowCount())
    {
        QMessageBox::information(0,tr("警告"),tr("请先添加产品信息"),tr("确认"));
        return;
    }
    PreQTableWidget *uartTable = this->findChild<PreQTableWidget *>(tr("添加串口"));
    if(0 == uartTable->rowCount())
    {
        QMessageBox::information(0,tr("警告"),tr("请先添加串口信息"),tr("确认"));
        return;
    }
    QList<QLabel *> labelList = uartTable->findChildren<QLabel *>();
    QStringList uartList;
    foreach (QLabel *label , labelList)
    {
        uartList.append(label->text());
    }
    addDev *addDevPage = new addDev(toolkit.getDevType(),uartList,nullptr,QMap<QString,QVariant>());
    currTabIndex = ui->tabWidget->currentIndex();
    connect(addDevPage,SIGNAL(addResultSignal(QString,QMap<QString,QVariant>,bool)),this,SLOT(addResultSlot(QString,QMap<QString,QVariant>,bool)));
    connect(addDevPage,SIGNAL(unselect()),this,SLOT(unselectSlot()));
    addDevPage->show();
}

/*
    func:取消增加tab标签
*/
void MainWindow::unselectSlot()
{
    if(currTabIndex != ui->tabWidget->count()-1)
    {
        ui->tabWidget->setCurrentIndex(currTabIndex);
    }
    else if(ui->tabWidget->count() >= 2)
    {
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-2);
    }
}

/*
    func:增加tab标签
*/
void MainWindow::addResultSlot(QString name,QMap<QString,QVariant> info,bool isSave)
{
    QMap<QString,QVariant> devInfo;
    QMap<QString,QVariant> productInfo;
    QMap<QString,QVariant> writeModeInfo;
    if(isSave)
    {
        devInfo = info.find("addr").value().toMap();
        productInfo = info.find("product").value().toMap();
        writeModeInfo = info.find("writeMode").value().toMap();
    }
    QMap<QString,QVariant> regInfo;
    if(info.find("register") != info.end())
    {
        regInfo = info.find("register").value().toMap();
    }
    devTab *devPage = new devTab(regInfo,ui->tabWidget);
    ui->tabWidget->insertTab(ui->tabWidget->count()-1,devPage,name);
    ui->tabWidget->setCurrentWidget(devPage);
    if(isSave)
    {
        toolkit.writeFormConfig("dev/"+name+"/addr",devInfo);
        toolkit.writeFormConfig("dev/"+name+"/product",productInfo);
        toolkit.writeFormConfig("dev/"+name+"/writeMode",writeModeInfo);
    }
    if (toolkit.getDevType() != true && ui->tabWidget->count() > 1)
    {
        ui->tabWidget->setTabEnabled(ui->tabWidget->count() - 1,false);
    }
}

/*
    func:修改tab标签
*/
void MainWindow::modResultSlot(QString oldName,QString newName,QMap<QString,QVariant> info)
{
    QMap<QString,QVariant> devInfo = info.find("addr").value().toMap();
    QMap<QString,QVariant> productInfo = info.find("product").value().toMap();
    QMap<QString,QVariant> writeModeInfo = info.find("writeMode").value().toMap();
    int index = ui->tabWidget->tabBar()->tabAt(actionPos);
    ui->tabWidget->setCurrentIndex(index);
    ui->tabWidget->tabBar()->setTabText(index,newName);
    QMap<QString,QVariant> oldInfo = toolkit.readFormConfig("dev/"+oldName);
    toolkit.removeFormConfig("dev/"+oldName);
    toolkit.writeFormConfig("dev/"+newName,oldInfo);
    toolkit.writeFormConfig("dev/"+newName+"/addr",devInfo);
    toolkit.writeFormConfig("dev/"+newName+"/product",productInfo);
    toolkit.writeFormConfig("dev/"+newName+"/writeMode",writeModeInfo);
}

void MainWindow::processAddProductModifySignal()
{
    if (toolkit.getDevType() == true)
    {
        ui->tabWidget->setTabEnabled(ui->tabWidget->count() - 1,true);
    }
    else
    {
        ui->tabWidget->setTabEnabled(ui->tabWidget->count() - 1,false);
    }
}

/*
    func:删除tab标签
*/
void MainWindow::slotActionRemove()
{
    QMessageBox box(QMessageBox::Warning,tr("警告"),tr("该操作将删除该设备页面下所有信息，请确认是否继续"));
    box.setStandardButtons (QMessageBox::Yes|QMessageBox::No);
    box.setButtonText (QMessageBox::Yes,QString(tr("确定")));
    box.setButtonText (QMessageBox::No,QString(tr("取消")));
    if(QMessageBox::No == box.exec())
    {
        return;
    }
    int index = ui->tabWidget->tabBar()->tabAt(actionPos);
    if(index != ui->tabWidget->count()-1)
    {
        qDebug()<<"删除tab"<<ui->tabWidget->tabBar()->tabText(index);

        QMap<QString,QVariant> removeInfo = toolkit.readFormConfig("dev/"+ui->tabWidget->tabBar()->tabText(index) + "/register");
        QMap<QString,QVariant>::iterator iter = removeInfo.begin();
        while (iter != removeInfo.end())
        {
            QMap<QString,QVariant> regInfo = iter.value().toMap();
            QMap<QString,QVariant>::iterator regiter = regInfo.begin();
            while (regiter != regInfo.end())
            {
                QMap<QString,QVariant> funInfo = regiter.value().toMap();
                QMap<QString,QVariant>::iterator funIter = funInfo.begin();
                while (funIter != funInfo.end())
                {
                    qDebug()<<"删除功能标签："<<funIter.key();
                    toolkit.removeFormConfig("ttlv/" + funIter.key());
                    funIter ++;
                }
                regiter ++;
            }
            iter ++;
        }

        toolkit.removeFormConfig("dev/"+ui->tabWidget->tabBar()->tabText(index));
        ui->tabWidget->removeTab(index);
        ui->tabWidget->setCurrentIndex(index-1);
    }
}

/*
    func:tab标签弹出右键菜单
*/
void MainWindow::tableContextMenuRequested(const QPoint &pos)
{
    actionPos = pos;
    int index = ui->tabWidget->tabBar()->tabAt(actionPos);
    if(index != -1 && index != ui->tabWidget->count()-1)
    {
        table_widget_menu->exec(QCursor::pos());
    }
}


/*
    func:修改tab标签
*/
void MainWindow::slotActionModify()
{
    int index = ui->tabWidget->tabBar()->tabAt(actionPos);
    qDebug()<<"找到tab"<<index;
    PreQTableWidget *table = this->findChild<PreQTableWidget *>(tr("添加串口"));
    QList<QLabel *> labelList = table->findChildren<QLabel *>();
    QStringList uartList;
    foreach (QLabel *label , labelList)
    {
        uartList.append(label->text());
    }
    if(0 == uartList.count())
    {
        QMessageBox::information(0,tr("警告"),tr("请先添加至少一个串口"),tr("确认"));
        return;
    }
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev/"+ui->tabWidget->tabText(index));
    addDev *addDevPage = new addDev(toolkit.getDevType(),uartList,ui->tabWidget->tabText(index),info);
    connect(addDevPage,SIGNAL(modResultSignal(QString,QString,QMap<QString,QVariant>)),this,SLOT(modResultSlot(QString,QString,QMap<QString,QVariant>)));
    addDevPage->show();
}

/*
    func:保存工作区配置文件
*/
void MainWindow::on_action_outputProject_triggered()
{
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
    QString file = QFileDialog::getSaveFileName(0,tr("选择文件存储"),"","*.ini");
    QFile::remove(file);
    if(!QFile::copy(configfile,file))
    {
        QMessageBox::information(0,tr("警告"),tr("保存失败"),tr("确认"));
        return;
    }
}

/*
    func:打开工作区配置文件
*/
void MainWindow::on_action_inputProject_triggered()
{
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
    QString file = QFileDialog::getOpenFileName(NULL,tr("选取文件"),".","*.ini");
    if(file.isEmpty())
    {
        QMessageBox::information(this,tr("错误"),tr("文件打开失败"),tr("确认"));
        return;
    }

    qDebug() << "set Permissions result 1:" << QFile::setPermissions(configfile, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    QFile::remove(configfile);
    if(!QFile::copy(file,configfile))
    {
        QMessageBox::information(this,tr("错误"),tr("文件拷贝失败"),tr("确认"));
        return;
    }
    qDebug() << "set Permissions result 2:" << QFile::setPermissions(configfile, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    if (checkFeatureLabel())
    {
        QMessageBox::information(this,tr("警告"),tr("当前版本不支持同名功能标签，已删除部分同名功能，请检查删除后的格式"),tr("确定"));
    }

    if (checkMultiple())
    {
        QMessageBox::information(this,tr("警告"),tr("部分数值类型的倍率超出设定范围（0.000001-1.000000），已将倍率修改为1.000000"),tr("确定"));
    }

    if (toolkit.readNewConfigFile() == false)
    {
        QMap<QString,QVariant> product = toolkit.readFormConfig("product");
        QMap<QString, QVariant>::iterator iter = product.begin();
        QMap<QString,QVariant> info;
        QString name;
        while(iter != product.end())
        {
            info = iter.value().toMap();
            name = iter.key();
            iter ++;
            break;
        }

        info.insert("devType", "网关版");
        toolkit.writeFormConfig("product/" + name, info);
        qDebug()<<"直连版下只能存在一个设备";
        QMessageBox::information(this, tr("警告"), tr("直连版产品存在多个设备型号或多个从机地址，已将直连版修改为网关版"), tr("确认"));
    }
    else
    {
        QMap<QString,QVariant> product = toolkit.readFormConfig("product");
        QMap<QString, QVariant>::iterator iter = product.begin();
        QMap<QString,QVariant> info;
        QString name;
        bool isExist = false;
        if (!product.isEmpty())
        {
            while(iter != product.end())
            {
                info = iter.value().toMap();
                name = iter.key();
                if (iter.value().toMap().value("devType").toString().size() != 0)
                {
                    isExist = true;
                }
                break;
            }
            if(!isExist && !info.isEmpty())
            {
                info.insert("devType", "直连版");
            }
            toolkit.writeFormConfig("product/" + name, info);
        }
    }

    /* 先删除所有控件 */
    if (listToolBox != NULL)
    {
        delete  listToolBox;
        listToolBox = NULL;
        listToolBox = new ToolBox();
    }

    Form *productForm = new Form(tr("添加产品"));
    listToolBox->addWidget(tr("产品信息"), productForm);
    connect(productForm,SIGNAL(modifySignal()), this, SLOT(processAddProductModifySignal()));
    listToolBox->addWidget(tr("串口列表"), new Form(tr("添加串口")));
    listToolBox->addWidget(tr("功能列表"), new Form(tr("添加功能")));
    listLayout->addWidget(listToolBox);
    int tabCount = ui->tabWidget->count();
    for (int i = tabCount-2; i >= 0; i--)
    {
        QWidget *rmTabWidget = ui->tabWidget->widget(i);
        if (rmTabWidget != NULL)
        {
            delete rmTabWidget;
            rmTabWidget = NULL;
        }
    }

    /* 然后模拟重新打开窗 */
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev");
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end())
    {
        /* 创建设备tab */
        addResultSlot(iter.key(),iter.value().toMap(),false);
        iter++;
    }
    if (toolkit.getDevType() == true)
    {
        ui->tabWidget->setTabEnabled(ui->tabWidget->count() - 1,true);
    }
    else
    {
        ui->tabWidget->setTabEnabled(ui->tabWidget->count() - 1,false);
    }
}

/*
    func:清空工作区
*/
void MainWindow::on_action_clearProject_triggered()
{
    /* 删除配置文件 */
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
    QFile::remove(configfile);
    /* 删除所有控件 */
    if (listToolBox != NULL)
    {
        delete  listToolBox;
        listToolBox = NULL;
        listToolBox = new ToolBox();
    }

    Form *productForm = new Form(tr("添加产品"));
    listToolBox->addWidget(tr("产品信息"), productForm);
    connect(productForm,SIGNAL(modifySignal()), this, SLOT(processAddProductModifySignal()));
    listToolBox->addWidget(tr("串口列表"), new Form(tr("添加串口")));
    listToolBox->addWidget(tr("功能列表"), new Form(tr("添加功能")));
    listLayout->addWidget(listToolBox);

    int tabCount = ui->tabWidget->count();
    for (int i = tabCount-2; i >= 0; i--)
    {
        QWidget *rmTabWidget = ui->tabWidget->widget(i);
        if (rmTabWidget != NULL)
        {
            delete rmTabWidget;
            rmTabWidget = NULL;
        }
    }
}

/*
    func:导出配置文件
*/
void MainWindow::on_action_output_triggered()
{
    if(ui->tabWidget->count() == 1)
    {
        QMessageBox::information(0,tr("警告"),tr("请先添加设备信息"),tr("确认"));
        return;
    }
    PreQTableWidget *productTable = this->findChild<PreQTableWidget *>(tr("添加产品"));
    PreQTableWidget *ttlvTable = this->findChild<PreQTableWidget *>(tr("添加功能"));
    if(productTable->rowCount() == 0)
    {
        QMessageBox::information(0,tr("警告"),tr("请先添加产品信息"),tr("确认"));
        return;
    }
    if(ttlvTable->rowCount() == 0)
    {
        QMessageBox::information(0,tr("警告"),tr("请先添加功能信息"),tr("确认"));
        return;
    }
    if(toolkit.getDevType())
    {
        QMap<QString,QVariant> devInfo = toolkit.readFormConfig("dev");
        QMap<QString, QVariant>::iterator iter = devInfo.begin();
        while (iter != devInfo.end())
        {
            if(iter.value().toMap().value("product").toMap().isEmpty())
            {
                QMessageBox::information(0,tr("警告"),tr("选择网关类型时，需要为从机设备添加产品信息(ProductKey和ProductSecert)，请检查从机设备设置"),tr("确认"));
                return;
            }
            iter++;
        }
    }
    QString filePath = QFileDialog::getExistingDirectory(0,tr("选择保存路径"),"");
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyyMMddhhmmsszzz");
    toolkit.ConfigTransformationToModule(filePath,current_date);
    toolkit.ConfigTransformationToCloud(filePath,current_date);
}

/*
    func:关于
*/
void MainWindow::on_action_about_triggered()
{
    about *page = new about();
    page->show();
}

/*
    func:反馈
*/
void MainWindow::on_action_feedback_triggered()
{
    feedback *page = new feedback();
    page->show();
}

/*
    func:使用文档
*/
void MainWindow::on_action_doc_triggered()
{
    if (ui->menu_5->title() == "语言")
    {
        QUrl url("https://iot-cloud-docs.quectelcn.com/tool/QthTools-develop/QthTools-DTU_Modbus/UserManual.html");
        QDesktopServices::openUrl(url);
    }
    else
    {
        QUrl url("https://iot-cloud-docs.quectelcn.com/en/tool/QthTools-develop/QthTools-DTU_Modbus/UserManual.html");
        QDesktopServices::openUrl(url);
    }
}

/*
    func:固件下载
*/
void MainWindow::on_action_down_triggered()
{
    fileDown *filedown = new fileDown();
    filedown->show();
}

void MainWindow::on_action_update_triggered()
{
    updateVersion->checkForUpdates();
}

void MainWindow::refresh_window()
{
    /* 删除所有控件 */
    if (listToolBox != NULL)
    {
        delete  listToolBox;
        listToolBox = NULL;
        listToolBox = new ToolBox();
    }

    /* 重新创建所有控件 */
    Form *productForm = new Form(tr("添加产品"));
    listToolBox->addWidget(tr("产品信息"), productForm);
    connect(productForm,SIGNAL(modifySignal()), this, SLOT(processAddProductModifySignal()));
    listToolBox->addWidget(tr("串口列表"), new Form(tr("添加串口")));
    listToolBox->addWidget(tr("功能列表"), new Form(tr("添加功能")));
    listLayout->addWidget(listToolBox);
    /* 删除所有tab窗口 */
    int tabCount = ui->tabWidget->count();
    for (int i = tabCount-2; i >= 0; i--)
    {
        QWidget *rmTabWidget = ui->tabWidget->widget(i);
        if (rmTabWidget != NULL)
        {
            delete rmTabWidget;
            rmTabWidget = NULL;
        }
    }

    /* 刷新tab右键动作 */
    ui->tabWidget->removeAction(action_modify);
    ui->tabWidget->removeAction(action_remove);
    if (action_modify != NULL)
    {
        delete action_modify;
        action_modify = NULL;
    }
    if (action_remove != NULL)
    {
        delete action_remove;
        action_remove = NULL;
    }
    if (table_widget_menu != NULL)
    {
        delete table_widget_menu;
        table_widget_menu = NULL;
    }
    table_widget_menu = new QMenu(ui->tabWidget);
    action_modify = new QAction(tr("修改"), ui->tabWidget);
    action_remove = new QAction(tr("删除"), ui->tabWidget);
    connect(action_modify, SIGNAL(triggered()), this, SLOT(slotActionModify()));
    connect(action_remove, SIGNAL(triggered()), this, SLOT(slotActionRemove()));
    table_widget_menu->addAction(action_modify);
    table_widget_menu->addAction(action_remove);

    /* 重新创建tab窗口 */
    QMap<QString,QVariant> info = toolkit.readFormConfig("dev");
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end())
    {
        /* 创建设备tab */
        addResultSlot(iter.key(),iter.value().toMap(),false);
        iter++;
    }
}

void MainWindow::languageChange(int mode)
{
    if(NULL != m_translator)
    {
        qApp->removeTranslator(m_translator);
        delete m_translator;
        m_translator = NULL;
    }

    m_translator = new QTranslator(this);
    if(0 == mode)
    {
        m_translator->load(":/chinese.qm");
        qApp->installTranslator(m_translator);
        QMap<QString,QVariant> language;
        language.insert("language", "chinese");
        toolkit.writeLanguageConfig("Language", language);
    }
    else if(1 == mode)
    {
        m_translator->load(":/english.qm");
        qApp->installTranslator(m_translator);
        QMap<QString,QVariant> language;
        language.insert("language", "english");
        toolkit.writeLanguageConfig("Language", language);
    }

    ui->retranslateUi(this);
    refresh_window();
}

void MainWindow::on_action_chinese_triggered()
{
    languageChange(0);
}

void MainWindow::on_action_english_triggered()
{
    languageChange(1);
}
