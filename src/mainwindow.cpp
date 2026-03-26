#include "mainwindow.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QDialog>
#include <QGridLayout>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Vinil Player");
    resize(1100, 720);
    setMinimumSize(900, 550);

    m_model = new TrackModel(this);

    // ── Central widget ──────────────────────────────────────
    auto *central = new QWidget(this);
    setCentralWidget(central);
    central->setStyleSheet(Theme::globalStyleSheet());

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    // ── Sidebar ─────────────────────────────────────────────
    auto *sidebar = new QWidget();
    sidebar->setFixedWidth(260);
    sidebar->setStyleSheet(QString("background-color: %1;").arg(Theme::surface().name()));
    buildSidebar(sidebar);
    bodyLayout->addWidget(sidebar);

    // Separator
    auto *sep = new QFrame();
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet(QString("color: %1;").arg(Theme::border().name()));
    sep->setFixedWidth(1);
    bodyLayout->addWidget(sep);

    // ── Stacked pages ───────────────────────────────────────
    m_stack = new QStackedWidget();
    m_stack->setStyleSheet("background: transparent;");

    m_homePage = new HomePage(m_model, this);
    m_addPage = new AddMusicPage(m_model, this);
    m_foldersPage = new FoldersPage(m_model, this);
    m_folderDetailPage = new FolderDetailPage(m_model, this);
    m_likedPage = new LikedPage(m_model, this);

    m_stack->addWidget(m_homePage);       // 0
    m_stack->addWidget(m_addPage);        // 1
    m_stack->addWidget(m_foldersPage);    // 2
    m_stack->addWidget(m_folderDetailPage); // 3
    m_stack->addWidget(m_likedPage);      // 4

    bodyLayout->addWidget(m_stack, 1);
    mainLayout->addLayout(bodyLayout, 1);

    // ── Player bar ──────────────────────────────────────────
    m_playerBar = new PlayerBar(m_model, this);
    mainLayout->addWidget(m_playerBar);

    // ── Connections ─────────────────────────────────────────
    // Home page
    connect(m_homePage, &HomePage::playRequested, this, &MainWindow::onTrackPlay);
    connect(m_homePage, &HomePage::likeToggled, this, [this](int id) {
        m_model->toggleLike(id);
        refreshCurrentPage();
    });
    connect(m_homePage, &HomePage::navigateTo, this, &MainWindow::navigateTo);

    // Add page
    connect(m_addPage, &AddMusicPage::trackAdded, this, [this](const Track &) {
        refreshCurrentPage();
        refreshSidebarFolders();
    });
    connect(m_addPage, &AddMusicPage::navigateBack, this, [this]() { navigateTo("home"); });

    // Folders page
    connect(m_foldersPage, &FoldersPage::folderSelected, this, [this](const QString &name) {
        navigateTo("folder", name);
    });

    // Folder detail
    connect(m_folderDetailPage, &FolderDetailPage::playRequested, this, &MainWindow::onTrackPlay);
    connect(m_folderDetailPage, &FolderDetailPage::likeToggled, this, [this](int id) {
        m_model->toggleLike(id);
        refreshCurrentPage();
    });
    connect(m_folderDetailPage, &FolderDetailPage::deleteRequested, this, [this](int id) {
        m_model->removeTrack(id);
        refreshCurrentPage();
        refreshSidebarFolders();
    });
    connect(m_folderDetailPage, &FolderDetailPage::navigateBack, this, [this]() { navigateTo("folders"); });

    // Liked page
    connect(m_likedPage, &LikedPage::playRequested, this, &MainWindow::onTrackPlay);
    connect(m_likedPage, &LikedPage::likeToggled, this, [this](int id) {
        m_model->toggleLike(id);
        refreshCurrentPage();
    });
    connect(m_likedPage, &LikedPage::navigateBack, this, [this]() { navigateTo("home"); });

    // Player bar
    connect(m_playerBar, &PlayerBar::trackChanged, this, [this](int) {
        refreshCurrentPage();
    });

    // Model changes
    connect(m_model, &TrackModel::tracksChanged, this, [this]() {
        m_trackCountLabel->setText(QString("%1 faixa%2 na biblioteca")
            .arg(m_model->tracks().size())
            .arg(m_model->tracks().size() != 1 ? "s" : ""));
    });

    // Initial state
    navigateTo("home");
}

