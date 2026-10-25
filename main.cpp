#include <QApplication>
#include "dmm_maindiag.h"

int main(int argc, char** argv)
{
    DMM_MainDiag *DMM_Diag = 0;
    QApplication app(argc, argv);
    DMM_Diag = new DMM_MainDiag();
    DMM_Diag->show();
    int ret = app.exec();
    delete DMM_Diag; // releases the instrument (back to local mode)
    return ret;
}



