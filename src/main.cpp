#include <QApplication>
#include <QSettings>
#include "mainwindow.h"
#include "theme.h"

//acrescentar um aviso em caso esteja repetindo a música ao adicionar
//corrigir replay da musica
//corrigir reload da musica
//ao clicar a pasta que fica entre musica e coração de like, deverá direcionar a pasta determinada
//acrescentar o play em coleção curtidas
//separar play em cada playlist e não ter play somente em "Recentes"
//considerar as musicas curtidas como playlist propria
//acrescentar a fila das ultimas musicas escutadas
//reload dinâmica de saudação: "bom dia", "boa tarde" e "boa noite" em cada horário determinada

static const int RESTART_CODE = 1000;

int main(int argc, char *argv[])
{
    int exitCode;
    do {
        QApplication app(argc, argv);
        app.setApplicationName("Vinil Player");
        app.setOrganizationName("VinilPlayer");

        QSettings settings;
        Theme::setActiveTheme(Theme::themeById(settings.value("theme", "warm").toString()));

        app.setStyleSheet(Theme::globalStyleSheet());

        QFont defaultFont("Segoe UI", 12);
        defaultFont.setWeight(QFont::Medium);
        app.setFont(defaultFont);

        MainWindow window;
        QObject::connect(&window, &MainWindow::themeChangeRequested,
                         []() { qApp->exit(RESTART_CODE); });
        window.show();

        exitCode = app.exec();
    } while (exitCode == RESTART_CODE);

    return exitCode;
}