void MainWindow::buildSidebar(QWidget *sidebar) {
    auto *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Logo
    auto *logoWidget = new QWidget();
    logoWidget->setStyleSheet("background: transparent;");
    auto *logoLayout = new QHBoxLayout(logoWidget);
    logoLayout->setContentsMargins(20, 20, 20, 16);
    logoLayout->setSpacing(8);

    auto *logoIcon = new QLabel("\uE8D6");
    logoIcon->setFont(Theme::iconFont(16));
    logoIcon->setStyleSheet(QString("color: %1; background: %2; border-radius: 8px; padding: 4px 8px;")
        .arg(Theme::bg().name(), Theme::accent().name()));
    logoLayout->addWidget(logoIcon);

    auto *logoText = new QLabel("Vinil");
    logoText->setFont(Theme::titleFont(18));
    logoText->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::text().name()));
    logoLayout->addWidget(logoText);

    auto *badge = new QLabel("PLAYER");
    badge->setFont(Theme::bodyFont(9));
    badge->setStyleSheet(QString("color: %1; background: rgba(232,164,74,0.15); border-radius: 4px; padding: 2px 6px; font-weight: bold;")
        .arg(Theme::accent().name()));
    logoLayout->addWidget(badge);
    logoLayout->addStretch();

    layout->addWidget(logoWidget);

    // Navigation buttons
    auto *navWidget = new QWidget();
    navWidget->setStyleSheet("background: transparent;");
    auto *navLayout = new QVBoxLayout(navWidget);
    navLayout->setContentsMargins(10, 0, 10, 0);
    navLayout->setSpacing(2);

    auto makeNavBtn = [this](const QString &text, const QString &icon) -> QPushButton* {
        auto *btn = new QPushButton(QString("  %1  %2").arg(icon, text));
        btn->setFixedHeight(40);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFont(Theme::bodyFont(13));
        btn->setStyleSheet(QString(
            "QPushButton { background: transparent; color: %1; border: none; border-radius: 10px; text-align: left; padding-left: 14px; font-family: \"Segoe UI\", \"Segoe MDL2 Assets\"; }"
            "QPushButton:hover { background: rgba(255,255,255,0.05); color: %2; }"
        ).arg(Theme::textSoft().name(), Theme::text().name()));
        return btn;
    };

    m_navHome    = makeNavBtn("Início",    "\uE10F");
    m_navAdd     = makeNavBtn("Adicionar", "\uE109");
    m_navFolders = makeNavBtn("Pastas",    "\uE188");

    connect(m_navHome, &QPushButton::clicked, [this]() { navigateTo("home"); });
    connect(m_navAdd, &QPushButton::clicked, [this]() { navigateTo("add"); });
    connect(m_navFolders, &QPushButton::clicked, [this]() { navigateTo("folders"); });

    navLayout->addWidget(m_navHome);
    navLayout->addWidget(m_navAdd);
    navLayout->addWidget(m_navFolders);

    layout->addWidget(navWidget);

    // Sidebar folders
    auto *foldersHeader = new QLabel("  SUAS PASTAS");
    foldersHeader->setFont(Theme::bodyFont(10));
    foldersHeader->setStyleSheet(QString("color: %1; background: transparent; font-weight: bold; letter-spacing: 1px; padding: 16px 20px 4px;")
        .arg(Theme::textMuted().name()));
    layout->addWidget(foldersHeader);

    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    m_sidebarFoldersContainer = new QWidget();
    m_sidebarFoldersContainer->setStyleSheet("background: transparent;");
    m_sidebarFoldersLayout = new QVBoxLayout(m_sidebarFoldersContainer);
    m_sidebarFoldersLayout->setContentsMargins(10, 0, 10, 0);
    m_sidebarFoldersLayout->setSpacing(1);
    m_sidebarFoldersLayout->addStretch();

    scrollArea->setWidget(m_sidebarFoldersContainer);
    layout->addWidget(scrollArea, 1);

    // Footer
    auto *footerSep = new QFrame();
    footerSep->setFrameShape(QFrame::HLine);
    footerSep->setStyleSheet(QString("color: %1;").arg(Theme::border().name()));
    footerSep->setFixedHeight(1);
    layout->addWidget(footerSep);

    auto *footerWidget = new QWidget();
    footerWidget->setStyleSheet("background: transparent;");
    auto *footerLayout = new QHBoxLayout(footerWidget);
    footerLayout->setContentsMargins(20, 8, 12, 12);
    footerLayout->setSpacing(4);

    m_trackCountLabel = new QLabel("0 faixas na biblioteca");
    m_trackCountLabel->setFont(Theme::bodyFont(10));
    m_trackCountLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textMuted().name()));
    footerLayout->addWidget(m_trackCountLabel, 1);

    auto *themeBtn = new QPushButton("\uE790");
    themeBtn->setFixedSize(28, 28);
    themeBtn->setCursor(Qt::PointingHandCursor);
    themeBtn->setFont(Theme::iconFont(12));
    themeBtn->setToolTip("Escolher tema");
    themeBtn->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; border-radius: 6px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.08); color: %2; }"
    ).arg(Theme::textMuted().name(), Theme::accent().name()));
    connect(themeBtn, &QPushButton::clicked, this, &MainWindow::showThemePicker);
    footerLayout->addWidget(themeBtn);

    layout->addWidget(footerWidget);
}

