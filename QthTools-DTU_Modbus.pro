QT       += core gui
QT       += serialport

TRANSLATIONS += \
    english.ts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
include ($$PWD/third/QSimpleUpdater/QSimpleUpdater.pri)
CONFIG += c++11

TARGET = QthTools-DTU_Modbus

QMAKE_CFLAGS_RELEASE += -g
QMAKE_CXXFLAGS_RELEASE += -g
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_LFLAGS_RELEASE = -mthreads -W


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Form.cpp \
    ToolBox.cpp \
    add/adddev.cpp \
    add/addproduct.cpp \
    add/addregister.cpp \
    add/addregisterform.cpp \
    add/addttlv.cpp \
    add/addttlvbool.cpp \
    add/addttlvbyte.cpp \
    add/addttlvenum.cpp \
    add/addttlvnum.cpp \
    add/adduart.cpp \
    ccrashstack.cpp \
    devtab.cpp \
    down/filedown.cpp \
    down/serial.cpp \
    help/about.cpp \
    help/feedback.cpp \
    help/userdoc.cpp \
    jsonFile.cpp \
    main.cpp \
    mainwindow.cpp \
    mydoublespinbox.cpp \
    myspinbox.cpp \
    preqtablewidget.cpp \
    quecthing/ql_iotTtlv.c \
    quecthing/quos_SupportTool.c \
    quecthing/quos_cjson.c \
    quecthing/quos_twll.c \
    toolkit.cpp \
    toolpage.cpp

HEADERS += \
    Form.h \
    ToolBox.h \
    ToolPage.h \
    add/adddev.h \
    add/addproduct.h \
    add/addregister.h \
    add/addregisterform.h \
    add/addttlv.h \
    add/addttlvbool.h \
    add/addttlvbyte.h \
    add/addttlvenum.h \
    add/addttlvnum.h \
    add/adduart.h \
    ccrashstack.h \
    devtab.h \
    down/filedown.h \
    down/serial.h \
    help/about.h \
    help/feedback.h \
    help/userdoc.h \
    jsonFile.h \
    mainwindow.h \
    mydoublespinbox.h \
    myspinbox.h \
    preqtablewidget.h \
    quecthing/Qhal_types.h \
    quecthing/ql_iotTtlv.h \
    quecthing/quos_SupportTool.h \
    quecthing/quos_cjson.h \
    quecthing/quos_twll.h \
    toolkit.h

FORMS += \
    Form.ui \
    ToolBox.ui \
    ToolPage.ui \
    add/adddev.ui \
    add/addproduct.ui \
    add/addregister.ui \
    add/addregisterform.ui \
    add/addttlv.ui \
    add/addttlvbool.ui \
    add/addttlvbyte.ui \
    add/addttlvenum.ui \
    add/addttlvnum.ui \
    add/adduart.ui \
    devtab.ui \
    down/filedown.ui \
    help/about.ui \
    help/feedback.ui \
    help/userdoc.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    doc.qrc \
    icon.qrc \
    img.qrc \
    qss.qrc

RC_FILE += icon/quectel.rc

TRANSLATIONS += \
    chinese.ts \
    english.ts
