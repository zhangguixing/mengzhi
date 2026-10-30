#include "ui_study.h"
#include "canvas.h"
#include <QtWidgets>

namespace {
// Keep the real history items, selection model and thumbnails; only reduce their furniture.
void compactFolioHistory(QListWidget* history) {
    if (!history) return;
    for (int i = 0; i < history->count(); ++i) {
        auto item = history->item(i);
        item->setSizeHint(QSize(126, 116));
        auto card = history->itemWidget(item);
        if (!card) continue;
        if (auto layout = card->layout()) {
            layout->setContentsMargins(9, 9, 9, 8);
            layout->setSpacing(5);
        }
        for (auto label : card->findChildren<QLabel*>()) {
            label->setStyleSheet(QString());
            if (!label->pixmap().isNull()) {
                label->setFixedHeight(58);
                label->setPixmap(label->pixmap().scaled(108, 58, Qt::KeepAspectRatio,
                                                       Qt::SmoothTransformation));
            }
        }
    }
}
}

void applyUiStudyFolio(QWidget* window) {
    window->setProperty("studyInk", "#353635");
    window->setProperty("studyAccent", "#943F33");
    window->setProperty("studyOnAccent", "#FAF9F7");
    window->setProperty("studyDark", false);

    QFont ui("Microsoft YaHei UI");
    ui.setPixelSize(12);
    window->setFont(ui);
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#EFEEEC"));
    palette.setColor(QPalette::WindowText, QColor("#353635"));
    palette.setColor(QPalette::Base, QColor("#FCFBF9"));
    palette.setColor(QPalette::AlternateBase, QColor("#F2F1EE"));
    palette.setColor(QPalette::Text, QColor("#353635"));
    palette.setColor(QPalette::Button, QColor("#F8F7F4"));
    palette.setColor(QPalette::ButtonText, QColor("#353635"));
    palette.setColor(QPalette::Highlight, QColor("#EEDDD7"));
    palette.setColor(QPalette::HighlightedText, QColor("#6D3028"));
    palette.setColor(QPalette::PlaceholderText, QColor("#737471"));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#989993"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#989993"));
    window->setPalette(palette);

    for (auto label : window->findChildren<QLabel*>()) label->setStyleSheet(QString());
    auto root = window->findChild<QWidget*>("root");
    if (root && root->layout()) {
        root->layout()->setContentsMargins(0, 0, 16, 14);
        root->layout()->setSpacing(0);
    }
    if (auto top = window->findChild<QHBoxLayout*>("shellTop")) {
        top->setContentsMargins(22, 17, 8, 17);
        top->setSpacing(10);
    }
    if (auto body = window->findChild<QHBoxLayout*>("shellBody")) body->setSpacing(8);
    if (auto side = window->findChild<QWidget*>("glassSidebar")) {
        side->setFixedWidth(154);
        side->layout()->setContentsMargins(12, 19, 10, 5);
        side->layout()->setSpacing(8);
    }
    if (auto header = window->findChild<QHBoxLayout*>("workHeader")) {
        header->setContentsMargins(23, 17, 17, 17);
        header->setSpacing(12);
    }
    if (auto label = window->findChild<QLabel*>("workspaceLabel")) {
        label->setText(QStringLiteral("工作空间"));
        label->setContentsMargins(7, 0, 0, 5);
    }
    if (auto label = window->findChild<QLabel*>("recentLabel")) {
        label->setText(QStringLiteral("最近文档"));
        label->setContentsMargins(7, 22, 0, 4);
    }
    if (auto label = window->findChild<QLabel*>("localLabel")) {
        label->setText(QStringLiteral("本机处理 · 离线可用"));
        label->setContentsMargins(7, 0, 0, 9);
    }
    auto reading = window->findChild<QWidget*>("readingPanel");
    auto reference = window->findChild<QWidget*>("canvasArea");
    if (reading) {
        reading->setMaximumWidth(QWIDGETSIZE_MAX);
        reading->setMinimumWidth(285);
        reading->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        reading->layout()->setContentsMargins(24, 22, 24, 17);
        reading->layout()->setSpacing(14);
    }
    if (reference) {
        reference->setMinimumWidth(420);
        auto labels = reference->findChildren<QLabel*>("muted", Qt::FindDirectChildrenOnly);
        if (!labels.isEmpty()) {
            labels.first()->setText(QStringLiteral("原图参照"));
            labels.first()->setContentsMargins(20, 23, 0, 0);
        }
    }
    if (auto split = window->findChild<QSplitter*>("canvasSplit")) {
        if (reading) split->insertWidget(0, reading);
        split->setHandleWidth(1);
        split->setChildrenCollapsible(false);
        split->setStretchFactor(0, 52);
        split->setStretchFactor(1, 48);
        split->setSizes({550, 510});
    }
    if (auto canvas = window->findChild<Canvas*>()) {
        auto canvasPalette = palette;
        canvasPalette.setColor(QPalette::Window, QColor("#EEEDE9"));
        canvas->setPalette(canvasPalette);
    }
    if (auto tools = window->findChild<QFrame*>("tools")) {
        tools->layout()->setContentsMargins(5, 5, 5, 5);
        tools->layout()->setSpacing(1);
        for (auto button : tools->findChildren<QToolButton*>()) {
            button->setFixedSize(28, 28);
            button->setIconSize(QSize(17, 17));
        }
    }
    if (auto editor = window->findChild<QPlainTextEdit*>("result")) {
        QFont mono("Cascadia Mono");
        mono.setStyleHint(QFont::Monospace);
        mono.setPixelSize(14);
        editor->setFont(mono);
        editor->setContentsMargins(0, 5, 0, 0);
        editor->document()->setDocumentMargin(2);
    }
    if (auto recognize = window->findChild<QPushButton*>("recognizeButton")) {
        recognize->setProperty("primary", true);
        recognize->setMinimumHeight(40);
    }
    for (const auto& id : {"captureButton", "exportButton"}) {
        if (auto button = window->findChild<QPushButton*>(id)) button->setProperty("primary", false);
    }
    if (auto history = window->findChild<QListWidget*>("history")) {
        history->setSpacing(5);
        compactFolioHistory(history);
        QObject::connect(history->model(), &QAbstractItemModel::rowsInserted, history,
                         [history] { QTimer::singleShot(0, history, [history] { compactFolioHistory(history); }); });
    }

    window->setStyleSheet(QString::fromUtf8(R"QSS(
QWidget { color:#353635; font-family:"Microsoft YaHei UI","Segoe UI"; font-size:12px; }
QMainWindow, QWidget#root { background:#EFEEEC; }
QLabel { background:transparent; border:0; }
QLabel#topBrand { font-size:19px; font-weight:600; color:#30322F; }
QLabel#topVersion { color:#777973; font-size:10px; padding-left:2px; }
QWidget#glassSidebar { background:transparent; }
QLabel#workspaceLabel, QLabel#recentLabel { color:#6C6E68; font-size:10px; font-weight:600; }
QLabel#localLabel { color:#71736C; font-size:10px; }
QFrame#workbench { background:#F8F7F4; border:1px solid #DDDEDA; border-radius:9px; }
QLabel#documentTitle { color:#393B36; font-size:14px; font-weight:600; }
QWidget#readingPanel { background:#FCFBF9; border-top:1px solid #E3E3DE; }
QWidget#canvasArea { background:#EEEDE9; border-top:1px solid #E0E0DA; }
QSplitter::handle { background:#DEDFD9; }
QSplitter::handle:hover { background:#943F33; }
QLabel#resultHeading { color:#343631; font-size:17px; font-weight:600; }
QLabel#engineLabel { color:#797B73; font-size:10px; }
QLabel#muted, QLabel#editorHint, QLabel#privacyHint { color:#72756C; font-size:10px; }
QLabel#editorHint { padding:2px 0 9px 0; }
QLabel#privacyHint { padding-top:2px; }
QPushButton { min-height:28px; padding:2px 12px; border:1px solid #D8DAD3; border-radius:5px; background:#F8F7F4; font-weight:500; }
QPushButton:hover { background:#EAE9E4; border-color:#BEC1B7; }
QPushButton:pressed { background:#DDDFD7; }
QPushButton:focus { border-color:#943F33; }
QPushButton:disabled { color:#989B92; background:#EFEEE9; border-color:#E1E2DC; }
QPushButton#quiet { border-color:transparent; background:transparent; padding:2px 7px; }
QPushButton#quiet:hover { background:#E5E5DF; }
QPushButton#nav { color:#703B31; background:#E9DDD6; border:0; padding:4px 8px; font-size:11px; font-weight:600; }
QPushButton#nav:hover { background:#E1D0C7; }
QPushButton#recognizeButton { color:#FAF9F7; background:#943F33; border:1px solid #943F33; font-weight:600; }
QPushButton#recognizeButton:hover { background:#80372D; border-color:#80372D; }
QPushButton#recognizeButton:pressed { background:#6B3028; }
QPushButton#recognizeButton:disabled { background:#C4A59D; border-color:#C4A59D; color:#F7F3F0; }
QPushButton#exportButton { color:#7B392F; border-color:#C6ADA3; background:#F4ECE6; }
QPushButton#captureButton { border-color:#A8AAA0; background:#F8F7F4; }
QListWidget#history { background:transparent; border:0; outline:0; }
QListWidget#history::item { padding:0; border:1px solid transparent; border-radius:5px; }
QListWidget#history::item:hover { background:#E7E6E1; }
QListWidget#history::item:selected { background:#F9F8F5; border:1px solid #C4B0A5; }
QListWidget#history QLabel { font-size:10px; }
QListWidget#history QLabel#muted { color:#74766D; font-size:9px; }
QFrame#tools { background:#F8F7F4; border:1px solid #D6D9D0; border-radius:6px; }
QFrame#tools QToolButton { background:transparent; border:1px solid transparent; border-radius:3px; }
QFrame#tools QToolButton:hover { background:#E8E8E0; }
QFrame#tools QToolButton:checked { background:#E9D8D0; border-color:#C4A298; }
QFrame#tools QToolButton:disabled { background:transparent; border-color:transparent; }
QTabWidget#resultTabs { background:transparent; }
QTabWidget::pane { border:0; border-top:1px solid #E1E2DB; background:transparent; }
QTabBar::tab { background:transparent; color:#73766B; padding:10px 18px 11px 0; margin-right:20px; border-bottom:2px solid transparent; font-size:12px; }
QTabBar::tab:selected { color:#883F33; border-bottom:2px solid #943F33; font-weight:600; }
QTabBar::tab:hover { color:#943F33; }
QPlainTextEdit#result { background:#FCFBF9; color:#363933; border:0; selection-background-color:#E9D6CD; selection-color:#613229; font-family:"Cascadia Mono","Consolas"; font-size:14px; }
QTableWidget { background:#FCFBF9; alternate-background-color:#F1F1EB; gridline-color:#DFE1D8; border:1px solid #DFE1D8; selection-background-color:#EAD9D0; selection-color:#613229; }
QHeaderView::section { background:#EDEEE7; padding:7px; border:0; border-right:1px solid #DDDFD5; border-bottom:1px solid #DDDFD5; font-size:11px; }
QComboBox { background:#F8F7F4; border:1px solid #D7DACF; border-radius:4px; padding:4px 10px; min-height:23px; }
QComboBox QAbstractItemView { background:#F8F7F4; selection-background-color:#EAD9D0; }
QScrollBar:vertical { background:transparent; width:7px; margin:1px; }
QScrollBar:horizontal { background:transparent; height:7px; margin:1px; }
QScrollBar::handle { background:#C5C8BD; border-radius:3px; min-height:24px; min-width:24px; }
QScrollBar::handle:hover { background:#9CA18F; }
QScrollBar::add-line, QScrollBar::sub-line { width:0; height:0; }
QScrollBar::add-page, QScrollBar::sub-page { background:transparent; }
QToolTip { background:#343832; color:#F7F7F1; border:0; padding:7px; }
)QSS"));
}
