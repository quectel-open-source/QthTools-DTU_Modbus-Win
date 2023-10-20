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
#include "toolkit.h"
#include <QWidget>
#include "jsonFile.h"
#include <QStandardPaths>
#include <QJsonArray>
#include <QDateTime>
#include <QMessageBox>
#include "QFileInfo"
#include "QFile"
#include "QDir"

#ifdef __cplusplus
extern "C" {
#include "quecthing/ql_iotTtlv.h"
#include "quecthing/quos_cjson.h"
#include "quecthing/quos_SupportTool.h"
#endif

toolKit::toolKit()
{

}

/*
    func:寻找父类
*/
QObject *toolKit::findParent(QObject *obj,QString className,QString name)
{
    if(nullptr == obj)
    {
        return NULL;
    }
    QObject *parentObj = obj;
    while ((parentObj = parentObj->parent()))
    {
//        qDebug()<<parentObj->objectName()<<parentObj->metaObject()->className();
        if((className.isEmpty() || className == parentObj->metaObject()->className()) && (name.isEmpty() || name == parentObj->objectName()))
        {
            return parentObj;
        }
    }
    return NULL;
}


/*
    func:清除布局
*/
void toolKit::clearLayout(QLayout *layout)
{
    QLayoutItem *item;
    while((item = layout->takeAt(0)) != 0)
    {
        /* 删除widget */
        if(item->widget())
        {
            delete item->widget();
        }
        /* 删除子布局 */
        QLayout *childLayout = item->layout();
        if(childLayout)
        {
            clearLayout(childLayout);
        }
        delete item;
    }
}

/*
    func:写配置
*/
void toolKit::writeFormConfig(QString name,QMap<QString,QVariant> info)
{
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
    jsonFile *configFile = new jsonFile(configfile);
    configFile->readFile();
    QJsonObject configObj;
    QMap<QString, QVariant>::iterator iter = info.begin();
//    qDebug()<<"写配置"<<name<<info;
    while (iter != info.end())
    {
//        qDebug()<<"写配置2"<<iter.key()<<iter.value()<<iter.value().toJsonValue();
        configObj.insert(iter.key(),iter.value().toJsonValue());
        iter++;
    }
    configFile->writeJsonKeyValue(name,configObj);
    configFile->writeFile();
    delete configFile;
    configFile = NULL;
}

/*
    func:读配置
*/
QMap<QString,QVariant> toolKit::readFormConfig(QString name)
{
    QMap<QString,QVariant> info;
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
    jsonFile *configFile = new jsonFile(configfile);
    QJsonValue jsonvalue;;
    configFile->readFile();
    configFile->readJsonKeyValue(name,&jsonvalue);
    if(jsonvalue.isObject())
    {
        QJsonObject configObj = jsonvalue.toObject();
        QJsonObject::const_iterator it = configObj.constBegin();
        QJsonObject::const_iterator end = configObj.constEnd();
        while(it != end)
        {
//            qDebug()<<"读配置"<<name<<it.key()<<it.value();
            info.insert(it.key(),it.value().toVariant());
            it++;
        }
    }
    delete configFile;
    configFile = NULL;
    return info;
}

/*
    func:读语言配置
*/

QMap<QString,QVariant> toolKit::readLanguageConfig(QString name)
{
    QMap<QString,QVariant> info;
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/basicConfig.ini";
    jsonFile *configFile = new jsonFile(configfile);
    QJsonValue jsonvalue;;
    configFile->readFile();
    configFile->readJsonKeyValue(name,&jsonvalue);
    if(jsonvalue.isObject())
    {
        QJsonObject configObj = jsonvalue.toObject();
        QJsonObject::const_iterator it = configObj.constBegin();
        QJsonObject::const_iterator end = configObj.constEnd();
        while(it != end)
        {
//            qDebug()<<"读配置"<<name<<it.key()<<it.value();
            info.insert(it.key(),it.value().toVariant());
            it++;
        }
    }
    delete configFile;
    configFile = NULL;
    return info;
}

/*
    func:写语言配置
*/
void toolKit::writeLanguageConfig(QString name,QMap<QString,QVariant> info)
{
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/basicConfig.ini";
    jsonFile *configFile = new jsonFile(configfile);
    configFile->readFile();
    QJsonObject configObj;
    QMap<QString, QVariant>::iterator iter = info.begin();
    while (iter != info.end())
    {
        configObj.insert(iter.key(),iter.value().toJsonValue());
        iter++;
    }
    configFile->writeJsonKeyValue(name,configObj);
    configFile->writeFile();
    delete configFile;
    configFile = NULL;
}

/*
    func:检查导入文件格式
*/
bool toolKit::readNewConfigFile()
{
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
    jsonFile *configFile = new jsonFile(configfile);
    QJsonValue jsonvalue;;
    configFile->readFile();
    configFile->readJsonKeyValue("product",&jsonvalue);
    bool isSingleProduct = false;
    if(jsonvalue.isObject())
    {
        QJsonObject configObj = jsonvalue.toObject();
        QJsonObject::const_iterator it = configObj.constBegin();
        QJsonObject::const_iterator end = configObj.constEnd();
        while(it != end)
        {
            if (it.value().toObject().value("devType").toString() == "网关版")
            {
                delete configFile;
                configFile = NULL;
                return true;
            }
            else
            {
                isSingleProduct = true;
                break;
            }
            it++;
        }
    }
    configFile->readJsonKeyValue("dev",&jsonvalue);
    int devMun = 0;
    if(jsonvalue.isObject())
    {
        QJsonObject configObj = jsonvalue.toObject();
        QJsonObject::const_iterator it = configObj.constBegin();
        QJsonObject::const_iterator end = configObj.constEnd();
        while(it != end)
        {
            devMun ++;
            qDebug()<<"读配置"<<it.key()<<it.value();
            if (isSingleProduct == true)
            {
                if (devMun > 1)
                {
                    qDebug()<<"直连版下存在多个设备";
                    delete configFile;
                    configFile = NULL;
                    return false;
                }
                else
                {
                    QJsonObject addrObj = it.value().toObject().value("addr").toObject();
                    QJsonObject::const_iterator addrit = addrObj.constBegin();
                    QJsonObject::const_iterator addrend = addrObj.constEnd();
                    while(addrit != addrend)
                    {
                        if(addrit.value().toArray().count() > 1)
                        {
                            qDebug()<<"直连版设备串口下存在多个从机设备";
                            delete configFile;
                            configFile = NULL;
                            return false;
                        }
                        addrit++;
                    }
                }
            }
            it++;
        }
    }
    delete configFile;
    configFile = NULL;
    return true;
}
/*
    func:删除配置
*/
void toolKit::removeFormConfig(QString name)
{
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
    jsonFile *configFile = new jsonFile(configfile);
    configFile->readFile();
    configFile->removeJsonKeyValue(name);
    configFile->writeFile();
    delete configFile;
    configFile = NULL;
}

/*
    func:添加新的功能模型信息
*/
void toolKit::addPosInfo(QString path,int bit,int row,int column,int span)
{
    QJsonArray posArray;
    QMap<QString,QVariant> posInfo = readFormConfig(path);
    posArray = posInfo.value("pos").toJsonArray();
    QJsonObject posObj;
    posObj.insert("bit",bit);
    posObj.insert("row",row);
    posObj.insert("column",column);
    posObj.insert("span",span);
    for (int iPos = 0; iPos < posArray.count(); iPos++)
    {
        QJsonObject posObj = posArray.at(iPos).toObject();
        if(posObj.value("row").toInt() == row && (-1 == bit || posObj.value("bit").toInt() == bit ))
        {
            posArray.removeAt(iPos);
        }
    }
    posArray.append(posObj);
    posInfo.insert("pos",posArray);
    writeFormConfig(path,posInfo);
}

/*
    func:删除子设备配置
*/
void toolKit::removeSubDevInfoConfig(void)
{
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
    jsonFile *configFile = new jsonFile(configfile);
    configFile->readFile();
    QJsonValue devValue;
    configFile->readJsonKeyValue("dev",&devValue);
    if(devValue.isObject())
    {
        QJsonObject devObj = devValue.toObject();
        QJsonObject::Iterator devIt;
        for(devIt=devObj.begin();devIt!=devObj.end();devIt++)
        {
            QString devName = devIt.key();
            configFile->removeJsonKeyValue("dev/"+devName+"/product");
        }
    }
    configFile->writeFile();
    delete configFile;
    configFile = NULL;
}


bool toolKit::getDevType(void)
{
    QMap<QString,QVariant> info = readFormConfig("product");
    QMap<QString, QVariant>::iterator iter = info.begin();
    QMap<QString,QVariant> productInfo = iter.value().toMap();
    if(productInfo.find("devType") != productInfo.end())
    {
        QString devType =  iter.value().toMap().find("devType").value().toString();
        if(devType == "网关版")
        {
            qDebug()<<"网关版";
            return true;
        }
    }
    qDebug()<<"直连版";
    return false;
}

#define QIOT_MB_FILE_HEAD  0XAA55AA55
typedef struct
{
    quint32_t head;
    quint8_t crc;
    quint16_t len;
    quint8_t buf[1]; /* 数据开始位置 */
} SafeFlashHeadData_t;
/*
    func:根据配置文件转换成模组烧写的modbus配置文件
*/
bool toolKit::ConfigTransformationToModule(QString filePath,QString current_date)
{
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
    jsonFile *configFile = new jsonFile(configfile);
    configFile->readFile();
    QJsonObject moduleJson;
    moduleJson.insert(QString::number(QIOT_MBID_VER),2);
    QJsonValue productValue;

    QString fileName;
    /* 产品信息 */
    configFile->readJsonKeyValue("product",&productValue);
    if(productValue.isObject())
    {
        QJsonObject configObj = productValue.toObject();
        QJsonObject::Iterator it;
        for(it=configObj.begin();it!=configObj.end();it++)
        {
            QString name = it.key();
            QJsonObject productObj = it.value().toObject();
            QString productKey = productObj.value("productKey").toString();
            QString productSecret = productObj.value("productSecret").toString();
            QString report = productObj.value("report").toString();
            QString devType = productObj.value("devType").toString();
            if(report == "变化上报")
            {
                moduleJson.insert(QString::number(QIOT_MBID_REPORT),0);
            }
            else
            {
                moduleJson.insert(QString::number(QIOT_MBID_REPORT),1);
            }
            moduleJson.insert(QString::number(QIOT_MBID_PK),productKey);
            moduleJson.insert(QString::number(QIOT_MBID_PS),productSecret);
            if (devType == "直连版")
            {
                moduleJson.insert(QString::number(QIOT_MBID_DEV_TYPE),0);
            }
            else if (devType == "网关版")
            {
                moduleJson.insert(QString::number(QIOT_MBID_DEV_TYPE),1);
            }
            fileName = filePath + "/" +productKey + "_" + current_date + "_local.bin";
            break;
        }
    }
    /* 串口信息 */
    QJsonValue uartValue;
    configFile->readJsonKeyValue("uart",&uartValue);
    if(uartValue.isObject())
    {
        QJsonArray configArray;
        QJsonObject configObj = uartValue.toObject();
        QJsonObject::Iterator it;
        for(it=configObj.begin();it!=configObj.end();it++)
        {
            QString port = it.key();
            QJsonObject uartObj = it.value().toObject();
            QString baudrate = uartObj.value("baudrate").toString();
            QString dataBits = uartObj.value("dataBits").toString();
            QString parity = uartObj.value("parity").toString();
            QString stopBits = uartObj.value("stopBits").toString();
            int cmdInterval = uartObj.value("cmdInterval").toInt();
            int pollingInterval = uartObj.value("pollingInterval").toInt();
            int resendCount = 2;
            if(uartObj.find("resendCount") != uartObj.end())
            {
                resendCount = uartObj.value("resendCount").toInt();
            }
            QJsonObject newUartObj;
            newUartObj.insert(QString::number(QIOT_MBID_UART_PORT),port.toInt());
            newUartObj.insert(QString::number(QIOT_MBID_UART_BAUDRATE),baudrate.toInt());
            if(dataBits == "5")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_DATABITS),0);
            }
            else if(dataBits == "6")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_DATABITS),1);
            }
            else if(dataBits == "7")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_DATABITS),2);
            }
            else if(dataBits == "8")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_DATABITS),3);
            }
            if(stopBits == "1")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_STOPBITS),0);
            }
            else if(stopBits == "1.5")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_STOPBITS),1);
            }
            else if(stopBits == "2")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_STOPBITS),2);
            }
            if(parity == "None")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_PARITY),0);
            }
            else if(parity == "Even")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_PARITY),1);
            }
            else if(parity == "Odd")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_PARITY),2);
            }
            else if(parity == "Mark")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_PARITY),3);
            }
            else if(parity == "Space")
            {
                newUartObj.insert(QString::number(QIOT_MBID_UART_PARITY),4);
            }
            newUartObj.insert(QString::number(QIOT_MBID_UART_POLLITV),pollingInterval);
            newUartObj.insert(QString::number(QIOT_MBID_UART_CMDITV),cmdInterval);
            newUartObj.insert(QString::number(QIOT_MBID_UART_RESENDCOUNT),resendCount);
            configArray.append(newUartObj);
        }
        moduleJson.insert(QString::number(QIOT_MBID_UARTCFG),configArray);
    }

    QJsonValue ttlvValue;
    configFile->readJsonKeyValue("ttlv",&ttlvValue);
    /* 寄存器信息 */
    QJsonValue devValue;
    configFile->readJsonKeyValue("dev",&devValue);
    if(devValue.isObject())
    {
        int devId = 1;
        int tslId = 1;
        QJsonArray registerConfigArray;
        QJsonObject devObj = devValue.toObject();
        QJsonObject::Iterator devIt;
        for(devIt=devObj.begin();devIt!=devObj.end();devIt++)
        {
            QJsonObject registerConfigObj;
//            QString devName = devIt.key();
            QJsonArray devAddrArray;
            if(getDevType())
            {
                QJsonObject subProductObj;
                QJsonObject productObj = devIt.value().toObject().value("product").toObject();
                subProductObj.insert(QString::number(QIOT_MBID_SUB_PK),productObj.value("productKey").toString());
                subProductObj.insert(QString::number(QIOT_MBID_SUB_PS),productObj.value("productSecret").toString());
                registerConfigObj.insert(QString::number(QIOT_MBID_SUB),subProductObj);
            }
            /* writeMode */
            int writeCoilMode = 0;
            int writeRegMode = 0;
            if(devIt.value().toObject().find("writeMode") != devIt.value().toObject().end())
            {
                QJsonObject writeModeObj = devIt.value().toObject().value("writeMode").toObject();
                if(writeModeObj.find("register") != writeModeObj.end())
                {
                    writeCoilMode = writeModeObj.value("coil").toInt();
                }
                if(writeModeObj.find("register") != writeModeObj.end())
                {
                    writeRegMode = writeModeObj.value("register").toInt();
                }
            }
            /* devAddr */
            QJsonObject addrObj = devIt.value().toObject().value("addr").toObject();
            QJsonObject::Iterator addrIt;
            for(addrIt=addrObj.begin();addrIt!=addrObj.end();addrIt++)
            {
                QString port = addrIt.key();
                QJsonArray addrArray = addrIt.value().toArray();
                QJsonObject portAddrObj;
                QJsonArray slaveArray;
                foreach (QJsonValue id, addrArray)
                {
                    QJsonObject slaveObj;
                    slaveObj.insert(QString::number(QIOT_MBID_SLAVE_ADDR),id.toInt());
                    slaveObj.insert(QString::number(QIOT_MBID_SLAVE_ID),getDevType()?0:devId++);
                    slaveArray.append(slaveObj);
                }
                if(slaveArray.count())
                {
                    portAddrObj.insert(QString::number(QIOT_MBID_UART_PORT),port.toInt());
                    portAddrObj.insert(QString::number(QIOT_MBID_ADDR_SLAVE),slaveArray);
                    devAddrArray.append(portAddrObj);
                }
            }
            registerConfigObj.insert(QString::number(QIOT_MBID_DEV_ADDR),devAddrArray);
            /* devData */
            QJsonObject devDataObj;
            QJsonObject registerObj = devIt.value().toObject().value("register").toObject();
            QJsonObject::Iterator registerFuncIt;
            for(registerFuncIt=registerObj.begin();registerFuncIt!=registerObj.end();registerFuncIt++)
            {
                QJsonArray devDataFunArray0x01;
                QJsonArray devDataFunArray0x02;
                QJsonArray devDataFunArray0x03;
                QJsonArray devDataFunArray0x04;
                QJsonArray devDataFunArray0x05;
                QJsonArray devDataFunArray0x06;
                QJsonArray devDataFunArray0x0f;
                QJsonArray devDataFunArray0x10;
                QString funcName = registerFuncIt.key();
                QJsonObject registerAddrObj = registerFuncIt.value().toObject();
                QJsonObject::Iterator registerAddrIt;
                for(registerAddrIt=registerAddrObj.begin();registerAddrIt!=registerAddrObj.end();registerAddrIt++)
                {
                    QJsonObject devAddrTtlvObj;
                    QJsonArray devAddrTtlvDataArrayR;
                    QJsonArray devAddrTtlvDataArrayWO;
                    QJsonArray devAddrTtlvDataArrayWM;
                    QString registerAddrName = registerAddrIt.key();
                    QJsonObject registerTtlvObj = registerAddrIt.value().toObject();
                    if(registerTtlvObj.isEmpty())
                    {
                        continue;
                    }
                    QRegExp rx("0x(.*)-0x(.*)");
                    if(-1 != registerAddrName.indexOf(rx))
                    {
                        int startAddr = rx.cap(1).toInt(nullptr,16);
                        int endAddr = rx.cap(2).toInt(nullptr,16);
                        int num = endAddr-startAddr+1;
                        devAddrTtlvObj.insert(QString::number(QIOT_MBID_REG_ADDR),startAddr);
                        devAddrTtlvObj.insert(QString::number(QIOT_MBID_REG_NUM),num);
                    }
                    QJsonObject::Iterator registerTtlvIt;
                    for(registerTtlvIt=registerTtlvObj.begin();registerTtlvIt!=registerTtlvObj.end();registerTtlvIt++)
                    {
                        QString registerTtlvName = registerTtlvIt.key();
                        qDebug()<<"功能名称"<<registerTtlvName<<tslId;
                        QJsonObject registerTtlvAttr = registerTtlvIt.value().toObject().value("attr").toObject();
                        QJsonArray registerTtlvPosArray = registerTtlvIt.value().toObject().value("pos").toArray();
                        for (int iPos = 0; iPos < registerTtlvPosArray.count(); iPos++)
                        {
                            QJsonObject devAddrTtlvDataObj;
                            QJsonObject registerTtlvPos = registerTtlvPosArray.at(iPos).toObject();
                            int row = registerTtlvPos.value("row").toInt()-1;
                            int span = registerTtlvPos.value("span").toInt();
                            if(registerTtlvAttr.value("type").toString() == "布尔值")
                            {
                                if(funcName.indexOf("线圈") >= 0 || funcName.indexOf("离散量") >= 0)
                                {
                                    int bitOffset = row%8;
                                    int byteOffset = row/8;
                                    devAddrTtlvDataObj.insert(QString::number(QIOT_MBID_REG_BYTEOFFSET),byteOffset);
                                    devAddrTtlvDataObj.insert(QString::number(QIOT_MBID_REG_BITOFFSET),bitOffset);
                                }
                                else if(funcName.indexOf("寄存器") >= 0)
                                {
                                    int bit = registerTtlvPos.value("bit").toInt();
                                    int bitOffset = bit%8;
                                    int byteOffset = bit<8?1:0;
                                    devAddrTtlvDataObj.insert(QString::number(QIOT_MBID_REG_BYTEOFFSET),byteOffset+row*2);
                                    devAddrTtlvDataObj.insert(QString::number(QIOT_MBID_REG_BITOFFSET),bitOffset);
                                }
                            }
                            else if(registerTtlvAttr.value("type").toString() == "数值" || registerTtlvAttr.value("type").toString() == "枚举")
                            {
                                devAddrTtlvDataObj.insert(QString::number(QIOT_MBID_REG_BYTEOFFSET),row*2);
                                devAddrTtlvDataObj.insert(QString::number(QIOT_MBID_REG_BITOFFSET),0);
                            }
                            else if(registerTtlvAttr.value("type").toString() == "字节流")
                            {
                                int len = registerTtlvAttr.value("len").toInt();
                                devAddrTtlvDataObj.insert(QString::number(QIOT_MBID_REG_BYTEOFFSET),row*2);
                                devAddrTtlvDataObj.insert(QString::number(QIOT_MBID_REG_BITOFFSET),0);
                            }
                            devAddrTtlvDataObj.insert(QString::number(QIOT_MBID_MODEL_ID),tslId);
                            devAddrTtlvDataObj.insert(QString::number(QIOT_MBID_TSLID),tslId++);
                            if(registerTtlvAttr.value("subType").toString().indexOf("读") >= 0)
                            {
                                devAddrTtlvDataArrayR.append(devAddrTtlvDataObj);
                            }
                            if(registerTtlvAttr.value("subType").toString().indexOf("写") >= 0)
                            {
                                if(span > 1)
                                {
                                    devAddrTtlvDataArrayWM.append(devAddrTtlvDataObj);
                                }
                                else
                                {
                                    devAddrTtlvDataArrayWO.append(devAddrTtlvDataObj);
                                }
                            }
                        }
                    }
                    if(funcName == "线圈")
                    {
                        if(!devAddrTtlvDataArrayR.isEmpty())
                        {
                            devAddrTtlvObj.insert(QString::number(QIOT_MBID_REG_DATA),devAddrTtlvDataArrayR);
                            devDataFunArray0x01.append(devAddrTtlvObj);
                        }
                        if(!devAddrTtlvDataArrayWO.isEmpty())
                        {
                            devAddrTtlvObj.insert(QString::number(QIOT_MBID_REG_DATA),devAddrTtlvDataArrayWO);
                            if(0 == writeCoilMode)
                            {
                                devDataFunArray0x05.append(devAddrTtlvObj);
                            }
                            else
                            {
                                devDataFunArray0x0f.append(devAddrTtlvObj);
                            }
                        }
                    }
                    else if(funcName == "保持寄存器")
                    {
                        if(!devAddrTtlvDataArrayR.isEmpty())
                        {
                            devAddrTtlvObj.insert(QString::number(QIOT_MBID_REG_DATA),devAddrTtlvDataArrayR);
                            devDataFunArray0x03.append(devAddrTtlvObj);
                        }
                        if(!devAddrTtlvDataArrayWO.isEmpty())
                        {
                            devAddrTtlvObj.insert(QString::number(QIOT_MBID_REG_DATA),devAddrTtlvDataArrayWO);
                            if(0 == writeRegMode)
                            {
                                devDataFunArray0x06.append(devAddrTtlvObj);
                            }
                            else
                            {
                                devDataFunArray0x10.append(devAddrTtlvObj);
                            }

                        }
                        if(!devAddrTtlvDataArrayWM.isEmpty())
                        {
                            devAddrTtlvObj.insert(QString::number(QIOT_MBID_REG_DATA),devAddrTtlvDataArrayWM);
                            devDataFunArray0x10.append(devAddrTtlvObj);
                        }
                    }
                    else if(funcName == "输入寄存器")
                    {
                        if(!devAddrTtlvDataArrayR.isEmpty())
                        {
                            devAddrTtlvObj.insert(QString::number(QIOT_MBID_REG_DATA),devAddrTtlvDataArrayR);
                            devDataFunArray0x04.append(devAddrTtlvObj);
                        }
                    }
                    else if(funcName == "离散量")
                    {
                        if(!devAddrTtlvDataArrayR.isEmpty())
                        {
                            devAddrTtlvObj.insert(QString::number(QIOT_MBID_REG_DATA),devAddrTtlvDataArrayR);
                            devDataFunArray0x02.append(devAddrTtlvObj);
                        }
                    }
                }
                if(!devDataFunArray0x01.isEmpty())
                {
                    devDataObj.insert(QString::number(QIOT_MBID_DATA_0X01),devDataFunArray0x01);
                }
                if(!devDataFunArray0x02.isEmpty())
                {
                    devDataObj.insert(QString::number(QIOT_MBID_DATA_0X02),devDataFunArray0x02);
                }
                if(!devDataFunArray0x03.isEmpty())
                {
                    devDataObj.insert(QString::number(QIOT_MBID_DATA_0X03),devDataFunArray0x03);
                }
                if(!devDataFunArray0x04.isEmpty())
                {
                    devDataObj.insert(QString::number(QIOT_MBID_DATA_0X04),devDataFunArray0x04);
                }
                if(!devDataFunArray0x05.isEmpty())
                {
                    devDataObj.insert(QString::number(QIOT_MBID_DATA_0X05),devDataFunArray0x05);
                }
                if(!devDataFunArray0x06.isEmpty())
                {
                    devDataObj.insert(QString::number(QIOT_MBID_DATA_0X06),devDataFunArray0x06);
                }
                if(!devDataFunArray0x0f.isEmpty())
                {
                    devDataObj.insert(QString::number(QIOT_MBID_DATA_0X0F),devDataFunArray0x0f);
                }
                if(!devDataFunArray0x10.isEmpty())
                {
                    devDataObj.insert(QString::number(QIOT_MBID_DATA_0X10),devDataFunArray0x10);
                }
            }
            registerConfigObj.insert(QString::number(QIOT_MBID_DEV_DATA),devDataObj);
            if(!devDataObj.isEmpty())
            {
                registerConfigArray.append(registerConfigObj);
            }
        }
        if(0 == registerConfigArray.count())
        {
            QMessageBox::information(0,QObject::tr("警告"),QObject::tr("请先添加功能信息"),QObject::tr("确认"));
            delete configFile;
            configFile = NULL;
            return false;
        }
        moduleJson.insert(QString::number(QIOT_MBID_DEVCFG),registerConfigArray);
    }
    /* 功能信息 */
    if(ttlvValue.isObject())
    {
        QJsonArray ttlvConfigArray;
        QJsonObject configObj = ttlvValue.toObject();
        QJsonObject::Iterator it;
        int event = 1;
        for(it=configObj.begin();it!=configObj.end();it++)
        {
            event++;
        }
        int modelId = 1;
        int eventId = event;
        for(it=configObj.begin();it!=configObj.end();it++)
        {
            QJsonObject newUartObj;
            QString name = it.key();
            QJsonObject ttlvObj = it.value().toObject();
            QString type = ttlvObj.value("type").toString();

            QJsonObject devObj = devValue.toObject();
            QJsonObject::Iterator devIt;
            bool isFind = false;
            for(devIt=devObj.begin();!isFind && devIt!=devObj.end();devIt++)
            {
                QJsonObject registerObj = devIt.value().toObject().value("register").toObject();
                QJsonObject::Iterator registerFuncIt;
                for(registerFuncIt=registerObj.begin();!isFind && registerFuncIt!=registerObj.end();registerFuncIt++)
                {
                    QJsonObject registerAddrObj = registerFuncIt.value().toObject();
                    QJsonObject::Iterator registerAddrIt;
                    for(registerAddrIt=registerAddrObj.begin();!isFind && registerAddrIt!=registerAddrObj.end();registerAddrIt++)
                    {
                        QJsonObject registerTtlvObj = registerAddrIt.value().toObject();
                        if(registerTtlvObj.isEmpty())
                        {
                            continue;
                        }
                        QJsonObject::Iterator registerTtlvIt;
                        for(registerTtlvIt=registerTtlvObj.begin();!isFind && registerTtlvIt!=registerTtlvObj.end();registerTtlvIt++)
                        {
                            QString registerTtlvName = registerTtlvIt.key();
                            if (registerTtlvName != name)
                            {
                                if (registerTtlvIt.value().toObject().value("attr").toObject().value("event").toObject().value("addEvent").toBool() == true && registerTtlvIt.value().toObject().value("attr").toObject().value("subType").toString() != "只写")
                                {
                                    eventId++;
                                }
                                modelId ++;
                            }
                            else
                            {
                                isFind = true;
                                break;
                            }
                        }
                    }
                }
            }

            qDebug()<<"属性名称"<<it.key()<<modelId;
            if(type == "数值" || type == "枚举")
            {
                if (type == "数值")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_ATTR_TYPE),1);
                }
                if (type == "枚举")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_ATTR_TYPE),3);
                }
                QString byte_order = ttlvObj.value("byte-order").toString();
                QString increment = ttlvObj.value("increment").toString();
                QString multiple = ttlvObj.value("multiple").toString();
                QString numType = ttlvObj.value("numType").toString();
                newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),"bool");
                newUartObj.insert(QString::number(QIOT_MBID_MODEL_MULTIPLE),multiple.toDouble());
                newUartObj.insert(QString::number(QIOT_MBID_MODEL_INCREMENT),increment.toDouble());
                if(byte_order == "大端模式")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_BYTEORDER),0);
                }
                else if(byte_order == "小端模式")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_BYTEORDER),1);
                }
                else if(byte_order == "大端模式字节交换")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_BYTEORDER),2);
                }
                else if(byte_order == "小端模式字节交换")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_BYTEORDER),3);
                }
                if(numType == "16位有符号整形")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),0);
                }
                else if(numType == "16位无符号整形")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),1);
                }
                else if(numType == "32位有符号整形")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),2);
                }
                else if(numType == "32位无符号整形")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),3);
                }
                else if(numType == "64位有符号整形")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),4);
                }
                else if(numType == "64位无符号整形")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),5);
                }
                else if(numType == "32位单精度浮点型")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),6);
                }
                else if(numType == "64位双精度浮点型")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),7);
                }
                else if(numType == "16进制有符号A.B型")
                {
                    newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),10);
                }
            }
            else if(type == "布尔值")
            {
                newUartObj.insert(QString::number(QIOT_MBID_ATTR_TYPE),0);
                newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),8);
            }
            else if(type == "字节流")
            {
                int len = ttlvObj.value("len").toInt();
                newUartObj.insert(QString::number(QIOT_MBID_ATTR_TYPE),2);
                newUartObj.insert(QString::number(QIOT_MBID_MODEL_TYPE),9);
                newUartObj.insert(QString::number(QIOT_MBID_MODEL_LEN),len);
            }
            newUartObj.insert(QString::number(QIOT_MBID_MODEL_ID),modelId);

            QJsonObject eventObj = ttlvObj.value("event").toObject();
            if (!eventObj.isEmpty())
            {
                if (eventObj.value("addEvent").toBool() == true  && ttlvObj.value("subType").toString() != "只写")
                {
                    qDebug()<<"事件名称"<<eventObj.value("eventName").toString()<<eventId;
                    QJsonObject newEventObj;
                    newEventObj.insert(QString::number(QIOT_MBID_EVENT_ID),eventId);
                    QJsonObject eventConditionsObj = eventObj.value("eventConditions").toObject();
                    if (type == "布尔值")
                    {
                        newEventObj.insert(QString::number(QIOT_MBID_EVENT_BOOL_TRIGGER),eventConditionsObj.value("bool").toBool());
                    }
                    else if (type == "数值")
                    {
                        newEventObj.insert(QString::number(QIOT_MBID_EVENT_MUN_MINI),eventConditionsObj.value("miniMum").toString().toDouble());
                        newEventObj.insert(QString::number(QIOT_MBID_EVENT_MUN_MAX),eventConditionsObj.value("maxMum").toString().toDouble());
                    }
                    else if (type == "枚举")
                    {
                        QJsonObject::Iterator eventIt;
                        QJsonArray enumArr;
                        for(eventIt=eventConditionsObj.begin();eventIt!=eventConditionsObj.end();eventIt++)
                        {
                            if(eventIt.value().toBool() == true)
                            {
                                enumArr.append(eventIt.key().toInt());
                            }
                        }
                        newEventObj.insert(QString::number(QIOT_MBID_EVENT_ENUM_TRIGGER),enumArr);
                    }
                    else if (type == "字节流")
                    {
                        newEventObj.insert(QString::number(QIOT_MBID_EVENT_BYTE_TRIGGER),"\\x" + eventConditionsObj.value("byteStream").toString());
                    }
                    newUartObj.insert(QString::number(QIOT_MBID_EVENT_INFO),newEventObj);
                }
            }
            ttlvConfigArray.append(newUartObj);
            modelId = 1;
            eventId = event;
        }
        moduleJson.insert(QString::number(QIOT_MBID_MODELCFG),ttlvConfigArray);
    }
    /* JSON转TTLV */
    QByteArray qjsonByte = QJsonDocument(moduleJson).toJson();
    const char *qjsonData = qjsonByte.data();
    cJSON *cjsonByte = cJSON_Parse(qjsonData);
    void *ttlvHead = Ql_iotJson2Ttlv(cjsonByte);
    quint32_t bufLen = Ql_iotTtlvFormatLen(ttlvHead) + __GET_POS_ELEMENT(SafeFlashHeadData_t, buf);
    SafeFlashHeadData_t *newSfHD = (SafeFlashHeadData_t *)HAL_MALLOC(bufLen);
    qDebug()<<"jsonData"<<qjsonData;
    if (NULL == ttlvHead || 0 == bufLen || NULL == newSfHD)
    {
        qDebug()<<"警告"<<ttlvHead<<bufLen<<newSfHD;
        QMessageBox::information(0,QObject::tr("警告"),QObject::tr("文件创建失败，请稍后再试"),QObject::tr("确认"));
        if(NULL == newSfHD)
        {
            HAL_FREE(newSfHD);
        }
        delete configFile;
        configFile = NULL;
        return false;
    }
    newSfHD->len = Ql_iotTtlvFormat(ttlvHead, newSfHD->buf);
    qDebug()<<"buf"<<newSfHD->buf;
    newSfHD->crc = (quint8_t)Quos_crcCalculate(0, newSfHD->buf, newSfHD->len);
    newSfHD->head = QIOT_MB_FILE_HEAD;
    /* 保存文件 */
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write((const char *)newSfHD,bufLen);
    file.flush();
    file.close();
    HAL_FREE(newSfHD);
    qjsonByte.clear();
    qjsonByte.squeeze();
    delete configFile;
    configFile = NULL;
    return true;
}


