#include <QtWidgets>
#include "canvas.h"

namespace {
void pearlHistory(QListWidget* history) {
    if (!history) return;
    for (int i = 0; i < history->count(); ++i) {
        auto item = history->item(i);
        item->setSizeHint(QSize(150, 150));
        if (auto card = history->itemWidget(item)) {
            if (card->layout()) {
                card->layout()->setContentsMargins(10, 10, 10, 10);
                card->layout()->setSpacing(5);
            }
            const auto labels = card->findChildren<QLabel*>();
            for (auto label : labels) {
                if (!label->pixmap().isNull()) continue;
                const bool meta = label->objectName() == "muted";
                label->setStyleSheet(meta ? "color:#657772;font-size:11px;background:transparent;"
                                         : "color:#263A36;font-size:12px;font-weight:500;background:transparent;");
            }
        }
    }
}

class PearlHistoryObserver final : public QObject {
public:
    explicit PearlHistoryObserver(QListWidget* history) : QObject(history), list(history) {}
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::ChildAdded || event->type() == QEvent::Show) {
            if (!queued) {
                queued = true;
                QTimer::singleShot(0, this, [this] { queued = false; pearlHistory(list); });
            }
        }
        return QObject::eventFilter(watched, event);
    }
private:
    QListWidget* list;
    bool queued = false;
};
}

