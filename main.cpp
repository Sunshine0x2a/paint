#include <QApplication>

#include "canva.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Canva c;
    c.show();
    return a.exec();
}
