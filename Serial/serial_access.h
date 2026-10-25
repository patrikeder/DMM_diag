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
#include <QSerialPort>

class Serial_Access: public QObject
{
    Q_OBJECT
#define ERR_MSG_S sl_err_msg.append
#define DBG_MSG_S sl_dbg_msg.append


public:
  // interface name that selects the built-in simulated instrument instead of a real port
  static const QString SIM_INTERFACE;

  Serial_Access(QString interface);
  ~Serial_Access();
  
  int Serial_send(QString cmd);
  int Serial_send_wait(QString cmd, int timeout_ms); // send, block until written and answered (for shutdown)

  
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
  QSerialPort *port;
  int ser_connected;
  int rx_acks; // YES/NO acknowledges received so far

  // simulated instrument
  bool sim_active;
  bool sim_connected;
  QString sim_sens, sim_dcac;
  int sim_res;
  int sim_tick;
  void sim_send(const QString &cmd);
  QString sim_value();
  int set_interface_attribs ();
  
};

#endif // SERIAL_ACCESS_H
