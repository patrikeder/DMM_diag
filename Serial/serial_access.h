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

#include <QString>
#include <QStringList>

class Serial_Access
{
public:
  Serial_Access(QString interface, QString setting = "none");
  ~Serial_Access();
  
  int Serial_send(QString cmd);
  QString Serial_get(QString cmd,int bufSize);
  
  int Serial_get_connected();
  int Serial_disconnect();
  int Serial_connect(QString pr_interface);
  
  QStringList sl_dbg_msg;
  QStringList sl_err_msg;
  
private:
  int Serial_open(QString interface);
  int fd_interface;
    int ser_connected;
  int set_interface_attribs (int fd, int speed, int parity);
  int set_blocking (int fd, int should_block);
  
};

#endif // SERIAL_ACCESS_H
