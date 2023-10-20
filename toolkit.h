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
#ifndef TOOLKIT_H
#define TOOLKIT_H

#include <QObject>
#include <QLayout>

class toolKit
{
public:
    toolKit();
    void clearLayout(QLayout *layout);
    QObject *findParent(QObject *obj,QString className,QString name);
    void writeFormConfig(QString name,QMap<QString,QVariant> info);
    void writeLanguageConfig(QString name,QMap<QString,QVariant> info);
    void removeFormConfig(QString name);
    void removeSubDevInfoConfig(void);
    bool getDevType(void);
    QMap<QString,QVariant> readFormConfig(QString name);
    QMap<QString,QVariant> readLanguageConfig(QString name);
    bool readNewConfigFile();
    bool ConfigTransformationToModule(QString filePath,QString current_date);
    bool ConfigTransformationToCloud(QString filePath,QString current_date);
    void addPosInfo(QString path,int bit,int row,int column,int span);
};

#endif // TOOLKIT_H
