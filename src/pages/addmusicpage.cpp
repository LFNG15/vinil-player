#include "addmusicpage.h"
#include <QLabel>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QScrollArea>
#include <QFrame>
#include <QUuid>
#include <QRegularExpression>
#include <algorithm>

AddMusicPage::AddMusicPage(TrackModel *model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    setAcceptDrops(true);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");

    auto *content = new QWidget();
    content->setStyleSheet("background: transparent;");
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(12);

    // Back button
    auto *backBtn = new QPushButton("←");
    backBtn->setFixedSize(34, 34);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(QString(
        "QPushButton { background: rgba(255,255,255,0.05); color: %1; border: none; border-radius: 17px; font-size: 16px; }"
        "QPushButton:hover { background: rgba(255,255,255,0.1); }"
    ).arg(Theme::text().name()));
    connect(backBtn, &QPushButton::clicked, this, &AddMusicPage::navigateBack);
    layout->addWidget(backBtn, 0, Qt::AlignLeft);

    // Title
    auto *title = new QLabel("Inserção de Músicas");
    title->setFont(Theme::titleFont(28));
    title->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::text().name()));
    layout->addWidget(title);

    auto *subtitle = new QLabel("Arraste seus arquivos de áudio ou clique para selecionar");
    subtitle->setFont(Theme::bodyFont(13));
    subtitle->setStyleSheet(QString("color: %1; background: transparent; padding-bottom: 12px;").arg(Theme::textSoft().name()));
    layout->addWidget(subtitle);

    // Drop zone
    m_dropZone = new QWidget();
    m_dropZone->setFixedHeight(180);
    m_dropZone->setCursor(Qt::PointingHandCursor);
    m_dropZone->setStyleSheet(QString(
        "QWidget { border: 2px dashed %1; border-radius: 16px; background: rgba(255,255,255,0.01); }"
    ).arg(Theme::border().name()));

    auto *dropLayout = new QVBoxLayout(m_dropZone);
    dropLayout->setAlignment(Qt::AlignCenter);
    dropLayout->setSpacing(10);

    auto *uploadIcon = new QLabel("⬆");
    uploadIcon->setFont(QFont("Segoe UI", 32));
    uploadIcon->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textMuted().name()));
    uploadIcon->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(uploadIcon);

    m_dropLabel = new QLabel("Clique ou arraste arquivos de áudio");
    m_dropLabel->setFont(Theme::bodyFont(14));
    m_dropLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textSoft().name()));
    m_dropLabel->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(m_dropLabel);

    auto *formatLabel = new QLabel("MP3, WAV, OGG, M4A, AAC, FLAC");
    formatLabel->setFont(Theme::bodyFont(11));
    formatLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textMuted().name()));
    formatLabel->setAlignment(Qt::AlignCenter);
    dropLayout->addWidget(formatLabel);

    // Make drop zone clickable
    m_dropZone->installEventFilter(this);
    // We'll handle click via mouse press
    layout->addWidget(m_dropZone);

    // Browse button
    auto *browseBtn = new QPushButton("Procurar Arquivos");
    browseBtn->setFixedSize(180, 40);
    browseBtn->setCursor(Qt::PointingHandCursor);
    browseBtn->setFont(Theme::bodyFont(13));
    browseBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: %2; border: 1px solid %3; border-radius: 20px; font-weight: 600; }"
        "QPushButton:hover { background: %4; }"
    ).arg(Theme::card().name(), Theme::textSoft().name(), Theme::border().name(), Theme::cardHover().name()));
    connect(browseBtn, &QPushButton::clicked, [this]() {
        QStringList files = QFileDialog::getOpenFileNames(this, "Selecionar Músicas", QString(),
            "Áudio (*.mp3 *.wav *.ogg *.m4a *.aac *.flac *.wma *.webm);;Todos (*.*)");
        if (!files.isEmpty()) processFiles(files);
    });
    layout->addWidget(browseBtn, 0, Qt::AlignLeft);
    layout->addSpacing(8);

    // File list container
    m_fileListContainer = new QWidget();
    m_fileListContainer->setStyleSheet("background: transparent;");
    m_fileListLayout = new QVBoxLayout(m_fileListContainer);
    m_fileListLayout->setContentsMargins(0, 0, 0, 0);
    m_fileListLayout->setSpacing(6);
    m_fileListContainer->hide();
    layout->addWidget(m_fileListContainer);

    // Folder section
    m_folderSection = new QWidget();
    m_folderSection->setStyleSheet("background: transparent;");
    auto *folderLayout = new QVBoxLayout(m_folderSection);
    folderLayout->setContentsMargins(0, 0, 0, 0);
    folderLayout->setSpacing(8);

    auto *folderLabel = new QLabel("PASTA DE DESTINO");
    folderLabel->setFont(Theme::bodyFont(11));
    folderLabel->setStyleSheet(QString("color: %1; background: transparent; font-weight: bold; letter-spacing: 1px;").arg(Theme::textSoft().name()));
    folderLayout->addWidget(folderLabel);

    auto *folderRow = new QHBoxLayout();
    folderRow->setSpacing(8);

    m_folderCombo = new QComboBox();
    m_folderCombo->setFont(Theme::bodyFont(13));
    m_folderCombo->setMinimumWidth(200);
    m_folderCombo->setStyleSheet(QString(R"(
        QComboBox {
            background: %1; color: %2; border: 1px solid %3;
            border-radius: 8px; padding: 10px 14px;
        }
        QComboBox::drop-down { border: none; width: 30px; }
        QComboBox::down-arrow { image: none; border: none; }
        QComboBox QAbstractItemView {
            background: %4; color: %2; border: 1px solid %3;
            selection-background-color: %5;
        }
    )").arg(Theme::surface().name(), Theme::text().name(), Theme::border().name(),
            Theme::card().name(), Theme::cardHover().name()));
    folderRow->addWidget(m_folderCombo);

    m_newFolderEdit = new QLineEdit();
    m_newFolderEdit->setPlaceholderText("Nova pasta...");
    m_newFolderEdit->setFont(Theme::bodyFont(13));
    m_newFolderEdit->setMinimumWidth(160);
    m_newFolderEdit->setStyleSheet(QString(R"(
        QLineEdit {
            background: %1; color: %2; border: 1px solid %3;
            border-radius: 8px; padding: 10px 14px;
        }
        QLineEdit:focus { border-color: %4; }
    )").arg(Theme::surface().name(), Theme::text().name(), Theme::border().name(), Theme::accent().name()));
    folderRow->addWidget(m_newFolderEdit);
    folderRow->addStretch();

    folderLayout->addLayout(folderRow);

    m_addBtn = new QPushButton("Adicionar à Biblioteca");
    m_addBtn->setFont(Theme::bodyFont(14));
    m_addBtn->setFixedHeight(46);
    m_addBtn->setMinimumWidth(220);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    m_addBtn->setStyleSheet(QString(
        "QPushButton { background: %1; color: %2; border: none; border-radius: 23px; font-weight: bold; padding: 0 32px; }"
        "QPushButton:hover { background: %3; }"
    ).arg(Theme::accent().name(), Theme::bg().name(), Theme::accent().lighter(110).name()));
    connect(m_addBtn, &QPushButton::clicked, this, &AddMusicPage::addAllToLibrary);
    folderLayout->addSpacing(4);
    folderLayout->addWidget(m_addBtn, 0, Qt::AlignLeft);

    m_folderSection->hide();
    layout->addWidget(m_folderSection);
    layout->addStretch();

    scroll->setWidget(content);
    outerLayout->addWidget(scroll);
}