void MainWindow::refreshSidebarFolders() {
    // Clear
    QLayoutItem *item;
    while ((item = m_sidebarFoldersLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    auto folders = m_model->folders();
    if (folders.isEmpty()) {
        auto *emptyLabel = new QLabel("Nenhuma pasta");
        emptyLabel->setFont(Theme::bodyFont(11));
        emptyLabel->setStyleSheet(QString("color: %1; background: transparent; padding: 4px 14px;").arg(Theme::textMuted().name()));
        m_sidebarFoldersLayout->addWidget(emptyLabel);
    } else {
        for (auto &f : folders) {
            int count = m_model->tracksInFolder(f.name).size();
            auto *btn = new QPushButton();
            btn->setCursor(Qt::PointingHandCursor);
            btn->setFixedHeight(34);
            btn->setFont(Theme::bodyFont(12));

            auto *btnLayout = new QHBoxLayout(btn);
            btnLayout->setContentsMargins(14, 0, 14, 0);
            auto *nameLabel = new QLabel(f.name);
            nameLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textSoft().name()));
            nameLabel->setFont(Theme::bodyFont(12));
            auto *countLabel = new QLabel(QString::number(count));
            countLabel->setFont(Theme::monoFont(10));
            countLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textMuted().name()));
            btnLayout->addWidget(nameLabel);
            btnLayout->addStretch();
            btnLayout->addWidget(countLabel);

            bool isActive = (m_currentPage == "folder" && m_folderDetailPage->property("folderName").toString() == f.name);
            btn->setStyleSheet(QString(
                "QPushButton { background: %1; border: none; border-radius: 8px; }"
                "QPushButton:hover { background: rgba(255,255,255,0.05); }"
            ).arg(isActive ? "rgba(232,164,74,0.12)" : "transparent"));

            QString folderName = f.name;
            connect(btn, &QPushButton::clicked, [this, folderName]() { navigateTo("folder", folderName); });

            m_sidebarFoldersLayout->addWidget(btn);
        }
    }

    m_sidebarFoldersLayout->addStretch();
}

