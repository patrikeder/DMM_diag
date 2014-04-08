/*
 * Copyright 2014 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef M2550_ACCESS_H
#define M2550_ACCESS_H

#include <QString>
#include <QStringList>
#include "../Serial/serial_access.h"

#define INTERFACE_SETTINGS

class M2550_access: public QObject
{
    Q_OBJECT
public:
    #define DBG_MSG_M sl_dbg_msg_m2550.append
    #define ERR_MSG_M sl_err_msg_m2550.append


    enum eMeas {Volt,Curr,Freq,Cap,Res};


    M2550_access(QString interface);
    ~M2550_access();
    bool isConnected();
    bool disconnect();
    int M2550_connect(QString interface);
    
    int setMeasurement(QString type="VOLT", QString DCnAC="DC");
    int setResolution(int res=4);

    int updateSettings();
    int getSettings();

    int getMeasurement();

    int getValue();
    
    int getIDN();
    
    QStringList sl_dbg_msg_m2550;
    QStringList sl_err_msg_m2550;
    QString sl_msg_m2550;

private slots:
    void getMSG();

signals:
    void M2550_ack_received();

private:
    QString pr_interface,tSENS,tDCAC,tRES,tRANGE;

    bool connected;
    Serial_Access *m2550_serial;    

    const QString ccRange[2] = {"MAN","AUTO"};
    const QString ccMeasures[4] = {"VOLT","AMP","OHM","HERZ"};
    const QString ccResolution = "6";

};

#endif // M2550_ACCESS_H
