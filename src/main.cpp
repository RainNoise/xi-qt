#include "ximainwindow.h"

#include <QApplication>
#include <QDesktopWidget>


int main(int argc, char *argv[]) {
    // load config

    // launch exe
    QApplication a(argc, argv);
    xi::XiMainWindow w;
    w.show();
    w.resize(800, 600);
    w.move ((QApplication::desktop()->width()-w.width())/2, (QApplication::desktop()->height()-w.height())/2);

    return a.exec();
}