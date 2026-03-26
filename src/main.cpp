#include <QApplication>
#include "mainwindow.h"
#include "theme.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Vinil Player");
    app.setOrganizationName("VinilPlayer");

    app.setStyleSheet(Theme::globalStyleSheet());

    QFont defaultFont("Segoe UI", 12);
    defaultFont.setWeight(QFont::Medium);
    app.setFont(defaultFont);

    MainWindow window;
    window.show();

    return app.exec();
}