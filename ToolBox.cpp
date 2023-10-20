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
#include "ToolBox.h"
#include "ui_ToolBox.h"
#include "ToolPage.h"
#include <QVBoxLayout>

ToolBox::ToolBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolBox),
    m_pContentVBoxLayout(nullptr)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    QWidget *widget = new QWidget(this);
    m_pContentVBoxLayout = new QVBoxLayout(widget);
    m_pContentVBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentVBoxLayout->setSpacing(2);
    m_pContentVBoxLayout->setAlignment(Qt::AlignTop);
    ui->scrollArea->setWidget(widget);
}

ToolBox::~ToolBox()
{
    delete ui;
}

void ToolBox::addWidget(const QString &title, QWidget *widget)
{
    ToolPage *page = new ToolPage(this);
    page->addWidget(title, widget);

    if (title == tr("功能列表"))
    {
        page->hide();
    }
    m_pContentVBoxLayout->addWidget(page);
}


void ToolBox::delWidget(int index)
{
    QLayoutItem *item = m_pContentVBoxLayout->takeAt(index);
    delete item->widget();
}

void ToolBox::delWidget(QWidget *widget)
{
    m_pContentVBoxLayout->removeWidget(widget);
}

QWidget *ToolBox::getWidget(int index)
{
    QLayoutItem *item = m_pContentVBoxLayout->itemAt(index);
    return item->widget();
}

QWidget *ToolBox::getWidget(const QString &title)
{
    for (int i = 0; i < m_pContentVBoxLayout->count(); i++)
    {
        if(m_pContentVBoxLayout->itemAt(i)->widget()->objectName() == title)
        {
            return m_pContentVBoxLayout->itemAt(i)->widget();
        }
    }
    return nullptr;
}
