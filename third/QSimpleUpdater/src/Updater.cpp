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
/*
 * Copyright (c) 2014-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>
#include <QApplication>
#include <QJsonDocument>
#include <QDesktopServices>

#include "Updater.h"
#include "Downloader.h"
#include <QDebug>
#include <QLineEdit>
#include <QTextEdit>
#include "../include/updateConfig.h"

QWidget *updatewindow;

Updater::Updater()
{
   m_url = "";
   m_openUrl = "";
   m_changelog = "";
   m_downloadUrl = "";
   m_latestVersion = "";
   m_customAppcast = false;
   m_notifyOnUpdate = true;
   m_notifyOnFinish = false;
   m_updateAvailable = false;
   m_downloaderEnabled = true;
   m_moduleName = qApp->applicationName();
   m_moduleVersion = qApp->applicationVersion();
   m_mandatoryUpdate = false;

   m_downloader = new Downloader();
   connect(m_downloader,&Downloader::updateExeEvent,this,&Updater::updateExeEvent);
   m_downloader->setCancelInstall(false);
   m_manager = new QNetworkAccessManager();

#if defined Q_OS_WIN
   m_platform = "windows";
#elif defined Q_OS_MAC
   m_platform = "osx";
#elif defined Q_OS_LINUX
   m_platform = "linux";
#elif defined Q_OS_ANDROID
   m_platform = "android";
#elif defined Q_OS_IOS
   m_platform = "ios";
#endif

   setUserAgentString(QString("%1/%2 (Qt; QSimpleUpdater)").arg(qApp->applicationName(), qApp->applicationVersion()));

   connect(m_downloader, SIGNAL(downloadFinished(QString, QString)), this, SIGNAL(downloadFinished(QString, QString)));
   connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(onReply(QNetworkReply *)));
}

Updater::~Updater()
{
   delete m_downloader;
}

/**
 * Returns the URL of the update definitions file
 */
QString Updater::url() const
{
   return m_url;
}

/**
 * Returns the URL that the update definitions file wants us to open in
 * a web browser.
 *
 * \warning You should call \c checkForUpdates() before using this functio
 */
QString Updater::openUrl() const
{
   return m_openUrl;
}

/**
 * Returns the changelog defined by the update definitions file.
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::changelog() const
{
   return m_changelog;
}

/**
 * Returns the name of the module (if defined)
 */
QString Updater::moduleName() const
{
   return m_moduleName;
}

/**
 * Returns the platform key (be it system-set or user-set).
 * If you do not define a platform key, the system will assign the following
 * platform key:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 */
QString Updater::platformKey() const
{
   return m_platform;
}

/**
 * Returns the download URL defined by the update definitions file.
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::downloadUrl() const
{
   return m_downloadUrl;
}

/**
 * Returns the latest version defined by the update definitions file.
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::latestVersion() const
{
   return m_latestVersion;
}

/**
 * Returns the user-agent header used by the client when communicating
 * with the server through HTTP
 */
QString Updater::userAgentString() const
{
   return m_userAgentString;
}

/**
 * Returns the "local" version of the installed module
 */
QString Updater::moduleVersion() const
{
   return m_moduleVersion;
}

/**
 * Returns \c true if the updater should NOT interpret the downloaded appcast.
 * This is useful if you need to store more variables (or information) in the
 * JSON file or use another appcast format (e.g. XML)
 */
bool Updater::customAppcast() const
{
   return m_customAppcast;
}

/**
 * Returns \c true if the updater should notify the user when an update is
 * available.
 */
bool Updater::notifyOnUpdate() const
{
   return m_notifyOnUpdate;
}

/**
 * Returns \c true if the updater should notify the user when it finishes
 * checking for updates.
 *
 * \note If set to \c true, the \c Updater will notify the user even when there
 *       are no updates available (by congratulating him/her about being smart)
 */
bool Updater::notifyOnFinish() const
{
   return m_notifyOnFinish;
}

/**
 * Returns \c true if there the current update is mandatory.
 * \warning You should call \c checkForUpdates() before using this function
 */
bool Updater::mandatoryUpdate() const
{
   return m_mandatoryUpdate;
}

/**
 * Returns \c true if there is an update available.
 * \warning You should call \c checkForUpdates() before using this function
 */
bool Updater::updateAvailable() const
{
   return m_updateAvailable;
}

/**
 * Returns \c true if the integrated downloader is enabled.
 * \note If set to \c true, the \c Updater will open the downloader dialog if
 *       the user agrees to download the update.
 */
