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
#include "mydoublespinbox.h"
#include <QKeyEvent>
#include <QDebug>
#include <QLineEdit>
MyDoubleSpinBox::MyDoubleSpinBox(QWidget *parent) : QDoubleSpinBox(parent)
{

}

void MyDoubleSpinBox::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_0 && this->lineEdit()->selectionLength() != text().length())
    {
        QString text = this->lineEdit()->text();
        int cursorPosition = this->lineEdit()->cursorPosition();
        if(text.startsWith('-'))
        {
            text = text.mid(1);
            cursorPosition = cursorPosition - 1;
        }
        if(text.length())
        {
            QStringList textList = text.split(".");
            qDebug()<<textList<<cursorPosition;
            if(cursorPosition <= QString(textList.at(0)).length())
            {
                QString cursorText = textList.at(0);
                if(cursorText.length() >= 6)
                {
                    return;
                }
                else if(cursorText.toInt() == 0)
                {
                    return;
                }

            }
        }
    }
    QDoubleSpinBox::keyPressEvent(event);
}

