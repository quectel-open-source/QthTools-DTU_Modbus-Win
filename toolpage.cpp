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
#include "ToolPage.h"
#include "ui_ToolPage.h"
#include <QDebug>
#include <QFormLayout>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QFile>

ToolPage::ToolPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolPage),
    m_bIsExpanded(true),
    m_pLabel(nullptr)
{
    ui->setupUi(this);

    ui->widgetContent->setAttribute(Qt::WA_StyledBackground);

    m_pLabel = new QLabel(this);
    m_pLabel->setFixedSize(20, 20);
    m_pLabel->setPixmap(QPixmap(":/img/down-arrow.png").scaled(m_pLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    QHBoxLayout *layout = new QHBoxLayout(ui->pushButtonFold);
    layout->setContentsMargins(0, 0, 5, 0);
    layout->addStretch(1);
    layout->addWidget(m_pLabel);

    QFile file(":/qss/toolpage.qss");
    if (file.open(QIODevice::ReadOnly)) {
        setStyleSheet(file.readAll());
    }
    file.close();

    connect(ui->pushButtonFold, &QPushButton::clicked, this, &ToolPage::onPushButtonFoldClicked);
}

ToolPage::~ToolPage()
{
    delete ui;
}

void ToolPage::addWidget(const QString &title, QWidget *widget)
{
    ui->pushButtonFold->setText(title);
    ui->verticalLayoutContent->addWidget(widget);
}

void ToolPage::expand()
{
    ui->widgetContent->show();
    m_bIsExpanded = true;

    m_pLabel->setPixmap(QPixmap(":/img/down-arrow.png").scaled(m_pLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void ToolPage::collapse()
{
    ui->widgetContent->hide();
    m_bIsExpanded = false;

    m_pLabel->setPixmap(QPixmap(":/img/left-arrow.png").scaled(m_pLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void ToolPage::onPushButtonFoldClicked()
{
    if (m_bIsExpanded) {
        collapse();
    } else {
        expand();
    }
}