void MainWindow::navigateTo(const QString &page, const QString &data) {
    m_currentPage = page;

    // Update nav button styles
    auto activeStyle = [](bool active) {
        if (active) {
            return QString(
                "QPushButton { background: rgba(232,164,74,0.15); color: %1; border: none; border-radius: 10px; text-align: left; padding-left: 14px; font-weight: bold; }"
            ).arg(Theme::accent().name());
        }
        return QString(
            "QPushButton { background: transparent; color: %1; border: none; border-radius: 10px; text-align: left; padding-left: 14px; }"
            "QPushButton:hover { background: rgba(255,255,255,0.05); color: %2; }"
        ).arg(Theme::textSoft().name(), Theme::text().name());
    };

    m_navHome->setStyleSheet(activeStyle(page == "home"));
    m_navAdd->setStyleSheet(activeStyle(page == "add"));
    m_navFolders->setStyleSheet(activeStyle(page == "folders" || page == "folder"));

    if (page == "home") {
        m_stack->setCurrentIndex(0);
    } else if (page == "add") {
        m_addPage->refresh();
        m_stack->setCurrentIndex(1);
    } else if (page == "folders") {
        m_stack->setCurrentIndex(2);
    } else if (page == "folder") {
        m_folderDetailPage->setFolder(data);
        m_folderDetailPage->setProperty("folderName", data);
        m_stack->setCurrentIndex(3);
    } else if (page == "liked") {
        m_stack->setCurrentIndex(4);
    }

    refreshCurrentPage();
    refreshSidebarFolders();
}

void MainWindow::refreshCurrentPage() {
    int curId = m_playerBar->currentTrackId();
    bool playing = m_playerBar->isPlaying();

    if (m_stack->currentIndex() == 0) {
        m_homePage->refresh(curId, playing);
    } else if (m_stack->currentIndex() == 2) {
        m_foldersPage->refresh();
    } else if (m_stack->currentIndex() == 3) {
        m_folderDetailPage->refresh(curId, playing);
    } else if (m_stack->currentIndex() == 4) {
        m_likedPage->refresh(curId, playing);
    }
}

void MainWindow::onTrackPlay(const Track &track) {
    m_playerBar->playTrack(track);
    refreshCurrentPage();
}

void MainWindow::showThemePicker() {
    auto *dlg = new QDialog(this);
    dlg->setWindowTitle("Escolher Tema");
    dlg->setFixedSize(356, 290);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setStyleSheet(QString(
        "QDialog { background: %1; }"
        "QLabel  { background: transparent; color: %2; }"
    ).arg(Theme::surface().name(), Theme::text().name()));

    auto *layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto *title = new QLabel("Paleta de Cores");
    title->setFont(Theme::titleFont(14));
    layout->addWidget(title);

    auto *grid = new QGridLayout();
    grid->setSpacing(8);

    const auto themes   = Theme::allThemes();
    const QString curId = Theme::activeTheme().id;

    for (int i = 0; i < themes.size(); ++i) {
        const auto &t = themes[i];

        auto *btn = new QPushButton();
        btn->setFixedSize(152, 72);
        btn->setCursor(Qt::PointingHandCursor);

        bool active = (t.id == curId);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
            "    stop:0 %1, stop:1 %2);"
            "  border: 2px solid %3;"
            "  border-radius: 10px;"
            "}"
            "QPushButton:hover { border: 2px solid %4; }"
        ).arg(t.bg.name(), t.accent.name(),
              active ? t.accent.name() : t.border.name(),
              t.accent.name()));

        auto *btnLayout = new QVBoxLayout(btn);
        btnLayout->setAlignment(Qt::AlignCenter);
        btnLayout->setContentsMargins(0, 0, 0, 0);

        auto *nameLabel = new QLabel(t.name);
        nameLabel->setFont(Theme::bodyFont(10));
        nameLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(t.text.name()));
        nameLabel->setAlignment(Qt::AlignCenter);
        btnLayout->addWidget(nameLabel);

        connect(btn, &QPushButton::clicked, dlg, [this, t, dlg]() {
            QSettings s;
            s.setValue("theme", t.id);
            dlg->accept();
            emit themeChangeRequested();
        });

        grid->addWidget(btn, i / 2, i % 2);
    }

    layout->addLayout(grid);
    dlg->exec();
}