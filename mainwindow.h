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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include "toolkit.h"
#include "ToolBox.h"
#include "third/QSimpleUpdater/toolupdate.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_tabWidget_tabBarClicked(int index);
    void addResultSlot(QString,QMap<QString,QVariant>,bool isLoad);
    void modResultSlot(QString oldName,QString newName,QMap<QString,QVariant> info);
    void tableContextMenuRequested(const QPoint &pos);
    void slotActionRemove();
    void slotActionModify();
    void unselectSlot();
    void on_action_outputProject_triggered();
    void on_action_inputProject_triggered();
    void on_action_output_triggered();
    void on_action_about_triggered();
    void on_action_feedback_triggered();
    void on_action_doc_triggered();
    void on_action_clearProject_triggered();
    void on_action_down_triggered();
    void on_action_update_triggered();
    void on_action_chinese_triggered();
    void on_action_english_triggered();
    void languageChange(int mode);
    void refresh_window();
    void processAddProductModifySignal();
    bool checkFeatureLabel();
    bool checkMultiple();

private:
    Ui::MainWindow *ui;
    QPoint actionPos;
    QMenu *table_widget_menu;
    toolKit toolkit;
    ToolBox *listToolBox;
    int currTabIndex;
    toolUpdate *updateVersion;
    QAction *action_modify;
    QAction *action_remove;
    QGridLayout *listLayout;
};

extern QTranslator *m_translator;

#endif // MAINWINDOW_H
