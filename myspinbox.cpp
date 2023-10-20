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
#include "myspinbox.h"
#include <QKeyEvent>
#include <QDebug>
#include <QLineEdit>
MySpinBox::MySpinBox(QWidget *parent) : QSpinBox(parent)
{
}

void MySpinBox::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_0)
    {
        QString cursorText = text().left(this->lineEdit()->cursorPosition());
        if(cursorText.startsWith('-'))
        {
            cursorText = cursorText.mid(1);
        }
        qDebug()<<this->lineEdit()->cursorPosition()<<text()<<cursorText;
        if(this->lineEdit()->selectionLength() == text().length())
        {
            if(minimum() > 0)
            {
                return;
            }
        }
        else if(this->lineEdit()->cursorPosition() == 0)
        {
            if(minimum() > 0)
            {
                return;
            }
            else if(text().length() != 0)
            {
                return;
            }
        }
        else if(cursorText.toInt() == 0)
        {
            return;
        }
    }
    QSpinBox::keyPressEvent(event);
}

