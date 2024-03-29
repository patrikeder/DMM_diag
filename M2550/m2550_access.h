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

class M2550_access: public QObject
{
  Q_OBJECT
public:
#define DBG_MSG_M sl_dbg_msg_m2550.append // debug messages
#define ERR_MSG_M sl_err_msg_m2550.append // error indicators

  M2550_access(QString interface);
  ~M2550_access();
  bool isConnected();
  bool disconnect();
  int M2550_connect(QString interface);

  int updateSettings();
  int getSettings();

  int getMeasurement();
  int getIDN();

  QStringList sl_dbg_msg_m2550; // Debug message buffer
  QStringList sl_err_msg_m2550; // Error message buffer
  QString sl_msg_m2550;         // Main message buffer

public slots:
  int setMeasurement(QString type="VOLT");
  int setResolution(int res=4);
  int setDCAC(QString DCnAC="DC");
  int DisplayOFF();


private slots:
  void getMSG();

signals:
  void M2550_ack_received();

private:
  QString pr_interface,tSENS,tDCAC,tRES,tRANGE;

  bool connected;
  Serial_Access *m2550_serial;

};

#endif // M2550_ACCESS_H
