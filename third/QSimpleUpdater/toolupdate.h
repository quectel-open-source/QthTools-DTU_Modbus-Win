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
#ifndef TOOLUPDATE_H
#define TOOLUPDATE_H

#include <QWidget>
#include "third/QSimpleUpdater/src/Updater.h"
#include "third/QSimpleUpdater/src/Downloader.h"
#include "third/QSimpleUpdater/include/QSimpleUpdater.h"


namespace Ui {
class toolUpdate;
}

class toolUpdate : public QWidget
{
    Q_OBJECT

public:
    explicit toolUpdate(QWidget *parent = nullptr);
    ~toolUpdate();
public slots:
    void checkForUpdates();
signals:
//    void stopSubThread();
private slots:
    void resetFields();
    void updateChangelog(const QString &url);
    void displayAppcast(const QString &url, const QByteArray &reply);
    void updataExeEvent(QString newSoftName);
private:
    Ui::toolUpdate *ui;
//    QNetworkAccessManager *manager;
    QSimpleUpdater *m_updater;
    bool firstUse = false;
};

#endif // TOOLUPDATE_H
