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

#include "serial_access.h"
#include "QtExtSerialPort/qextserialport.h"
#include <QtCore>
//#include <QDebug>


Serial_Access::Serial_Access(QString interface)
{
  int i_return = 0;
  DBG_MSG_S("INTERFACE: DBG_EN");
  ERR_MSG_S("INTERFACE: ERR_EN");
  msg_available = false;
  sl_msg.clear();
  sl_dbg_msg.clear();
  sl_err_msg.clear();

  this->port = new QextSerialPort(interface,QextSerialPort::EventDriven);
  port->setPortName(interface);
  if (port->open(QIODevice::ReadWrite | QIODevice::Unbuffered)){
      i_return = set_interface_attribs();//(*port,BAUD19200,PAR_NONE);
      port->flush();
      DBG_MSG_S("INTERFACE: created");
    }
  else {
      // error code goes here
      i_return = -1;
      ERR_MSG_S("INTERFACE: create error");
    }

  if  (i_return >= 0) {
      if (Serial_get_connected()){
          if (QObject::connect(port, SIGNAL(readyRead()), this,SLOT(Serial_get())))
            {
              DBG_MSG_S("INTERFACE: "+interface+" INTERFACE: connected");
            }
          else{
              ERR_MSG_S("INTERFACE: signal NOT connected");
            }
        }
      else {
          ERR_MSG_S("INTERFACE: could NOT open"+interface);
        }
    }
  else{
      ERR_MSG_S("INTERFACE: "+interface+" NOT available");
    }
}

Serial_Access::~Serial_Access()
{
  Serial_disconnect();
}


int Serial_Access::Serial_send(QString cmd)
{
  cmd.append("\n");
  DBG_MSG_S("INTERFACE: Write "+cmd);
  port->write(cmd.toAscii());
  return 0;
}

void Serial_Access::Serial_get()
{
  QString qs_tmp;
  // read out all available bytes
  if (port->bytesAvailable()) {
      qs_tmp = QString(port->readAll());
    }
  // store into message buffer
  sl_msg.append(qs_tmp);
  msg_available = true; //TODO: needed ?
  // signal available data
  emit Serial_received();
}


int Serial_Access::Serial_open(QString pr_interface)
{
  int i_return = 0;

  this->port = new QextSerialPort(pr_interface,QextSerialPort::EventDriven);
  port->setPortName(pr_interface);
  if (port->open(QIODevice::ReadWrite | QIODevice::Unbuffered)){
      i_return = set_interface_attribs();//(*port,BAUD19200,PAR_NONE);
    }
  else {
      i_return = -1;
    }
  return i_return;

}

int Serial_Access::Serial_get_connected()
{
  return port->isOpen();
}

int Serial_Access::Serial_connect(QString pr_interface) {
  int i_return = 0;

  if  (Serial_open(pr_interface) >= 0) {
      i_return = 0;
    }
  else {
      i_return = -1;

    }

  return i_return;
}


int Serial_Access::Serial_disconnect()
{
  if (port->isOpen())
    port->close();
  DBG_MSG_S("INTERFACE: disconnected");

  return 0;
}



int
Serial_Access::set_interface_attribs ()//QextSerialPort &fd, BaudRateType speed, ParityType parity)
{

  port->setBaudRate(BAUD19200);
  port->setFlowControl(FLOW_OFF);
  port->setParity(PAR_NONE);
  port->setDataBits(DATA_8);
  port->setStopBits(STOP_1);
  return 0;
}

int
Serial_Access::set_blocking (QextSerialPort fd, int should_block)
{

}

bool Serial_Access::Serial_get_ready(){
  return msg_available;
}

void Serial_Access::Serial_reset_ready(){
  msg_available = false;
}

