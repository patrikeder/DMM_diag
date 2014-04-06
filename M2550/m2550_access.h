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

enum eMeas {Volt,Curr,Freq,Cap,Res};
enum eRes  {TREE,FOUR,FIFE};


class M2550_access
{
public:
    M2550_access(QString interface);
    ~M2550_access();
    bool isConnected();
    bool disconnect();
    bool connect(QString interface);
    
    bool setMeasurement(eMeas type);
    bool setResolution(eRes res);

    int getValue();
    
    QString getIDN();
    
    QStringList sl_dbg_msg_m2550;
    QStringList sl_err_msg_m2550;


private:
    QString pr_interface;

    bool connected;
    Serial_Access *m2550_serial;
    void getMSG();

    QString ccRange[2] = {"MAN","AUTO"};
    QString ccMeasures[4] = {"VOLT","AMP","OHM","HERZ"};
    //static const char *const eResolution

};

#endif // M2550_ACCESS_H