bool Updater::downloaderEnabled() const
{
   return m_downloaderEnabled;
}

/**
 * Returns \c true if the updater shall not intervene when the download has
 * finished (you can use the \c QSimpleUpdater signals to know when the
 * download is completed).
 */
bool Updater::useCustomInstallProcedures() const
{
   return m_downloader->useCustomInstallProcedures();
}

/**
 * Downloads and interpets the update definitions file referenced by the
 * \c url() function.
 */
void Updater::checkForUpdates()
{
    qDebug()<<__FUNCTION__<<url();
   QNetworkRequest request(url());
   request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

   if (!userAgentString().isEmpty())
   {
      request.setRawHeader("User-Agent", userAgentString().toUtf8());
   }
   m_downloader->setCancelInstall(false);
   m_manager->get(request);
}

/**
 * Changes the \c url in which the \c Updater can find the update definitions
 * file.
 */
void Updater::setUrl(const QString &url)
{
   m_url = url;
}

/**
 * Changes the module \a name.
 * \note The module name is used on the user prompts. If the module name is
 *       empty, then the prompts will show the name of the application.
 */
void Updater::setModuleName(const QString &name)
{
   m_moduleName = name;
}

/**
 * If \a notify is set to \c true, then the \c Updater will notify the user
 * when an update is available.
 */
void Updater::setNotifyOnUpdate(const bool notify)
{
   m_notifyOnUpdate = notify;
}

/**
 * If \a notify is set to \c true, then the \c Updater will notify the user
 * when it has finished interpreting the update definitions file.
 */
void Updater::setNotifyOnFinish(const bool notify)
{
   m_notifyOnFinish = notify;
}

/**
 * Changes the user agent string used to identify the client application
 * from the server in a HTTP session.
 *
 * By default, the user agent will co
 */
void Updater::setUserAgentString(const QString &agent)
{
   m_userAgentString = agent;
   m_downloader->setUserAgentString(agent);
}

/**
 * Changes the module \a version
 * \note The module version is used to compare the local and remote versions.
 *       If the \a version parameter is empty, then the \c Updater will use the
 *       application version (referenced by \c qApp)
 */
void Updater::setModuleVersion(const QString &version)
{
   m_moduleVersion = version;
}

/**
 * If the \a enabled parameter is set to \c true, the \c Updater will open the
 * integrated downloader if the user agrees to install the update (if any)
 */
void Updater::setDownloaderEnabled(const bool enabled)
{
   m_downloaderEnabled = enabled;
}

/**
 * Changes the platform key.
 * If the platform key is empty, then the system will use the following keys:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 */
void Updater::setPlatformKey(const QString &platformKey)
{
   m_platform = platformKey;
}

/**
 * If the \a customAppcast parameter is set to \c true, then the \c Updater
 * will not try to read the network reply from the server, instead, it will
 * emit the \c appcastDownloaded() signal, which allows the application to
 * read and interpret the appcast file by itself
 */
void Updater::setUseCustomAppcast(const bool customAppcast)
{
   m_customAppcast = customAppcast;
}

/**
 * If the \a custom parameter is set to \c true, the \c Updater will not try
 * to open the downloaded file. Use the signals fired by the \c QSimpleUpdater
 * to install the update from the downloaded file by yourself.
 */
void Updater::setUseCustomInstallProcedures(const bool custom)
{
   m_downloader->setUseCustomInstallProcedures(custom);
}

/**
 * If the \a mandatory_update is set to \c true, the \c Updater has to download and install the
 * update. If the user cancels or exits, the application will close
 */
void Updater::setMandatoryUpdate(const bool mandatory_update)
{
    m_mandatoryUpdate = mandatory_update;
}

void Updater::updateExeEvent(QString fileName)
{
    emit updateExeEventToUser(fileName);
}

static bool IsNumber(QString qstrSrc)
{
    QByteArray ba = qstrSrc.toLatin1();
    const char *s = ba.data();
    bool bret = true;
    while(*s)
    {
        if((*s>='0' && *s<='9') || *s=='.')
        {

        }
        else
        {
            bret = false;
            break;
        }
        s++;
    }
    return bret;
}

/**
 * Called when the download of the update definitions file is finished.
 */