void AddMusicPage::refresh() {
    m_folderCombo->clear();
    auto folders = m_model->folders();
    for (auto &f : folders) {
        m_folderCombo->addItem(f.name);
    }
    if (folders.isEmpty()) {
        m_folderCombo->addItem("Geral");
    }
}

void AddMusicPage::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        m_isDragOver = true;
        m_dropZone->setStyleSheet(QString(
            "QWidget { border: 2px dashed %1; border-radius: 16px; background: %2; }"
        ).arg(Theme::accent().name(), "rgba(232,164,74,0.12)"));
        m_dropLabel->setText("Solte os arquivos aqui");
    }
}

void AddMusicPage::dropEvent(QDropEvent *event) {
    m_isDragOver = false;
    m_dropZone->setStyleSheet(QString(
        "QWidget { border: 2px dashed %1; border-radius: 16px; background: rgba(255,255,255,0.01); }"
    ).arg(Theme::border().name()));
    m_dropLabel->setText("Clique ou arraste arquivos de áudio");

    QStringList paths;
    for (auto &url : event->mimeData()->urls()) {
        if (url.isLocalFile()) paths.append(url.toLocalFile());
    }
    if (!paths.isEmpty()) processFiles(paths);
}

void AddMusicPage::dragLeaveEvent(QDragLeaveEvent *) {
    m_isDragOver = false;
    m_dropZone->setStyleSheet(QString(
        "QWidget { border: 2px dashed %1; border-radius: 16px; background: rgba(255,255,255,0.01); }"
    ).arg(Theme::border().name()));
    m_dropLabel->setText("Clique ou arraste arquivos de áudio");
}

void AddMusicPage::processFiles(const QStringList &paths) {
    QStringList audioExts = {"mp3", "wav", "ogg", "m4a", "aac", "flac", "wma", "webm"};

    for (auto &path : paths) {
        QFileInfo fi(path);
        if (!audioExts.contains(fi.suffix().toLower())) continue;

        PendingFile pf;
        pf.id = QUuid::createUuid().toString(QUuid::Id128).left(10);
        pf.filePath = path;
        pf.fileSize = fi.size();
        pf.palette = Theme::randomPalette();

        // Try to parse "Artist - Title" from filename
        QString baseName = fi.completeBaseName();
        QStringList parts = baseName.split(QRegularExpression("\\s*[-–—]\\s*"));
        if (parts.size() >= 2) {
            pf.artist = parts[0].trimmed();
            pf.title = parts.mid(1).join(" - ").trimmed();
        } else {
            pf.title = baseName;
            pf.artist = "Desconhecido";
        }

        m_pendingFiles.append(pf);
    }

    refreshFileList();
}

