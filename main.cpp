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
#include "mainwindow.h"
#include <QApplication>
#include <QMutex>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QStandardPaths>
#include "QDir"
#include <QDebug>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QMessageBox>
#include "ccrashstack.h"
#include <qtranslator.h>

QFile *file = nullptr;
QTranslator *m_translator = NULL;

void printfLogToFile(QtMsgType type,const QMessageLogContext &context,const QString &msg)
{
    Q_UNUSED(type);
    Q_UNUSED(context);
    static QMutex mutex;
    mutex.lock();
    QString current_date_time = QDateTime::currentDateTime().toString("MM-dd hh:mm:ss zzz");
    QString message = QString("[%1] %2 %3").arg(current_date_time).arg(msg).arg("\r\n");
    QTextStream text_stream(file);
    text_stream << message;
    file->flush();
    mutex.unlock();
}

long __stdcall   callback(_EXCEPTION_POINTERS*   excp)
{
    CCrashStack crashStack(excp);
    QString sCrashInfo = crashStack.GetExceptionInfo();
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyyMMddhhmmsszzz");
    QString sFileName = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/dump_"+current_date+".log";

    QFile file(sFileName);
    if (file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        file.write(sCrashInfo.toUtf8());
        file.close();
    }

    return   EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char *argv[])
{

    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyyMMddhhmmsszzz");
    QString logFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/log_"+current_date+".txt";
    file = new QFile(logFile);
    file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Truncate);
    qInstallMessageHandler(printfLogToFile);

    SetUnhandledExceptionFilter(callback);
    QApplication a(argc, argv);
    QSystemSemaphore semaphore("SingleAppTest2Semaphore", 1);
    semaphore.acquire();

#ifndef Q_OS_WIN32
    // 在linux / unix 程序异常结束共享内存不会回收
    // 在这里需要提供释放内存的接口，就是在程序运行的时候如果有这段内存 先清除掉
    QSharedMemory nix_fix_shared_memory("SingleAppTest2");
    if (nix_fix_shared_memory.attach())
    {undefined
        nix_fix_shared_memory.detach();
    }
#endif
    QSharedMemory sharedMemory("QthTools-DTU_Modbus-Win");
    bool isRunning = false;
    if (sharedMemory.attach())
    {
        isRunning = true;
    }
    else
    {
        sharedMemory.create(1);
        isRunning = false;
    }
    semaphore.release();

    // 如果您已经运行了应用程序的一个实例，那么我们将通知用户。
    if (isRunning)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(QObject::tr("已有重复程序处于运行中状态！"));
        msgBox.exec();
        return 1;
    }

    QLocale locale;
    if(NULL != m_translator)
    {
        qApp->removeTranslator(m_translator);
        delete m_translator;
        m_translator = NULL;

    }
    m_translator = new QTranslator();

    if(locale.language() == QLocale::Chinese)
    {
        m_translator->load(":/chinese.qm");
        qApp->installTranslator(m_translator);
    }
    else
    {
        m_translator->load(":/english.qm");
        qApp->installTranslator(m_translator);
    }

    setbuf(stdout, NULL);//让printf立即输出
    MainWindow w;
    w.show();
    return a.exec();
}