/*
    func:判断字符串是否是整数
*/
bool stringIsInt(QString value)
{
    int decimal = value.indexOf(".");
    if(decimal < 0)
    {
        return true;
    }
    for (decimal+=1; decimal < value.length(); decimal++)
    {
        if(value.at(decimal) != '0')
        {
            return false;
        }
    }
    return true;
}

///*
//    func:根据配置文件转换成云平台导入的功能定义文件
//*/
//bool toolKit::ConfigTransformationToCloud(QString filePath,QString current_date)
//{
//    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
//    jsonFile *configFile = new jsonFile(configfile);
//    configFile->readFile();
//    jsonFile *cloudFile = NULL;
//    /* 功能信息 */
//    QJsonValue ttlvValue;
//    configFile->readJsonKeyValue("ttlv",&ttlvValue);
//    /* 产品信息 */
//    QJsonValue productValue;
//    configFile->readJsonKeyValue("product",&productValue);
//    if(productValue.isObject())
//    {
//        QJsonObject profileObj;
//        QJsonObject configObj = productValue.toObject();
//        QJsonObject::Iterator it;
//        for(it=configObj.begin();it!=configObj.end();it++)
//        {
//            QJsonObject productObj = it.value().toObject();
//            QString productKey = productObj.value("productKey").toString();
//            profileObj.insert("productKey",productKey);
//            profileObj.insert("version",current_date);
//            QString fileName = filePath + "/" +productKey + "_" + current_date + "_tsl.json";
//            cloudFile = new jsonFile(fileName);
//            cloudFile->readFile();
//            break;
//        }
//        if(cloudFile)
//        {
//            cloudFile->writeJsonKeyValue("profile",profileObj);
//        }
//    }
//    if(NULL == cloudFile)
//    {
//        QMessageBox::information(0,QObject::tr("警告"),QObject::tr("平台配置文件创建失败，请稍后再试"),QObject::tr("确认"));
//        return false;
//    }
//    /* 寄存器信息 */
//    QJsonArray propertiesArray;
//    QJsonArray eventsArray;
//    QJsonObject propertiesObj;
//    QJsonValue devValue;
//    configFile->readJsonKeyValue("dev",&devValue);
//    if(devValue.isObject())
//    {
//        int devId = 1;
//        QJsonObject devObj = devValue.toObject();
//        QJsonObject::Iterator devIt;
//        for(devIt=devObj.begin();devIt!=devObj.end();devIt++)
//        {
//            /* devAddr */
//            QJsonObject addrObj = devIt.value().toObject().value("addr").toObject();
//            QJsonObject::Iterator addrIt;
//            for(addrIt=addrObj.begin();addrIt!=addrObj.end();addrIt++)
//            {
//                QString port = addrIt.key();
//                QJsonArray addrArray = addrIt.value().toArray();
//                foreach (QJsonValue id, addrArray)
//                {
//                    QJsonArray propertiesSpecsArray;
//                    QString name = "port"+port+"_slave"+QString::number(id.toInt());
//                    propertiesObj.insert("code",name);
//                    propertiesObj.insert("dataType","STRUCT");
//                    propertiesObj.insert("name",devIt.key()+"_"+port+"_"+QString::number(id.toInt()));
//                    propertiesObj.insert("subType","RW");
//                    propertiesObj.insert("id",devId);
//                    propertiesObj.insert("type","PROPERTY");
//                    propertiesObj.insert("desc","");
//                    /* devData */
//                    QJsonObject registerObj = devIt.value().toObject().value("register").toObject();
////                    QStringList ttlvList;
//                    int ttlvId = 1;
//                    QJsonObject::Iterator registerFuncIt;
//                    for(registerFuncIt=registerObj.begin();registerFuncIt!=registerObj.end();registerFuncIt++)
//                    {
//                        QString funcCode = QString::fromLocal8Bit(registerFuncIt.key().toLocal8Bit());
//                        qDebug()<<"funcCode"<<funcCode;
//                        QJsonObject registerAddrObj = registerFuncIt.value().toObject();
//                        QJsonObject::Iterator registerAddrIt;
//                        for(registerAddrIt=registerAddrObj.begin();registerAddrIt!=registerAddrObj.end();registerAddrIt++)
//                        {
//                            QString registerAddrName = registerAddrIt.key();
//                            QRegExp rx("0x(.*)-0x(.*)");
//                            if(-1 == registerAddrName.indexOf(rx))
//                            {
//                                break;
//                            }
//                            int startAddr = rx.cap(1).toInt(nullptr,16);
//                            QJsonObject registerTtlvObj = registerAddrIt.value().toObject();
//                            QJsonObject::Iterator registerTtlvIt;
//                            for(registerTtlvIt=registerTtlvObj.begin();registerTtlvIt!=registerTtlvObj.end();registerTtlvIt++)
//                            {
//                                QJsonObject propertiesSpecsObj;
//                                QJsonObject eventsSpecsObj;
//                                QString registerTtlvName = registerTtlvIt.key();
//                                QJsonObject registerTtlvAttr = registerTtlvIt.value().toObject().value("attr").toObject();
//                                QJsonArray registerTtlvPosArray = registerTtlvIt.value().toObject().value("pos").toArray();
//                                for (int iPos = 0; iPos < registerTtlvPosArray.count(); iPos++)
//                                {
//                                    QJsonObject registerTtlvPos = registerTtlvPosArray.at(iPos).toObject();
//                                    if(registerTtlvAttr.value("subType").toString() == "读写")
//                                    {
//                                        propertiesSpecsObj.insert("subType","RW");
//                                    }
//                                    else if(registerTtlvAttr.value("subType").toString() == "只读")
//                                    {
//                                        propertiesSpecsObj.insert("subType","R");
//                                    }
//                                    else if(registerTtlvAttr.value("subType").toString() == "只写")
//                                    {
//                                        propertiesSpecsObj.insert("subType","W");
//                                    }
//                                    if(registerTtlvAttr.value("type").toString() == "布尔值")
//                                    {
//                                        QJsonArray specsArray;
//                                        QJsonObject trueSpecsObj;
//                                        QJsonObject falseSpecsObj;
//                                        trueSpecsObj.insert("dataType","BOOL");
//                                        trueSpecsObj.insert("name","true");
//                                        trueSpecsObj.insert("value","true");
//                                        falseSpecsObj.insert("dataType","BOOL");
//                                        falseSpecsObj.insert("name","false");
//                                        falseSpecsObj.insert("value","false");
//                                        specsArray.append(trueSpecsObj);
//                                        specsArray.append(falseSpecsObj);
//                                        propertiesSpecsObj.insert("dataType","BOOL");
//                                        propertiesSpecsObj.insert("specs",specsArray);
//                                    }
//                                    else if(registerTtlvAttr.value("type").toString() == "数值")
//                                    {
//                                        QString numType = registerTtlvAttr.value("numType").toString();
//                                        QString increment = registerTtlvAttr.value("increment").toString();
//                                        QString multiple = registerTtlvAttr.value("multiple").toString();
//                                        double dIncrement = increment.toDouble();
//                                        double dMultiple = multiple.toDouble();
//                                        qDebug()<<"increment"<<increment<<dIncrement;
//                                        qDebug()<<"multiple"<<multiple<<dMultiple;
//                                        double min,max;
//                                        if(numType == "16位有符号整形")
//                                        {
//                                            min = -32768*dMultiple+dIncrement;
//                                            max = 32768*dMultiple+dIncrement;
//                                        }
//                                        else if(numType == "16位无符号整形")
//                                        {
//                                            min = 0;
//                                            max = 32768*dMultiple+dIncrement;
//                                        }
//                                        else if(numType == "32位有符号整形")
//                                        {
//                                            min = -2147483648*dMultiple+dIncrement;
//                                            max = 2147483647*dMultiple+dIncrement;
//                                        }
//                                        else if(numType == "32位无符号整形")
//                                        {
//                                            min = 0;
//                                            max = 2147483647*dMultiple+dIncrement;
//                                        }
//                                        else if(numType == "64位有符号整形")
//                                        {
//                                            min = -2147483648*dMultiple+dIncrement;
//                                            max = 2147483647*dMultiple+dIncrement;
//                                        }
//                                        else if(numType == "64位无符号整形")
//                                        {
//                                            min = 0;
//                                            max = 2147483647*dMultiple+dIncrement;
//                                        }
//                                        else if(numType == "32位单精度浮点型")
//                                        {
//                                            min = -4294967295*dMultiple+dIncrement;
//                                            max = 4294967295*dMultiple+dIncrement;
//                                        }
//                                        else if(numType == "64位双精度浮点型")
//                                        {
//                                            min = -4294967295*dMultiple+dIncrement;
//                                            max = 4294967295*dMultiple+dIncrement;
//                                        }
//                                        if(min > max)
//                                        {
//                                            int temp = min;
//                                            min = max;
//                                            max = temp;
//                                        }
//                                        qDebug()<<"min"<<min;
//                                        qDebug()<<"max"<<max;
//                                        QJsonObject specsObj;
//                                        specsObj.insert("unit","");
//                                        if(numType == "32位单精度浮点型")
//                                        {
//                                            propertiesSpecsObj.insert("dataType","FLOAT");
//                                            if(min < -4294967295)
//                                            {
//                                                min = -4294967295;
//                                            }
//                                            if(max > 4294967295)
//                                            {
//                                                max = 4294967295;
//                                            }
//                                            specsObj.insert("min",QString::number(min,10,6));
//                                            specsObj.insert("max",QString::number(max,10,6));
//                                            specsObj.insert("step",QString::number(dMultiple,10,6));
//                                        }
//                                        else if(numType == "64位双精度浮点型")
//                                        {
//                                            propertiesSpecsObj.insert("dataType","DOUBLE");
//                                            if(min < -4294967295)
//                                            {
//                                                min = -4294967295;
//                                            }
//                                            if(max > 4294967295)
//                                            {
//                                                max = 4294967295;
//                                            }
//                                            specsObj.insert("min",QString::number(min,10,6));
//                                            specsObj.insert("max",QString::number(max,10,6));
//                                            specsObj.insert("step",QString::number(dMultiple,10,6));
//                                        }
//                                        else if(stringIsInt(increment) && stringIsInt(multiple))
//                                        {
//                                            propertiesSpecsObj.insert("dataType","INT");
//                                            if(min < -2147483648)
//                                            {
//                                                min = -2147483648;
//                                            }
//                                            if(max > 2147483647)
//                                            {
//                                                max = 2147483647;
//                                            }
//                                            specsObj.insert("min",QString::number(min,10,0));
//                                            specsObj.insert("max",QString::number(max,10,0));
//                                            specsObj.insert("step",QString::number(dMultiple,10,0));
//                                        }
//                                        else
//                                        {
//                                            propertiesSpecsObj.insert("dataType","DOUBLE");
//                                            if(min < -4294967295)
//                                            {
//                                                min = -4294967295;
//                                            }
//                                            if(max > 4294967295)
//                                            {
//                                                max = 4294967295;
//                                            }
//                                            specsObj.insert("min",QString::number(min,10,6));
//                                            specsObj.insert("max",QString::number(max,10,6));
//                                            specsObj.insert("step",QString::number(dMultiple,10,6));
//                                        }
//                                        propertiesSpecsObj.insert("specs",specsObj);
//                                    }
//                                    else if(registerTtlvAttr.value("type").toString() == "枚举")
//                                    {
//                                        QJsonArray specsArray;
//                                        QJsonObject enumObj = registerTtlvAttr.value("enum").toObject();
//                                        QJsonObject::const_iterator it = enumObj.constBegin();
//                                        QJsonObject::const_iterator end = enumObj.constEnd();
//                                        while(it != end)
//                                        {
//                                            QJsonObject specsObj;
//                                            specsObj.insert("dataType","ENUM");
//                                            specsObj.insert("value",it.key());
//                                            specsObj.insert("name",it.value());
//                                            specsArray.append(specsObj);
//                                            it++;
//                                        }
//                                        propertiesSpecsObj.insert("dataType","ENUM");
//                                        propertiesSpecsObj.insert("specs",specsArray);
//                                    }
//                                    else if(registerTtlvAttr.value("type").toString() == "字节流")
//                                    {
//                                        QJsonObject specsObj;
//                                        specsObj.insert("length",registerTtlvAttr.value("len").toInt());
//                                        propertiesSpecsObj.insert("dataType","TEXT");
//                                        propertiesSpecsObj.insert("specs",specsObj);
//                                    }
//                                    QString mark;/* _设备ID_类型_地址_比特:1_1_0000_0 */;
//                                    if(-1 == registerTtlvPos.value("bit").toInt())
//                                    {
//                                        mark.sprintf("_%d_%s_%04x",devId,funcCode.toUtf8().data(),startAddr+registerTtlvPos.value("row").toInt()-1);
//                                    }
//                                    else
//                                    {
//                                        mark.sprintf("_%d_%s_%04x_%d",devId,funcCode.toUtf8().data(),startAddr+registerTtlvPos.value("row").toInt()-1,registerTtlvPos.value("bit").toInt());
//                                    }
//                                    qDebug()<<"mark"<<mark<<registerTtlvName+mark;
//                                    propertiesSpecsObj.insert("name",registerTtlvName+mark);
//                                    propertiesSpecsObj.insert("code",QString::number(devId)+"_"+QString::number(ttlvId));
//                                    propertiesSpecsObj.insert("id",ttlvId++);
//                                    propertiesSpecsArray.append(propertiesSpecsObj);

