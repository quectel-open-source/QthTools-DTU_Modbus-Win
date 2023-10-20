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
#include "devtab.h"
#include "ui_devtab.h"
#include "add/addregister.h"
#include "add/addregisterform.h"
#include <QDebug>

devTab::devTab(QMap<QString,QVariant> info,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::devTab)
{
    ui->setupUi(this);

    QMap<QString,QVariant> info01;
    if(info.find("线圈") != info.end())
    {
        info01 = info.find("线圈").value().toMap();
    }
    ui->tabWidget->setCurrentWidget(ui->tab_0x01);
    QGridLayout *layout_0x01 = new QGridLayout(ui->tab_0x01);
    layout_0x01->setContentsMargins(0,0,0,0);
    addRegisterForm *page_0x01 = new addRegisterForm(info01,ui->tab_0x01);
    layout_0x01->addWidget(page_0x01);


    QMap<QString,QVariant> info03;
    if(info.find("保持寄存器") != info.end())
    {
        info03 = info.find("保持寄存器").value().toMap();
    }
    ui->tabWidget->setCurrentWidget(ui->tab_0x03);
    QGridLayout *layout_0x03 = new QGridLayout(ui->tab_0x03);
    layout_0x03->setContentsMargins(0,0,0,0);
    addRegisterForm *page_0x03 = new addRegisterForm(info03,ui->tab_0x03);
    layout_0x03->addWidget(page_0x03);

    QMap<QString,QVariant> info04;
    if(info.find("输入寄存器") != info.end())
    {
        info04 = info.find("输入寄存器").value().toMap();
    }
    ui->tabWidget->setCurrentWidget(ui->tab_0x04);
    QGridLayout *layout_0x04 = new QGridLayout(ui->tab_0x04);
    layout_0x04->setContentsMargins(0,0,0,0);
    addRegisterForm *page_0x04 = new addRegisterForm(info04,ui->tab_0x04);
    layout_0x04->addWidget(page_0x04);

    QMap<QString,QVariant> info02;
    if(info.find("离散量") != info.end())
    {
        info02 = info.find("离散量").value().toMap();
    }
    ui->tabWidget->setCurrentWidget(ui->tab_0x02);
    QGridLayout *layout_0x02 = new QGridLayout(ui->tab_0x02);
    layout_0x02->setContentsMargins(0,0,0,0);
    addRegisterForm *page_0x02 = new addRegisterForm(info02,ui->tab_0x02);
    layout_0x02->addWidget(page_0x02);

    ui->tabWidget->setCurrentWidget(ui->tab_0x01);
}

devTab::~devTab()
{
    delete ui;
}

