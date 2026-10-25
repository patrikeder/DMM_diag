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
#include <QtCore>
#include <QRandomGenerator>
#include <QTimer>
#include <QElapsedTimer>
#include <cmath>
#ifdef Q_OS_UNIX
#include <termios.h>
#endif
//#include <QDebug>

const QString Serial_Access::SIM_INTERFACE = "SIM";

Serial_Access::Serial_Access(QString interface)
{
  int i_return = 0;
  DBG_MSG_S("INTERFACE: DBG_EN");
  ERR_MSG_S("INTERFACE: ERR_EN");
  msg_available = false;
  sl_msg.clear();
  sl_dbg_msg.clear();
  sl_err_msg.clear();

  rx_acks = 0;
  sim_active = (interface == SIM_INTERFACE);
  sim_connected = sim_active;
  sim_sens = "VOLT";
  sim_dcac = "DC";
  sim_res = 5;
  sim_tick = 0;

  this->port = new QSerialPort(interface, this);
  // connect once; the port object is reused on reconnect
  QObject::connect(port, SIGNAL(readyRead()), this, SLOT(Serial_get()));
  if (sim_active){
      DBG_MSG_S("INTERFACE: SIMULATION");
      return;
    }
  if (port->open(QIODevice::ReadWrite)){
      i_return = set_interface_attribs();
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
          DBG_MSG_S("INTERFACE: "+interface+" INTERFACE: connected");
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
  if (sim_active){
      DBG_MSG_S("INTERFACE: Write "+cmd+" (sim)");
      if (!sim_connected)
        return -1;
      sim_send(cmd);
      return 0;
    }
  if (!port->isOpen())
    return -1;
  cmd.append("\n");
  DBG_MSG_S("INTERFACE: Write "+cmd);
  port->write(cmd.toLatin1());
  return 0;
}

// Like Serial_send, but returns only once the command has left the port and the instrument
// has answered (or timeout_ms passed). Needed before closing the port, as write() just queues
// and the port must not be closed while the command is still in transit.
int Serial_Access::Serial_send_wait(QString cmd, int timeout_ms)
{
  int ret = Serial_send(cmd);
  if (ret < 0 || sim_active)
    return ret;
  if (!port->waitForBytesWritten(timeout_ms))
    return -1;
#ifdef Q_OS_UNIX
  ::tcdrain(port->handle()); // wait until the UART / USB adapter has really sent it
#endif
  // Answers to earlier commands (e.g. a running measurement) can still arrive first, so
  // wait for an acknowledge and then until the line has been quiet for a moment.
  int acks_before = rx_acks;
  QElapsedTimer t;
  t.start();
  while (t.elapsed() < timeout_ms){
      bool answered = (rx_acks > acks_before);
      int wait = answered ? 150 : (int)(timeout_ms - t.elapsed());
      if (!port->waitForReadyRead(wait))
        break; // timeout, or quiet after the acknowledge
    }
  return (rx_acks > acks_before) ? 0 : -1;
}

void Serial_Access::Serial_get()
{
  QString qs_tmp;
  // read out all available bytes
  if (port->bytesAvailable()) {
      qs_tmp = QString::fromLatin1(port->readAll());
    }
  if (qs_tmp.contains("YES") || qs_tmp.contains("NO"))
    rx_acks++;
  // store into message buffer
  sl_msg.append(qs_tmp);
  msg_available = true; //TODO: needed ?
  // signal available data
  emit Serial_received();
}


int Serial_Access::Serial_open(QString pr_interface)
{
  int i_return = 0;

  sim_active = (pr_interface == SIM_INTERFACE);
  sim_connected = sim_active;
  if (sim_active){
      if (port->isOpen())
        port->close();
      DBG_MSG_S("INTERFACE: SIMULATION");
      return 0;
    }

  // reuse the existing port object so the readyRead connection survives a reconnect
  if (port->isOpen())
    port->close();
  port->setPortName(pr_interface);
  if (port->open(QIODevice::ReadWrite)){
      i_return = set_interface_attribs();
    }
  else {
      i_return = -1;
    }
  return i_return;

}

int Serial_Access::Serial_get_connected()
{
  return sim_active ? sim_connected : port->isOpen();
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
  sim_connected = false;
  if (port->isOpen())
    port->close();
  DBG_MSG_S("INTERFACE: disconnected");

  return 0;
}



int
Serial_Access::set_interface_attribs ()
{

  port->setBaudRate(QSerialPort::Baud19200);
  port->setFlowControl(QSerialPort::NoFlowControl);
  port->setParity(QSerialPort::NoParity);
  port->setDataBits(QSerialPort::Data8);
  port->setStopBits(QSerialPort::OneStop);
  return 0;
}

bool Serial_Access::Serial_get_ready(){
  return msg_available;
}

void Serial_Access::Serial_reset_ready(){
  msg_available = false;
}


// ---- simulated instrument -------------------------------------------------
// Answers like the real DMM: optional payload line, then YES/NO.

void Serial_Access::sim_send(const QString &cmd)
{
  QString payload;
  bool ok = true;

  if (cmd == "*IDN?"){
      payload = "SIMULATED,M2550,0,1.0";
    }
  else if (cmd == "READ:CH1"){
      payload = sim_value();
    }
  else if (cmd == "CONFIG?:CH1"){
      payload = sim_sens + ":" + sim_dcac + ",RANG AUTO,RES " + QString::number(sim_res);
    }
  else if (cmd.startsWith("SENS:")){
      // SENS:<type>[:<DC|AC>][,RANG <r>][,RES <n>]
      QStringList parts = cmd.mid(5).split(",");
      QStringList head = parts.value(0).split(":");
      if (QStringList({"VOLT", "CURR", "RESI", "CAP", "FREQ"}).contains(head.value(0))){
          sim_sens = head.value(0);
          if (head.size() > 1)
            sim_dcac = head.value(1);
          for (int i = 1; i < parts.size(); i++){
              if (parts.at(i).startsWith("RES "))
                sim_res = parts.at(i).mid(4).toInt();
            }
        }
      else
        ok = false;
    }
  else if (cmd == "DISP:IDLE" || cmd == "SYST:LOC"){
      // nothing to simulate
    }
  else
    ok = false;

  // like the real instrument: no echo, CR/LF line ends, acknowledge last
  QString reply = payload.isEmpty() ? QString() : payload + "\r\n";
  reply += QString(ok ? "YES" : "NO") + "\r\n";
  // answer asynchronously, like a real port
  QTimer::singleShot(30, this, [this, reply](){
      if (!sim_connected)
        return; // disconnected in the meantime
      sl_msg.append(reply);
      msg_available = true;
      emit Serial_received();
    });
}

QString Serial_Access::sim_value()
{
  double t = sim_tick++;
  double base, amp;
  if (sim_sens == "VOLT")      { base = (sim_dcac == "AC") ? 3.3 : 5.0; amp = 0.5; }
  else if (sim_sens == "CURR") { base = 0.1;    amp = 0.02; }
  else if (sim_sens == "RESI") { base = 1000.0; amp = 50.0; }
  else if (sim_sens == "CAP")  { base = 4.7e-6; amp = 0.3e-6; }
  else                         { base = 50.0;   amp = 0.5; }  // FREQ

  double noise = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.02 * base;
  double val = base + amp * std::sin(t * 0.2) + noise;

  // real format: sign, 3 integer digits, (digits-3) decimals, engineering exponent, e.g. +008.150E-3
  int dec = qMax(0, sim_res - 3);
  int exp3 = (val == 0) ? 0 : 3 * (int)std::floor(std::log10(std::fabs(val)) / 3);
  double m = std::fabs(val) / std::pow(10.0, exp3);
  QString mant = QString::number(m, 'f', dec).rightJustified(3 + (dec > 0 ? dec + 1 : 0), '0');
  return QString("%1%2E%3%4").arg(val < 0 ? "-" : "+").arg(mant)
                             .arg(exp3 < 0 ? "-" : "+").arg(qAbs(exp3));
}
