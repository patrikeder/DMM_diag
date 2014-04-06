#ifndef DMM_MainDiag_priv_H
#define DMM_MainDiag_priv_H

#include <QTimer>
#include <QString>
#include <QStringList>
#include "m2550_access.h"

M2550_access *M51_instr = 0;
QString cp_interface;
QTimer *diag_timer = 0;

#endif