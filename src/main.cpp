#include <QApplication>
#include <QSettings>
#include "mainwindow.h"
#include "theme.h"

//acrescentar um aviso em caso esteja repetindo a música ao adicionar
//corrigir replay da musica
//corrigir reload da musica
//ao clicar a pasta que fica entre musica e coração de like, deverá direcionar a pasta determinada
//criar a estilização da capa da playlist(renomear "pastas" para "playlists") de acordo com usuário
//considerar que possa inserir uma musica isolada da playlist(sem necessidade de criar playlist)
//poder adicionar a musica isolada a playlist ou musica já adicionada em outra playlist
//criação do crud da playlist
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
