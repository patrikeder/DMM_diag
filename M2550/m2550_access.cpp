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

#define DBG_MSG sl_dbg_msg_m2550.append
#define ERR_MSG sl_err_msg_m2550.append

void M2550_access::getMSG() {
    DBG_MSG(m2550_serial->sl_dbg_msg);
    m2550_serial->sl_dbg_msg.clear();
    ERR_MSG(m2550_serial->sl_err_msg);
    m2550_serial->sl_err_msg.clear();
}

M2550_access::M2550_access(QString interface) {
    pr_interface = interface;
    connected = false;
    m2550_serial = new Serial_Access(pr_interface,"nix");
    getMSG();
    connected = m2550_serial->Serial_get_connected();
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
    m2550_serial->Serial_disconnect();
    connected = false;
    getMSG();
}

bool M2550_access::connect(QString interface)
{
    if (m2550_serial == NULL) {
        m2550_serial = new Serial_Access(pr_interface,"nix");
    }
    else {
        if (!connected) {
            m2550_serial->Serial_connect(pr_interface);
        }
    }
    getMSG();
    connected = m2550_serial->Serial_get_connected();
}

bool M2550_access::setMeasurement(eMeas type)
{

}

bool M2550_access::setResolution(eRes res)
{

}

QString M2550_access::getIDN() {
    QString ret;
    DBG_MSG("INSTR: IDN?");
    ret = m2550_serial->Serial_get("*IDN?",512);    
    getMSG();
    return ret;
}
