//#include "maindialog.h"
#include "ckernel.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    Dialog w;
//    w.show();  //将显示窗口放在核心类
    CKernel::GetInstance();
    return a.exec();
}
