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

#ifndef SERIAL_ACCESS_H
#define SERIAL_ACCESS_H

#include <QSignalMapper>
#include <QString>
#include <QStringList>
#include <QtExtSerialPort/qextserialport.h>

class Serial_Access: public QObject
{
    Q_OBJECT
#define ERR_MSG_S sl_err_msg.append
#define DBG_MSG_S sl_dbg_msg.append


public:
  Serial_Access(QString interface);
  ~Serial_Access();
  
  int Serial_send(QString cmd);

  
  int Serial_get_connected();
  bool Serial_get_ready();
  void Serial_reset_ready();
  int Serial_disconnect();
  int Serial_connect(QString pr_interface);
  
  QStringList sl_dbg_msg; //DEBUG buffer
  QStringList sl_err_msg; //ERRor Buffer
  QStringList sl_msg; //Messages

signals:
  void Serial_received();

private slots:
  void Serial_get();

private:
  bool msg_available; // TODO: to be replaced by signal/mutex

  int Serial_open(QString interface);
  QextSerialPort *port;
  int ser_connected;
  int set_interface_attribs ();//QextSerialPort &fd, BaudRateType speed, ParityType parity);
  int set_blocking (QextSerialPort fd, int should_block);
  
};

#endif // SERIAL_ACCESS_H