// finesse · product/workbench · precise drafting desk · silver + deep teal
void applyUiStudyPearl(QWidget* window) {
    if (!window) return;
    window->setProperty("studyInk", "#263A36");
    window->setProperty("studyAccent", "#285F5B");
    window->setProperty("studyOnAccent", "#F8FBF9");
    window->setProperty("studyDark", false);
    window->resize(1280, 820);
    window->setMinimumSize(900, 620);
    QFont font("Segoe UI");
    font.setPixelSize(13);
    window->setFont(font);
    QPalette palette = window->palette();
    palette.setColor(QPalette::Window, QColor("#EDF1EE"));
    palette.setColor(QPalette::WindowText, QColor("#263A36"));
    palette.setColor(QPalette::Base, QColor("#FAFCFA"));
    palette.setColor(QPalette::Text, QColor("#263A36"));
    palette.setColor(QPalette::ButtonText, QColor("#263A36"));
    palette.setColor(QPalette::Highlight, QColor("#D3E5DF"));
    palette.setColor(QPalette::HighlightedText, QColor("#204D47"));
    window->setPalette(palette);
    for (auto label : window->findChildren<QLabel*>()) label->setStyleSheet({});
    for (auto canvas : window->findChildren<Canvas*>()) {
        canvas->setPalette(palette);
        canvas->setAutoFillBackground(true);
        canvas->update();
    }
    auto root = window->findChild<QWidget*>("root");
    if (root && root->layout()) {
        root->layout()->setContentsMargins(0, 0, 0, 0);
        root->layout()->setSpacing(0);
    }
    if (auto top = window->findChild<QHBoxLayout*>("shellTop")) {
        top->setContentsMargins(25, 18, 24, 18);
        top->setSpacing(12);
    }
    if (auto header = window->findChild<QHBoxLayout*>("workHeader")) {
        header->setContentsMargins(26, 18, 24, 17);
        header->setSpacing(12);
    }
    if (auto side = window->findChild<QWidget*>("glassSidebar")) {
        side->setFixedWidth(190);
        if (side->layout()) {
            side->layout()->setContentsMargins(14, 26, 14, 16);
            side->layout()->setSpacing(12);
        }
    }
    if (auto heading = window->findChild<QLabel*>("topBrand"))
        heading->setText(QStringLiteral("萌知 <span style='font-size:12px;font-weight:400;color:#657772'> / Mengzhi</span>"));
    if (auto heading = window->findChild<QLabel*>("resultHeading"))
        heading->setText(QStringLiteral("识别结果"));
    if (auto version = window->findChild<QLabel*>("topVersion"))
        version->setText(QStringLiteral("预览版"));
    if (auto inspector = window->findChild<QWidget*>("readingPanel")) {
        inspector->setMinimumWidth(292);
        inspector->setMaximumWidth(420);
        if (inspector->layout()) {
            inspector->layout()->setContentsMargins(24, 25, 24, 18);
            inspector->layout()->setSpacing(17);
        }
    }
    if (auto split = window->findChild<QSplitter*>("canvasSplit")) {
        split->setChildrenCollapsible(false);
        split->setHandleWidth(1);
        split->setSizes({742, 348});
        split->setStretchFactor(0, 1);
        split->setStretchFactor(1, 0);
    }
    if (auto tools = window->findChild<QFrame*>("tools")) {
        if (tools->layout()) {
            tools->layout()->setContentsMargins(5, 5, 5, 5);
            tools->layout()->setSpacing(1);
        }
        for (auto button : tools->findChildren<QToolButton*>()) {
            button->setFixedSize(28, 30);
            button->setIconSize(QSize(18, 18));
        }
        tools->setGraphicsEffect(nullptr);
    }
    if (auto tabs = window->findChild<QTabWidget*>("resultTabs"))
        tabs->tabBar()->setExpanding(false);
    for (auto editor : window->findChildren<QPlainTextEdit*>()) {
        QFont mono("Cascadia Mono");
        mono.setPixelSize(13);
        editor->setFont(mono);
        editor->setPalette(palette);
    }
    if (auto history = window->findChild<QListWidget*>("history")) {
        history->setSpacing(8);
        pearlHistory(history);
        if (!history->property("pearlObserved").toBool()) {
            history->setProperty("pearlObserved", true);
            auto observer = new PearlHistoryObserver(history);
            history->viewport()->installEventFilter(observer);
        }
    }

    window->setStyleSheet(QStringLiteral(R"QSS(
QWidget { color:#263A36; font-family:"Segoe UI","Microsoft YaHei UI","Microsoft YaHei"; font-size:13px; }
QWidget#root { background:#F5F7F4; }
QLabel { background:transparent; border:0; }
QLabel#topBrand { font-size:20px; font-weight:600; }
QLabel#topVersion { font-size:11px; color:#657772; padding-left:5px; }
QLabel#muted, QLabel#engineLabel, QLabel#editorHint, QLabel#privacyHint, QLabel#localLabel { color:#657772; font-size:11px; }
QLabel#workspaceLabel, QLabel#recentLabel { color:#657772; font-size:11px; font-weight:600; }
QLabel#documentTitle { font-size:17px; font-weight:600; }
QLabel#resultHeading { font-size:18px; font-weight:600; }
QLabel#engineLabel { color:#38665E; }
QWidget#glassSidebar { background:#E9EEEA; border-top:1px solid #D7DFD9; border-right:1px solid #D7DFD9; }
QFrame#workbench { background:#FAFCFA; border:0; border-top:1px solid #D7DFD9; border-radius:0; }
QWidget#canvasArea { background:#EDF1EE; border-top:1px solid #DFE6E0; }
QWidget#readingPanel { background:#FAFCFA; border-top:1px solid #DFE6E0; }
QPushButton { background:#FAFCFA; border:1px solid #CED8D1; border-radius:5px; padding:5px 12px; min-height:26px; font-weight:500; color:#304B43; }
QPushButton:hover { background:#E9F0EB; border-color:#91ABA0; }
QPushButton:pressed { background:#DCE8E0; border-color:#638778; }
QPushButton:disabled { background:#EDF1EE; color:#929F98; border-color:#E0E6E0; }
QPushButton[primary=true], QPushButton#captureButton, QPushButton#recognizeButton { color:#F8FBF9; background:#285F5B; border:1px solid #285F5B; }
QPushButton[primary=true]:hover, QPushButton#captureButton:hover, QPushButton#recognizeButton:hover { background:#1E514B; border-color:#1E514B; }
QPushButton[primary=true]:pressed, QPushButton#captureButton:pressed, QPushButton#recognizeButton:pressed { background:#17423D; }
QPushButton#recognizeButton { min-height:33px; font-weight:600; }
QPushButton#recognizeButton:disabled, QPushButton[primary=true]:disabled { background:#D8E2DC; border-color:#D8E2DC; color:#73867B; }
QPushButton#nav { background:#DAE7DF; border:1px solid #C4D8CC; color:#285F5B; text-align:left; font-weight:600; padding-left:12px; }
QPushButton#nav:hover { background:#CDDFD3; }
QPushButton#quiet { background:transparent; border-color:transparent; font-weight:400; }
QPushButton#quiet:hover { background:#DEE8E1; border-color:#D1DDD4; }
QPushButton:focus, QToolButton:focus { border:1px solid #285F5B; }
QListWidget#history { border:0; background:transparent; outline:0; padding:0; }
QListWidget#history::item { background:transparent; border:1px solid transparent; border-radius:6px; padding:0; }
QListWidget#history::item:hover { background:#E0E8E1; border-color:#CDD9D0; }
QListWidget#history::item:selected { background:#FAFCFA; border-color:#AAC3B4; }
QFrame#tools { background:#FAFCFA; border:1px solid #CAD6CD; border-radius:7px; }
QToolButton { background:transparent; border:1px solid transparent; border-radius:4px; padding:0; }
QToolButton:hover { background:#E6EDE7; border-color:#C6D6CB; }
QToolButton:checked { background:#D3E5DC; border-color:#ABC9B8; }
QToolButton:pressed { background:#C6DACD; }
QToolButton:disabled { background:transparent; border-color:transparent; }
QTabWidget::pane { border:0; background:#FAFCFA; }
QTabBar { border:0; background:transparent; }
QTabBar::tab { border:0; border-bottom:2px solid #E1E7E1; border-radius:0; background:transparent; color:#657772; padding:9px 20px 10px 0; margin-right:20px; font-weight:500; }
QTabBar::tab:selected { color:#285F5B; border-bottom:2px solid #285F5B; font-weight:600; }
QTabBar::tab:hover { color:#204D47; }
QPlainTextEdit { background:#FAFCFA; color:#263A36; border:0; padding:10px 0; font-family:"Cascadia Mono","Consolas","Microsoft YaHei"; font-size:13px; selection-background-color:#D3E5DF; selection-color:#204D47; }
QTableWidget { background:#FAFCFA; alternate-background-color:#F0F4F0; gridline-color:#DCE5DD; border:1px solid #D7DFD9; selection-background-color:#D3E5DF; selection-color:#204D47; }
QHeaderView::section { background:#EDF2ED; color:#567166; border:0; border-bottom:1px solid #D7DFD9; padding:7px; font-size:11px; }
QSplitter::handle { background:#D7DFD9; }
QScrollBar:vertical { background:transparent; width:7px; margin:2px 0; }
QScrollBar::handle:vertical { background:#C2CFC5; border-radius:3px; min-height:30px; }
QScrollBar::handle:vertical:hover { background:#93AB9B; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:transparent; }
QScrollBar:horizontal { background:transparent; height:7px; }
QScrollBar::handle:horizontal { background:#C2CFC5; border-radius:3px; min-width:30px; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0; }
QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background:transparent; }
QMenu, QDialog { background:#F5F7F4; color:#263A36; }
QMenu { border:1px solid #CED8D1; padding:5px; }
QMenu::item { padding:8px 22px; }
QMenu::item:selected { background:#DAE7DF; }
QToolTip { color:#F8FBF9; background:#2C4A40; border:0; padding:6px 9px; }
QLineEdit, QComboBox, QKeySequenceEdit { background:#FAFCFA; border:1px solid #CED8D1; border-radius:5px; padding:7px; }
QLineEdit:focus, QComboBox:focus, QKeySequenceEdit:focus { border-color:#285F5B; }
)QSS"));
}
