#include <QtGui/QApplication>
#include "DMM_Diag.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    DMM_Diag foo;
    foo.show();
    return app.exec();
}