void AddMusicPage::refreshFileList() {
    // Clear list
    QLayoutItem *item;
    while ((item = m_fileListLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    if (m_pendingFiles.isEmpty()) {
        m_fileListContainer->hide();
        m_folderSection->hide();
        return;
    }

    m_fileListContainer->show();
    m_folderSection->show();

    auto *header = new QLabel(QString("%1 arquivo%2 selecionado%3")
        .arg(m_pendingFiles.size())
        .arg(m_pendingFiles.size() > 1 ? "s" : "")
        .arg(m_pendingFiles.size() > 1 ? "s" : ""));
    header->setFont(Theme::bodyFont(14));
    header->setStyleSheet(QString("color: %1; background: transparent; font-weight: bold; padding-top: 8px;").arg(Theme::text().name()));
    m_fileListLayout->addWidget(header);

    for (int i = 0; i < m_pendingFiles.size(); ++i) {
        auto &pf = m_pendingFiles[i];

        auto *row = new QWidget();
        row->setFixedHeight(56);
        row->setStyleSheet(QString("background: %1; border-radius: 8px;").arg(Theme::card().name()));

        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(12, 6, 12, 6);
        rowLayout->setSpacing(10);

        // Color swatch
        auto *swatch = new QWidget();
        swatch->setFixedSize(36, 36);
        swatch->setStyleSheet(QString("background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 %1,stop:1 %2); border-radius: 6px;")
            .arg(pf.palette.c1.name(), pf.palette.c2.name()));
        rowLayout->addWidget(swatch);

        // Title + artist editable
        auto *infoLayout = new QVBoxLayout();
        infoLayout->setSpacing(1);

        auto *titleEdit = new QLineEdit(pf.title);
        titleEdit->setFont(Theme::bodyFont(13));
        titleEdit->setStyleSheet(QString("QLineEdit { background: transparent; border: none; color: %1; font-weight: 600; padding: 0; }").arg(Theme::text().name()));
        int idx = i;
        connect(titleEdit, &QLineEdit::textChanged, [this, idx](const QString &text) {
            if (idx < m_pendingFiles.size()) m_pendingFiles[idx].title = text;
        });
        infoLayout->addWidget(titleEdit);

        auto *artistEdit = new QLineEdit(pf.artist);
        artistEdit->setFont(Theme::bodyFont(11));
        artistEdit->setStyleSheet(QString("QLineEdit { background: transparent; border: none; color: %1; padding: 0; }").arg(Theme::textSoft().name()));
        connect(artistEdit, &QLineEdit::textChanged, [this, idx](const QString &text) {
            if (idx < m_pendingFiles.size()) m_pendingFiles[idx].artist = text;
        });
        infoLayout->addWidget(artistEdit);

        rowLayout->addLayout(infoLayout, 1);

        // Size
        auto *sizeLabel = new QLabel(QString("%1 MB").arg(pf.fileSize / (1024.0 * 1024.0), 0, 'f', 1));
        sizeLabel->setFont(Theme::monoFont(10));
        sizeLabel->setStyleSheet(QString("color: %1; background: transparent;").arg(Theme::textMuted().name()));
        rowLayout->addWidget(sizeLabel);

        // Remove button
        auto *removeBtn = new QPushButton("✕");
        removeBtn->setFixedSize(28, 28);
        removeBtn->setCursor(Qt::PointingHandCursor);
        removeBtn->setStyleSheet(QString(
            "QPushButton { background: transparent; color: %1; border: none; font-size: 14px; }"
            "QPushButton:hover { color: %2; }"
        ).arg(Theme::textMuted().name(), Theme::danger().name()));
        QString fileId = pf.id;
        connect(removeBtn, &QPushButton::clicked, [this, fileId]() {
            m_pendingFiles.erase(std::remove_if(m_pendingFiles.begin(), m_pendingFiles.end(),
                [&](const PendingFile &f) { return f.id == fileId; }), m_pendingFiles.end());
            refreshFileList();
        });
        rowLayout->addWidget(removeBtn);

        m_fileListLayout->addWidget(row);
    }

    m_addBtn->setText(QString("Adicionar %1 Música%2")
        .arg(m_pendingFiles.size())
        .arg(m_pendingFiles.size() > 1 ? "s" : ""));
}

void AddMusicPage::addAllToLibrary() {
    if (m_pendingFiles.isEmpty()) return;

    QString folder = m_newFolderEdit->text().trimmed();
    if (folder.isEmpty()) {
        folder = m_folderCombo->currentText();
    }
    if (folder.isEmpty()) folder = "Geral";

    for (auto &pf : m_pendingFiles) {
        Track t = Track::create(pf.title, pf.artist, folder, QUrl::fromLocalFile(pf.filePath));
        t.cover = pf.palette;
        m_model->addTrack(t);
        emit trackAdded(t);
    }

    m_pendingFiles.clear();
    m_newFolderEdit->clear();
    refreshFileList();
    emit navigateBack();
}