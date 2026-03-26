#ifndef THEME_H
#define THEME_H

#include <QString>
#include <QColor>
#include <QFont>
#include <QRandomGenerator>
#include <QStringList>

namespace Theme {

// ─── Colors ── warm vinyl / analog aesthetic ─────────────────────
inline QColor bg()          { return QColor("#1a1712"); }
inline QColor surface()     { return QColor("#221f19"); }
inline QColor card()        { return QColor("#2a2620"); }
inline QColor cardHover()   { return QColor("#342f28"); }
inline QColor accent()      { return QColor("#e8a44a"); }
inline QColor accentDim()   { return QColor("#c4883a"); }
inline QColor text()        { return QColor("#f0ece4"); }
inline QColor textSoft()    { return QColor("#b8b0a2"); }
inline QColor textMuted()   { return QColor("#7a7266"); }
inline QColor border()      { return QColor("#3a352d"); }
inline QColor danger()      { return QColor("#d45d5d"); }
inline QColor vinylBlack()  { return QColor("#111111"); }

// ─── Gradients (as pairs of colors for covers) ──────────────────
struct GradientPair {
    QColor c1, c2;
};

inline QList<GradientPair> palettes() {
    return {
        { QColor("#e8a44a"), QColor("#d45d5d") },
        { QColor("#6b8f71"), QColor("#2c3e50") },
        { QColor("#c9a959"), QColor("#8b5e3c") },
        { QColor("#5d7b93"), QColor("#2b2d42") },
        { QColor("#d4a373"), QColor("#9c6644") },
        { QColor("#b07156"), QColor("#4a3228") },
        { QColor("#a3b18a"), QColor("#344e41") },
        { QColor("#dda15e"), QColor("#bc6c25") },
        { QColor("#8ecae6"), QColor("#023047") },
        { QColor("#cdb4db"), QColor("#5a189a") },
    };
}

inline GradientPair randomPalette() {
    auto p = palettes();
    return p[QRandomGenerator::global()->bounded(p.size())];
}

// ─── Fonts ───────────────────────────────────────────────────────
inline QFont titleFont(int size = 28) {
    QFont f("Segoe UI", size);
    f.setWeight(QFont::Black);
    return f;
}

inline QFont bodyFont(int size = 14) {
    QFont f("Segoe UI", size);
    f.setWeight(QFont::Medium);
    return f;
}

inline QFont monoFont(int size = 11) {
    QFont f("Consolas", size);
    f.setStyleHint(QFont::Monospace);
    return f;
}

inline QFont iconFont(int size = 14) {
    return QFont("Segoe MDL2 Assets", size);
}

// ─── Global stylesheet ──────────────────────────────────────────
inline QString globalStyleSheet() {
    return R"(
        QWidget {
            background-color: #1a1712;
            color: #f0ece4;
            font-family: "Segoe UI", "Noto Sans", sans-serif;
        }
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #3a352d;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #7a7266;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: transparent;
        }
        QScrollBar:horizontal {
            height: 0px;
        }
        QToolTip {
            background-color: #2a2620;
            color: #f0ece4;
            border: 1px solid #3a352d;
            padding: 4px 8px;
            border-radius: 4px;
        }
    )";
}

// ─── Helpers ─────────────────────────────────────────────────────
inline QString formatTime(qint64 ms) {
    int totalSec = static_cast<int>(ms / 1000);
    int min = totalSec / 60;
    int sec = totalSec % 60;
    return QString("%1:%2").arg(min).arg(sec, 2, 10, QChar('0'));
}

}  // namespace Theme

#endif // THEME_H