//                                    QJsonObject eventInfo = registerTtlvAttr.value("event").toObject();
//                                    if (eventInfo.value("addEvent").toBool() == true)
//                                    {
//                                        QJsonArray outputDataArr;
//                                        QJsonObject outputDataObj;
//                                        outputDataObj.insert("$ref",QString("#/properties/id/%1").arg(ttlvId-1));
//                                        outputDataArr.append(outputDataObj);
//                                        eventsSpecsObj.insert("outputData",outputDataArr);
//                                        eventsSpecsObj.insert("code",QString::number(ttlvId));

//                                        eventsSpecsObj.insert("name",eventInfo.value("eventName").toString());
//                                        if (eventInfo.value("eventType").toString() == "告警")
//                                        {
//                                            eventsSpecsObj.insert("subType","ERROR");
//                                        }
//                                        else if (eventInfo.value("eventType").toString() == "故障")
//                                        {
//                                            eventsSpecsObj.insert("subType","WARN");
//                                        }
//                                        else if (eventInfo.value("eventType").toString() == "信息")
//                                        {
//                                            eventsSpecsObj.insert("subType","INFO");
//                                        }
//                                        eventsSpecsObj.insert("id",ttlvId);
//                                        eventsSpecsObj.insert("sort",ttlvId++);
//                                        eventsSpecsObj.insert("type","EVENT");
//                                        eventsArray.append(eventsSpecsObj);
//                                    }
//                                }
//                            }
//                        }

