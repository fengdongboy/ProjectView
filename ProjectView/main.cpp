#include "ProjectView.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ProjectView w;
    w.show();
    return a.exec();
}
