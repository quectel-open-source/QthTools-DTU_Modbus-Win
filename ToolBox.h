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
#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QWidget>

namespace Ui {
class ToolBox;
}

class QVBoxLayout;
class ToolBox : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBox(QWidget *parent = nullptr);
    ~ToolBox();

    void addWidget(const QString &title, QWidget *widget);
    void delWidget(int index);
    void delWidget(QWidget *widget);
    QWidget *getWidget(int index);
    QWidget *getWidget(const QString &title);

private:
    Ui::ToolBox *ui;
    QVBoxLayout *m_pContentVBoxLayout;


};

#endif // TOOLBOX_H