//                    }
//                    propertiesObj.insert("specs",propertiesSpecsArray);
//                    propertiesArray.append(propertiesObj);
//                    devId++;
//                }
//            }
//        }
//        cloudFile->writeJsonKeyValue("properties",propertiesArray);
//        cloudFile->writeJsonKeyValue("events",eventsArray);
//    }
//    /* 保存文件 */
//    cloudFile->writeFile();
//}

/*
    func:根据配置文件转换成云平台导入的功能定义文件
*/
bool toolKit::ConfigTransformationToCloud(QString filePath,QString current_date)
{
    QString configfile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/QthTools-DTU_Modbus/.config.ini";
    jsonFile *configFile = new jsonFile(configfile);
    configFile->readFile();
    /* 寄存器信息 */
    QJsonValue devValue;
    QJsonValue productValue;
    QJsonValue ttlvValue;
    configFile->readJsonKeyValue("dev",&devValue);
    configFile->readJsonKeyValue("product",&productValue);
    configFile->readJsonKeyValue("ttlv",&ttlvValue);
    QJsonObject configObj = ttlvValue.toObject();
    QJsonObject::Iterator it;
    int event = 1;
    for(it=configObj.begin();it!=configObj.end();it++)
    {
        event++;
    }
    if(devValue.isObject())
    {
        QJsonObject devObj = devValue.toObject();
        QJsonObject::Iterator devIt;
        int ttlvId = 1;
        int eventId = event;
        for(devIt=devObj.begin();devIt!=devObj.end();devIt++)
        {
            QJsonArray propertiesArray;
            QJsonArray eventsArray;
            QJsonObject productObj = devIt.value().toObject().value("product").toObject();
            QString productKey = productObj.value("productKey").toString();
            if (!getDevType())
            {
                if (productValue.isObject())
                {
                    QJsonObject productObj = productValue.toObject();
                    QJsonObject::Iterator productIt;
                    for(productIt=productObj.begin();productIt!=productObj.end();productIt++)
                    {
                        productKey = productIt.value().toObject().value("productKey").toString();
                    }
                }
            }
            QString fileName = filePath + "/" +productKey + "_" + current_date + "_tsl.json";
            jsonFile *subCloudFile = new jsonFile(fileName);
            subCloudFile->readFile();
            /* product */
            QJsonObject profileObj;
            profileObj.insert("productKey",productKey);
            profileObj.insert("version",current_date);
            subCloudFile->writeJsonKeyValue("profile",profileObj);
            /* devData */
            QJsonObject registerObj = devIt.value().toObject().value("register").toObject();
            QJsonObject::Iterator registerFuncIt;
//            for(registerFuncIt=registerObj.begin();registerFuncIt!=registerObj.end();registerFuncIt++)
//            {
//                QJsonObject registerAddrObj = registerFuncIt.value().toObject();
//                QJsonObject::Iterator registerAddrIt;
//                for(registerAddrIt=registerAddrObj.begin();registerAddrIt!=registerAddrObj.end();registerAddrIt++)
//                {
//                    QString registerAddrName = registerAddrIt.key();
//                    QRegExp rx("0x(.*)-0x(.*)");
//                    if(-1 == registerAddrName.indexOf(rx))
//                    {
//                        break;
//                    }
//                    QJsonObject registerTtlvObj = registerAddrIt.value().toObject();
//                    QJsonObject::Iterator registerTtlvIt;
//                    for(registerTtlvIt=registerTtlvObj.begin();registerTtlvIt!=registerTtlvObj.end();registerTtlvIt++)
//                    {
//                        if (registerTtlvIt.value().toObject().value("attr").toObject().value("event").toObject().value("addEvent").toBool() == true && registerTtlvIt.value().toObject().value("attr").toObject().value("subType").toString() != "只写")
//                        {
//                            eventId++;
//                        }
//                    }
//                }
//            }
            for(registerFuncIt=registerObj.begin();registerFuncIt!=registerObj.end();registerFuncIt++)
            {
                QString funcCode = QString::fromLocal8Bit(registerFuncIt.key().toLocal8Bit());
                qDebug()<<"funcCode"<<funcCode;
                QJsonObject registerAddrObj = registerFuncIt.value().toObject();
                QJsonObject::Iterator registerAddrIt;
                for(registerAddrIt=registerAddrObj.begin();registerAddrIt!=registerAddrObj.end();registerAddrIt++)
                {
                    QString registerAddrName = registerAddrIt.key();
                    QRegExp rx("0x(.*)-0x(.*)");
                    if(-1 == registerAddrName.indexOf(rx))
                    {
                        break;
                    }
                    int startAddr = rx.cap(1).toInt(nullptr,16);
                    QJsonObject registerTtlvObj = registerAddrIt.value().toObject();
                    QJsonObject::Iterator registerTtlvIt;
                    for(registerTtlvIt=registerTtlvObj.begin();registerTtlvIt!=registerTtlvObj.end();registerTtlvIt++)
                    {
                        QJsonObject propertiesSpecsObj;
                        QJsonObject eventsSpecsObj;
                        QString registerTtlvName = registerTtlvIt.key();
                        QJsonObject registerTtlvAttr = registerTtlvIt.value().toObject().value("attr").toObject();
                        QJsonArray registerTtlvPosArray = registerTtlvIt.value().toObject().value("pos").toArray();
                        for (int iPos = 0; iPos < registerTtlvPosArray.count(); iPos++)
                        {
                            QJsonObject registerTtlvPos = registerTtlvPosArray.at(iPos).toObject();
                            if(registerTtlvAttr.value("subType").toString() == "读写")
                            {
                                propertiesSpecsObj.insert("subType","RW");
                            }
                            else if(registerTtlvAttr.value("subType").toString() == "只读")
                            {
                                propertiesSpecsObj.insert("subType","R");
                            }
                            else if(registerTtlvAttr.value("subType").toString() == "只写")
                            {
                                propertiesSpecsObj.insert("subType","W");
                            }
                            if(registerTtlvAttr.value("type").toString() == "布尔值")
                            {
                                QJsonArray specsArray;
                                QJsonObject trueSpecsObj;
                                QJsonObject falseSpecsObj;
                                trueSpecsObj.insert("dataType","BOOL");
                                trueSpecsObj.insert("name","true");
                                trueSpecsObj.insert("value","true");
                                falseSpecsObj.insert("dataType","BOOL");
                                falseSpecsObj.insert("name","false");
                                falseSpecsObj.insert("value","false");
                                specsArray.append(trueSpecsObj);
                                specsArray.append(falseSpecsObj);
                                propertiesSpecsObj.insert("dataType","BOOL");
                                propertiesSpecsObj.insert("specs",specsArray);
                            }
                            else if(registerTtlvAttr.value("type").toString() == "数值")
                            {
                                QString numType = registerTtlvAttr.value("numType").toString();
                                QString increment = registerTtlvAttr.value("increment").toString();
                                QString multiple = registerTtlvAttr.value("multiple").toString();
                                double dIncrement = increment.toDouble();
                                double dMultiple = multiple.toDouble();
                                qDebug()<<"increment"<<increment<<dIncrement;
                                qDebug()<<"multiple"<<multiple<<dMultiple;
                                double min,max;
                                if(numType == "16位有符号整形")
                                {
                                    min = -32768*dMultiple+dIncrement;
                                    max = 32768*dMultiple+dIncrement;
                                }
                                else if(numType == "16位无符号整形")
                                {
                                    min = 0+dIncrement;
                                    max = 65535*dMultiple+dIncrement;
                                }
                                else if(numType == "32位有符号整形")
                                {
                                    min = -2147483648*dMultiple+dIncrement;
                                    max = 2147483647*dMultiple+dIncrement;
                                }
                                else if(numType == "32位无符号整形")
                                {
                                    min = 0+dIncrement;
                                    max = 4294967296*dMultiple+dIncrement;
                                }
                                else if(numType == "64位有符号整形")
                                {
                                    min = -2147483648*dMultiple+dIncrement;
                                    max = 2147483647*dMultiple+dIncrement;
                                }
                                else if(numType == "64位无符号整形")
                                {
                                    min = 0+dIncrement;
                                    max = 4294967296*dMultiple+dIncrement;
                                }
                                else if(numType == "32位单精度浮点型")
                                {
                                    min = -4294967295*dMultiple+dIncrement;
                                    max = 4294967295*dMultiple+dIncrement;
                                }
                                else if(numType == "64位双精度浮点型")
                                {
                                    min = -4294967295*dMultiple+dIncrement;
                                    max = 4294967295*dMultiple+dIncrement;
                                }
                                qDebug()<<"min"<<min;
                                qDebug()<<"max"<<max;
                                QJsonObject specsObj;
                                specsObj.insert("unit","");
                                if(numType == "32位单精度浮点型")
                                {
                                    propertiesSpecsObj.insert("dataType","FLOAT");
                                    if(min < -4294967295)
                                    {
                                        min = -4294967295;
                                    }
                                    if(max > 4294967295)
                                    {
                                        max = 4294967295;
                                    }
                                    specsObj.insert("min",QString::number(min,10,6));
                                    specsObj.insert("max",QString::number(max,10,6));
                                    specsObj.insert("step",QString::number(dMultiple,10,6));
                                }
                                else if(numType == "64位双精度浮点型")
                                {
                                    propertiesSpecsObj.insert("dataType","DOUBLE");
                                    if(min < -4294967295)
                                    {
                                        min = -4294967295;
                                    }
                                    if(max > 4294967295)
                                    {
                                        max = 4294967295;
                                    }
                                    specsObj.insert("min",QString::number(min,10,6));
                                    specsObj.insert("max",QString::number(max,10,6));
                                    specsObj.insert("step",QString::number(dMultiple,10,6));
                                }
                                else if(stringIsInt(increment) && stringIsInt(multiple))
                                {
                                    propertiesSpecsObj.insert("dataType","INT");
                                    if(min < -2147483648)
                                    {
                                        min = -2147483648;
                                    }
                                    if(max > 2147483647)
                                    {
                                        max = 2147483647;
                                    }
                                    specsObj.insert("min",QString::number(min,10,0));
                                    specsObj.insert("max",QString::number(max,10,0));
                                    specsObj.insert("step",QString::number(dMultiple,10,0));
                                }
                                else
                                {
                                    propertiesSpecsObj.insert("dataType","DOUBLE");
                                    if(min < -4294967295)
                                    {
                                        min = -4294967295;
                                    }
                                    if(max > 4294967295)
                                    {
                                        max = 4294967295;
                                    }
                                    specsObj.insert("min",QString::number(min,10,6));
                                    specsObj.insert("max",QString::number(max,10,6));
                                    specsObj.insert("step",QString::number(dMultiple,10,6));
                                }
                                propertiesSpecsObj.insert("specs",specsObj);
                            }
                            else if(registerTtlvAttr.value("type").toString() == "枚举")
                            {
                                QJsonArray specsArray;
                                QJsonObject enumObj = registerTtlvAttr.value("enum").toObject();
                                QJsonObject::const_iterator it = enumObj.constBegin();
                                QJsonObject::const_iterator end = enumObj.constEnd();
                                while(it != end)
                                {
                                    QJsonObject specsObj;
                                    specsObj.insert("dataType","ENUM");
                                    specsObj.insert("value",it.key());
                                    specsObj.insert("name",it.value());
                                    specsArray.append(specsObj);
                                    it++;
                                }
                                propertiesSpecsObj.insert("dataType","ENUM");
                                propertiesSpecsObj.insert("specs",specsArray);
                            }
                            else if(registerTtlvAttr.value("type").toString() == "字节流")
                            {
                                QJsonObject specsObj;
                                specsObj.insert("length",registerTtlvAttr.value("len").toInt());
                                propertiesSpecsObj.insert("dataType","TEXT");
                                propertiesSpecsObj.insert("specs",specsObj);
                            }
//                            QString mark;/* _类型_地址_比特:1_1_0000_0 */;
//                            if(-1 == registerTtlvPos.value("bit").toInt())
//                            {
//                                mark.sprintf("_%s_%04x",funcCode.toUtf8().data(),startAddr+registerTtlvPos.value("row").toInt()-1);
//                            }
//                            else
//                            {
//                                mark.sprintf("_%s_%04x_%d",funcCode.toUtf8().data(),startAddr+registerTtlvPos.value("row").toInt()-1,registerTtlvPos.value("bit").toInt());
//                            }
//                            qDebug()<<"mark"<<mark<<registerTtlvName+mark;
                            propertiesSpecsObj.insert("name",registerTtlvName);
                            propertiesSpecsObj.insert("code",QString::number(ttlvId));
                            propertiesSpecsObj.insert("id",ttlvId++);
                            propertiesSpecsObj.insert("type","PROPERTY");
                            propertiesArray.append(propertiesSpecsObj);

                            QJsonObject eventInfo = registerTtlvAttr.value("event").toObject();
                            if (eventInfo.value("addEvent").toBool() == true && registerTtlvAttr.value("subType").toString() != "只写")
                            {
                                QJsonArray outputDataArr;
                                QJsonObject outputDataObj;
                                outputDataObj.insert("$ref",QString("#/properties/id/%1").arg(ttlvId-1));
                                outputDataArr.append(outputDataObj);
                                eventsSpecsObj.insert("outputData",outputDataArr);
                                eventsSpecsObj.insert("code",QString::number(eventId));

                                eventsSpecsObj.insert("name",eventInfo.value("eventName").toString());
                                if (eventInfo.value("eventType").toString() == "告警")
                                {
                                    eventsSpecsObj.insert("subType","WARN");
                                }
                                else if (eventInfo.value("eventType").toString() == "故障")
                                {
                                    eventsSpecsObj.insert("subType","ERROR");
                                }
                                else if (eventInfo.value("eventType").toString() == "信息")
                                {
                                    eventsSpecsObj.insert("subType","INFO");
                                }
                                eventsSpecsObj.insert("id",eventId);
                                eventsSpecsObj.insert("sort",eventId++);
                                eventsSpecsObj.insert("type","EVENT");
                                eventsArray.append(eventsSpecsObj);
                            }
                        }
                    }
                }
            }
            subCloudFile->writeJsonKeyValue("events",eventsArray);
            subCloudFile->writeJsonKeyValue("properties",propertiesArray);
            subCloudFile->writeFile();
        }
    }
    delete configFile;
    configFile = NULL;
}
#ifdef __cplusplus
}
#endif