void Updater::onReply(QNetworkReply *reply)
{
   /* Check if we need to redirect */
   QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
   if (!redirect.isEmpty())
   {
      setUrl(redirect.toString());
      checkForUpdates();
      return;
   }
   /* There was a network error */
   if (reply->error() != QNetworkReply::NoError)
   {
      setUpdateAvailable(false);
      emit checkingFinished(url());
      return;
   }
   /* The application wants to interpret the appcast by itself */
   if (customAppcast())
   {
      emit appcastDownloaded(url(), reply->readAll());
      emit checkingFinished(url());
      return;
   }
   /* Try to create a JSON document from downloaded data */
   QJsonDocument document = QJsonDocument::fromJson(reply->readAll());

   /* JSON is invalid */
   if (document.isNull())
   {
      setUpdateAvailable(false);
      emit checkingFinished(url());
      return;
   }

   /* Get the platform information */
   QJsonObject updates = document.object().value("updates").toObject();
   QJsonObject platform = updates.value(platformKey()).toObject();

   /* Get update information */
   if (platform.contains("open-url"))
   {
        m_openUrl = platform.value("open-url").toString();
   }
   m_changelog = platform.value("changelog").toString();
   m_latestVersion = platform.value("latest-version").toString();
   QString exeName = platform.value("name").toString();
   m_downloadUrl = DEFS_URL+QString("%1/%2_%3.exe").arg(exeName).arg(exeName).arg(m_latestVersion);
   qDebug()<<"m_downloadUrl:"<<m_downloadUrl;
   if (platform.contains("force"))
      m_mandatoryUpdate = platform.value("force").toBool();

   /* Compare latest and current version */
   bool compareResult = false;
   if(IsNumber(latestVersion()) && IsNumber(moduleVersion()))
   {
        compareResult = compare(latestVersion(), moduleVersion());
   }
   else
   {
       if(0 != QString::compare(latestVersion(), moduleVersion()))
       {
           compareResult = true;
       }
   }
   setUpdateAvailable(compareResult);
   emit checkingFinished(url());
}

/**
 * Prompts the user based on the value of the \a available parameter and the
 * settings of this instance of the \c Updater class.
 */
