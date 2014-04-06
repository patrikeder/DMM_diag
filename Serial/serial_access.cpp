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

#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include "unistd.h"
#include "serial_access.h"

#include <QtExtSerialPort/qextserialport.h>

#define ERROR_MSG sl_err_msg.append
#define DBG_MSG sl_dbg_msg.append

Serial_Access::Serial_Access(QString interface, QString setting)
{
  DBG_MSG("INTERFACE: DBG_EN");
  ERROR_MSG("INTERFACE: ERR_EN");
  if  (Serial_open(interface) >= 0) {
        ser_connected = 1;
        DBG_MSG("INTERFACE: connected");
    }
    else {
        ser_connected = 0;
        DBG_MSG("INTERFACE: NOT connected");
    }
}

Serial_Access::~Serial_Access()
{
    Serial_disconnect();
}


int Serial_Access::Serial_send(QString cmd)
{
    DBG_MSG("INTERFACE: Write"+cmd);
    int wordsWritten = write(fd_interface, cmd.toAscii()+"\n", sizeof(cmd.toAscii()));
    return 0;
}

QString Serial_Access::Serial_get(QString cmd,int bufSize)
{
    QString qs_return;
    set_blocking(fd_interface,1);
    Serial_send(cmd);
    int wordsRead = read(fd_interface, qs_return.data_ptr(), bufSize);
    
    DBG_MSG("INTERFACE: GOT "+qs_return+" : "+QString::number(wordsRead)+" Words" );
    set_blocking(fd_interface,1);

    return qs_return;
}


int Serial_Access::Serial_open(QString pr_interface)
{
    int i_return = 0;

    fd_interface = open(
                       // the name of the serial port
                       // as a c-string (char *)
                       // eg. /dev/ttys0
                       pr_interface.toAscii(),
                       // configuration options
                       // O_RDWR - we need read
                       //     and write access
                       // O_CTTY - prevent other
                       //     input (like keyboard)
                       //     from affecting what we read
                       // O_NDELAY - We don't care if
                       //     the other side is
                       //     connected (some devices
                       //     don't explicitly connect)
                       O_RDWR | O_NOCTTY | O_NDELAY
                   );
    if(fd_interface == -1) {
        // error code goes here
        i_return = -1;
    }
    else {
        i_return = set_interface_attribs(fd_interface,B19200,0);
    }
    return i_return;

}

int Serial_Access::Serial_get_connected()
{
    return ser_connected;
}
int Serial_Access::Serial_connect(QString pr_interface) {
    int i_return = 0;

    if  (Serial_open(pr_interface) >= 0) {
        ser_connected = 1;
        i_return = 0;
        DBG_MSG("INTERFACE: connected");
    }
    else {
        ser_connected = 0;
        i_return = -1;
        DBG_MSG("INTERFACE: NOT connected");
    }

    return i_return;
}


int Serial_Access::Serial_disconnect()
{
    if (fd_interface > 0)
        close(fd_interface);
    DBG_MSG("INTERFACE: disconnected");

    return 0;
}



int
Serial_Access::set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        ERROR_MSG(QString::fromStdString("error from tcgetattr"));
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // ignore break signal
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        ERROR_MSG (QString("error from tcsetattr"));
        return -1;
    }
    return 0;
}

int
Serial_Access::set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        ERROR_MSG(QString("error from tggetattr"));
        return -1;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0) {
        ERROR_MSG(QString("error setting term attributes"));
        return -2;
    }
    else {
        return 0;
    }
}
