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
#ifndef TOOLPAGE_H
#define TOOLPAGE_H

#include <QWidget>

namespace Ui {
class ToolPage;
}

class QFormLayout;
class QLabel;
class ToolPage : public QWidget
{
    Q_OBJECT

public:
    explicit ToolPage(QWidget *parent = nullptr);
    ~ToolPage();

public slots:
    void addWidget(const QString &title, QWidget *widget);
    void expand();
    void collapse();

private slots:
    void onPushButtonFoldClicked();

private:
    Ui::ToolPage *ui;

    bool m_bIsExpanded;
    QLabel *m_pLabel;

};

#endif // TOOLPAGE_H