void Updater::setUpdateAvailable(const bool available)
{
   m_updateAvailable = available;

   QMessageBox box;
   box.setTextFormat(Qt::RichText);
   box.setIcon(QMessageBox::Information);

   qDebug()<<"m_mandatoryUpdate:"<<m_mandatoryUpdate;
   updatewindow = new QWidget();
   updatewindow->setWindowFlags(Qt::WindowCloseButtonHint);
   updatewindow->setWindowTitle(tr("更新"));
   if (updateAvailable()) // && (notifyOnUpdate() || notifyOnFinish())
   {
       QString text = tr("是否立即下载？");//Would you like to download the update now?
       if (m_mandatoryUpdate)
       {
        text = tr("是否立即下载？当前更新为强制更新，如取消，程序将无法使用。");//"Would you like to download the update now? This is a mandatory update, exiting now will close the application"
       }

       updatewindow->setStyleSheet("background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 255), stop:0.79 rgba(255, 255, 255, 255), stop:0.79001 rgba(240, 240, 240, 240), stop:1 rgba(240, 240, 240, 240));");
       QWidget *Wbutton = new QWidget();
       Wbutton->setStyleSheet("background-color:rgb(240, 240, 240);");
       QHBoxLayout *qbox = new QHBoxLayout(updatewindow);
       QHBoxLayout *button = new QHBoxLayout(Wbutton);
       QLabel *text1, *text2;
       QTextEdit *info;
       QPushButton *Yes_button, *No_button;
       Yes_button = new QPushButton(tr("确定"));
       Yes_button->setStyleSheet("font:9pt \"微软雅黑\";border-style:solid;border-width:2px;border-color:rgb(80, 108, 244);");
       Yes_button->setMinimumSize(75, 25);
       No_button = new QPushButton(tr("取消"));
       No_button->setMinimumSize(75, 25);
       No_button->setStyleSheet("font:9pt \"微软雅黑\";border-style:solid;border-width:1px;border-color:rgb(210, 210, 210);");
       button->addStretch();
       button->addWidget(Yes_button);
       button->addWidget(No_button);
       text1 = new QLabel();
       text2 = new QLabel();
       info = new QTextEdit();
       info->setReadOnly(true);
       info->setFrameStyle(QFrame::NoFrame);
       text1->setStyleSheet("font:12pt \"微软雅黑\";background-color:rgb(255, 255, 255);");
       text2->setStyleSheet("font:12pt \"微软雅黑\";background-color:rgb(255, 255, 255);");
       info->setStyleSheet("font:12pt \"微软雅黑\";background-color:rgb(255, 255, 255);border-style:solid;border-width:2px;border-color:rgb(240, 240, 240);");
       qbox->addWidget(text1, 1);
       qbox->addWidget(info, 6);
       qbox->addWidget(text2, 1);
       qbox->addWidget(Wbutton, 2);
       qbox->setDirection(QBoxLayout::TopToBottom);
       text1->setText(tr("%2 的版本%1已发布！").arg(latestVersion()).arg(moduleName()));//Version %1 of %2 has been released!
       text2->setText(text);
       QString title = QString(changelog().isEmpty()?"null":changelog());
       info->setText(title);

       updatewindow->setMaximumSize(500, 300);
       updatewindow->setMinimumSize(500, 300);
       updatewindow->setWindowModality(Qt::ApplicationModal);
       QObject::connect(Yes_button,&QPushButton::clicked,this,&Updater::on_action_yes_triggered);
       QObject::connect(No_button,&QPushButton::clicked,this,&Updater::on_action_no_triggered);
       updatewindow->show();
   }
   else if(notifyOnFinish())//&& updateAvailable())
   {
      updatewindow->setStyleSheet("background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 255), stop:0.51 rgba(255, 255, 255, 255), stop:0.51001 rgba(240, 240, 240, 240), stop:1 rgba(240, 240, 240, 240));");
      QLabel *text = new QLabel();
      QWidget *Wbutton = new QWidget();
      Wbutton->setStyleSheet("background-color:rgb(240, 240, 240);");
      QHBoxLayout *button = new QHBoxLayout(Wbutton);
      QPushButton *close_button = new QPushButton(tr("关闭"));
      close_button->setStyleSheet("font:9pt \"微软雅黑\";border-style:solid;border-width:1px;border-color:rgb(210, 210, 210);");
      close_button->setMinimumSize(75, 25);
      button->addStretch();
      button->addWidget(close_button);
      QHBoxLayout *qbox = new QHBoxLayout(updatewindow);
      qbox->addWidget(text, 3);
      qbox->addWidget(Wbutton, 2);
      qbox->setDirection(QBoxLayout::TopToBottom);
      text->setText(tr("无更新：当前为最新版本"));
      text->setStyleSheet("font:12pt \"微软雅黑\";background-color:rgb(255, 255, 255);");
      if (close_button->text() == "关闭")
      {
          Wbutton->setMaximumSize(280, 38);
          Wbutton->setMinimumSize(280, 38);
          updatewindow->setMaximumSize(300, 100);
          updatewindow->setMinimumSize(300, 100);
      }
      else
      {
          Wbutton->setMaximumSize(360, 38);
          Wbutton->setMinimumSize(360, 38);
          updatewindow->setMaximumSize(380, 100);
          updatewindow->setMinimumSize(380, 100);
      }
      updatewindow->setWindowModality(Qt::ApplicationModal);
      QObject::connect(close_button,&QPushButton::clicked,this,&Updater::on_action_close_triggered);
      updatewindow->show();
   }
}

void Updater::on_action_yes_triggered()
{
    if (!openUrl().isEmpty())
    QDesktopServices::openUrl(QUrl(openUrl()));

    else if (downloaderEnabled())
    {
        m_downloader->setWindowTitle(tr("更新"));
        m_downloader->setUrlId(url());
        m_downloader->setFileName(downloadUrl().split("/").last());
        m_downloader->setMandatoryUpdate(m_mandatoryUpdate);
        m_downloader->startDownload(QUrl(downloadUrl()));
    }
    else
    {
        QDesktopServices::openUrl(QUrl(downloadUrl()));
    }
    updatewindow->close();
}

void Updater::on_action_no_triggered()
{
    if (m_mandatoryUpdate)
    {
        QApplication::quit();
    }
    updatewindow->close();
}

void Updater::on_action_close_triggered()
{
    updatewindow->close();
}


/**
 * Compares the two version strings (\a x and \a y).
 *     - If \a x is greater than \y, this function returns \c true.
 *     - If \a y is greater than \x, this function returns \c false.
 *     - If both versions are the same, this function returns \c false.
 */
bool Updater::compare(const QString &x, const QString &y)
{
   QStringList versionsX = x.split(".");
   QStringList versionsY = y.split(".");

   int count = qMin(versionsX.count(), versionsY.count());

   for (int i = 0; i < count; ++i)
   {
      int a = QString(versionsX.at(i)).toInt();
      int b = QString(versionsY.at(i)).toInt();

      if (a > b)
         return true;

      else if (b > a)
         return false;
   }

   return versionsY.count() < versionsX.count();
}

#if QSU_INCLUDE_MOC
#   include "moc_Updater.cpp"
#endif
