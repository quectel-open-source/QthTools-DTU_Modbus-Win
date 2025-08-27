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
#ifndef ADDTTLVNUM_H
#define ADDTTLVNUM_H

#include <QWidget>
#include <QVariant>

namespace Ui {
class addTtlvNum;
}

class addTtlvNum : public QWidget
{
    Q_OBJECT

public:
    explicit addTtlvNum(QMap<QString,QVariant> info,QWidget *parent = nullptr);
    ~addTtlvNum();

private slots:
    void on_checkBox_event_clicked(bool checked);
    void on_comboBox_numType_currentIndexChanged(const QString &arg1);
    void on_doubleSpinBox_multiple_valueChanged(double arg1);
    void on_doubleSpinBox_increment_valueChanged(double arg1);
    void on_register_value_min_valueChanged(double arg1);

    void numericalLimits();
    bool stringIsInt(QString value);    
    void on_FlagCondition1_clicked(bool checked);

    void on_FlagCondition2_clicked(bool checked);

private:
    Ui::addTtlvNum *ui;
};

#endif // ADDTTLVNUM_H
