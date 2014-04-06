#include <QtGui/QApplication>
#include "dmm_maindiag.h"

int main(int argc, char** argv)
{
    DMM_MainDiag *DMM_Diag = 0;
    QApplication app(argc, argv);
    DMM_Diag = new DMM_MainDiag();
    DMM_Diag->show();
    return app.exec();
}



