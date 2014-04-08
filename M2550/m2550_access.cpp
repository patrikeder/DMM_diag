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

#include "m2550_access.h"
#include <QDebug>

void M2550_access::getMSG() {
    DBG_MSG_M(m2550_serial->sl_dbg_msg);
    m2550_serial->sl_dbg_msg.clear();

    ERR_MSG_M(m2550_serial->sl_err_msg);
    m2550_serial->sl_err_msg.clear();
    if (m2550_serial->sl_msg.size()>0)
    {
        QString tmp = m2550_serial->sl_msg.join("");
        sl_msg_m2550.append(tmp);
        m2550_serial->sl_msg.clear();
        m2550_serial->Serial_reset_ready();
        if (sl_msg_m2550.contains("YES") || sl_msg_m2550.contains("NO")){
            emit M2550_ack_received();
            qDebug()<<"Signal ack";
        }
    }
}

M2550_access::M2550_access(QString interface) {
    pr_interface = interface;
    connected = false;
    m2550_serial = new Serial_Access(pr_interface);
    connected = m2550_serial->Serial_get_connected();
    if (connected){
        QObject::connect(m2550_serial,SIGNAL(Serial_received()),this,SLOT(getMSG()));
        DBG_MSG_M("INSTR: SLOT conn");getMSG();
    }
    tSENS = "VOLT";
    tDCAC = "DC";
    tRES = "6";
    tRANGE = "AUTO";
}


M2550_access::~M2550_access()
{
    m2550_serial->~Serial_Access();
}

bool M2550_access::isConnected()
{
    return connected;
}

bool M2550_access::disconnect()
{
    m2550_serial->Serial_send("SYST:LOC");
    m2550_serial->Serial_disconnect();
    connected = false;
    getMSG();
    return connected;
}

int M2550_access::M2550_connect(QString interface)
{
    if (m2550_serial == NULL) {
        m2550_serial = new Serial_Access(interface);
    }
    else {
        if (!connected) {
            m2550_serial->Serial_connect(interface);
        }
    }
    connected = m2550_serial->Serial_get_connected();
}

int M2550_access::getMeasurement(){
    QString cmd = "READ:CH1";
    DBG_MSG_M("INSTR: "+cmd);
    int ret = m2550_serial->Serial_send(cmd);
    return ret;
}

int M2550_access::setMeasurement(QString type, QString DCnAC)
{
    tSENS=type;
    tDCAC=DCnAC;
    updateSettings();
    return 0;
}

int M2550_access::updateSettings(){
    QString cmd;
    if (tSENS == "VOLT" || tSENS == "CURR"){
        cmd = "SENS:"+tSENS+":"+tDCAC;//+":RANG "+tRANGE+",RES"+tRES;
    }
    else{
        if (tSENS == "RESI"){
            cmd = "SENS:"+tSENS;//+":RANG "+tRANGE+",RES"+tRES;
        }
        else{
            if (tSENS ==  "CAP"){
                cmd = "SENS:"+tSENS;//+":RANG "+tRANGE
            }
            else{
                if (tSENS ==  "FREQ"){
                    cmd = "SENS:"+tSENS;//+":RANG "+tRANGE
                }
                else{

                    cmd = "";
                }
            }
        }
    }


    DBG_MSG_M("INSTR: "+cmd);
    int ret = m2550_serial->Serial_send(cmd);
    getMSG();
    return ret;
}

int M2550_access::getSettings(){
    QString cmd = "CONFIG?:CH1";
    DBG_MSG_M("INSTR: "+cmd);
    int ret = m2550_serial->Serial_send(cmd);
    getMSG();
    return 0;
}


int M2550_access::setResolution(int res)
{
    tRES = QString::number(res);
    updateSettings();
    return 0;
}

int M2550_access::getIDN() {
    DBG_MSG_M("INSTR: IDN?");
    int ret = m2550_serial->Serial_send("*IDN?");
    getMSG();
    return ret;
}
