#include <QtWidgets>

// finesse · product workbench · approved image reference · graphite / icy blue
void applyUiStudyGraphite(QWidget* window)
{
    if (!window || window->property("graphiteStudyApplied").toBool()) return;
    auto root = window->findChild<QWidget*>("root");
    if (!root && window->objectName() == "root") root = window;
    if (!root) return;
    auto shell = qobject_cast<QVBoxLayout*>(root->layout());
    auto work = root->findChild<QFrame*>("workbench");
    auto side = root->findChild<QWidget*>("glassSidebar");
    auto history = root->findChild<QListWidget*>("history");
    auto split = root->findChild<QSplitter*>("canvasSplit");
    auto area = root->findChild<QWidget*>("canvasArea");
    auto panel = root->findChild<QWidget*>("readingPanel");
    if (!shell || !work || !side || !history || !split || !area || !panel) return;
    window->setProperty("graphiteStudyApplied", true);
    for (auto target : {window, root}) {
        target->setProperty("studyInk", QString("#e8ecf1"));
        target->setProperty("studyAccent", QString("#8bbce5"));
        target->setProperty("studyOnAccent", QString("#152331"));
        target->setProperty("studyDark", true);
    }
    window->setMinimumSize(900, 620);
    window->resize(1360, 860);
    QFont base("Microsoft YaHei UI"); base.setPixelSize(13); window->setFont(base);
    shell->setContentsMargins(20, 0, 20, 0);
    shell->setSpacing(0);
    auto top = root->findChild<QHBoxLayout*>("shellTop");
    auto body = root->findChild<QHBoxLayout*>("shellBody");
    auto workLayout = qobject_cast<QVBoxLayout*>(work->layout());
    auto left = qobject_cast<QVBoxLayout*>(area->layout());
    auto sourceRow = root->findChild<QHBoxLayout*>("workHeader");
    if (!top || !body || !workLayout || !left || !sourceRow) return;

    // The header contains only the brand and the three frequently used actions.
    top->setContentsMargins(8, 16, 0, 15);
    top->setSpacing(10);
    if (auto mark = qobject_cast<QLabel*>(top->itemAt(0)->widget())) {
        const auto pixmap = mark->pixmap();
        if (!pixmap.isNull()) mark->setPixmap(pixmap.scaled(38, 38, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (auto brand = root->findChild<QLabel*>("topBrand")) {
        brand->setText(QStringLiteral("萌知"));
        brand->setStyleSheet("font-size:23px;font-weight:600;color:#f0f3f7;padding-left:3px;");
    }
    if (auto descriptor = root->findChild<QLabel*>("topVersion")) {
        descriptor->setText(QStringLiteral("│  截图与识别"));
        descriptor->setStyleSheet("font-size:15px;color:#b3bbc5;padding-left:2px;");
    }
    QPushButton* focus = nullptr;
    QPushButton* import = nullptr;
    for (int i = 0; i < top->count(); ++i) {
        auto b = qobject_cast<QPushButton*>(top->itemAt(i)->widget());
        if (!b) continue;
        b->setMinimumHeight(39);
        if (b->text().contains(QStringLiteral("主题"))) focus = b;
        if (b->text().contains(QStringLiteral("导入"))) import = b;
    }
    if (focus) {
        focus->setObjectName("focusCanvasButton");
        focus->setText(QStringLiteral("专注模式"));
        focus->setToolTip(QStringLiteral("展开图像工作区"));
        focus->setProperty("glyph", "focus");
        focus->setCheckable(true);
        if (import) {
            int index = top->indexOf(focus);
            top->removeWidget(import);
            top->insertWidget(index, import);
        }
    }
    auto capture = root->findChild<QPushButton*>("captureButton");
    if (capture) {
        capture->setProperty("primary", true);
        capture->setProperty("graphitePrimary", true);
        capture->setMinimumWidth(119);
    }
    body->removeWidget(side);
    body->setContentsMargins(0, 0, 0, 11);
    body->setSpacing(0);
    side->hide();
    workLayout->setContentsMargins(0, 0, 0, 0);
    workLayout->setSpacing(0);

    // Rehouse the existing source header inside the source panel. The result header
    // now starts at the same height, rather than sitting beneath the document name.
    QHBoxLayout* statusRow = nullptr;
    for (int i = 0; i < workLayout->count(); ++i) {
        auto candidate = qobject_cast<QHBoxLayout*>(workLayout->itemAt(i)->layout());
        if (candidate && candidate != sourceRow) statusRow = candidate;
    }
    auto dimensions = root->findChild<QLabel*>("dimensionsLabel");
    if (!dimensions && statusRow && statusRow->count() > 1)
        dimensions = qobject_cast<QLabel*>(statusRow->itemAt(1)->widget());
    if (dimensions && statusRow) statusRow->removeWidget(dimensions);
    workLayout->removeItem(sourceRow);
    auto sourceHead = new QFrame(area);
    sourceHead->setObjectName("sourceHeader");
    sourceHead->setFixedHeight(68);
    auto sourceLayout = new QVBoxLayout(sourceHead);
    sourceLayout->setContentsMargins(17, 11, 16, 9);
    sourceLayout->setSpacing(4);
    sourceRow->setContentsMargins(0, 0, 0, 0);
    sourceRow->setSpacing(10);
    for (int i = 0; i < sourceRow->count(); ++i)
        if (auto b = qobject_cast<QPushButton*>(sourceRow->itemAt(i)->widget())) b->setFixedSize(28, 28);
    if (auto title = root->findChild<QLabel*>("documentTitle")) {
        title->setStyleSheet("font-size:16px;font-weight:600;color:#edf1f5;");
        title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    }
    sourceLayout->addLayout(sourceRow);
    if (dimensions) {
        dimensions->setObjectName("dimensionsLabel");
        dimensions->setStyleSheet("font-size:12px;color:#b4beca;");
        sourceLayout->addWidget(dimensions);
    }
    if (auto oldTitle = qobject_cast<QLabel*>(left->itemAt(0)->widget())) {
        left->removeWidget(oldTitle);
        oldTitle->hide();
    }
    left->insertWidget(0, sourceHead);
    left->setContentsMargins(1, 1, 1, 1);
    left->setSpacing(0);
    split->setHandleWidth(12);
    split->setChildrenCollapsible(false);
    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 0);
    split->setSizes({910, 398});
    area->setMinimumWidth(450);
    panel->setMinimumWidth(302);
    panel->setMaximumWidth(440);
    if (auto resultLayout = qobject_cast<QVBoxLayout*>(panel->layout())) {
        resultLayout->setContentsMargins(17, 20, 17, 17);
        resultLayout->setSpacing(13);
    }
    if (auto heading = root->findChild<QLabel*>("resultHeading"))
        heading->setStyleSheet("font-size:18px;font-weight:600;color:#f0f3f6;");
    if (auto engine = root->findChild<QLabel*>("engineLabel"))
        engine->setStyleSheet("font-size:12px;color:#d1d8e1;");

    // Keep the exact text geometry and editing behavior. Move the explanatory hint
    // into the tab bar, leaving the editor itself a clean, dark paper surface.
    auto tabs = root->findChild<QTabWidget*>("resultTabs");
    if (tabs) {
        tabs->setStyleSheet(QString());
        auto preserve = new QLabel(QStringLiteral("保留行列与缩进"), tabs);
        preserve->setObjectName("preserveHint");
        preserve->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        preserve->setContentsMargins(0, 0, 0, 4);
        tabs->setCornerWidget(preserve, Qt::TopRightCorner);
        for (int i = 0; i < tabs->count(); ++i) {
            auto page = tabs->widget(i);
            if (page->layout()) page->layout()->setContentsMargins(0, 13, 0, 0);
        }
    }
    if (auto hint = root->findChild<QLabel*>("editorHint")) {
        hint->hide();
        if (auto editor = root->findChild<QPlainTextEdit*>("result"))
            editor->setToolTip(hint->text());
    }
    auto editor = root->findChild<QPlainTextEdit*>("result");
    if (editor) {
        editor->setStyleSheet(QString());
        editor->setLineWrapMode(QPlainTextEdit::NoWrap);
        QFont f("Microsoft YaHei UI"); f.setPixelSize(14); editor->setFont(f);
        // The table tab already provides this action; avoid a second in-editor CTA.
        for (auto b : editor->parentWidget()->findChildren<QPushButton*>()) {
            b->setMinimumHeight(0);
            b->setMaximumHeight(0);
            b->setFocusPolicy(Qt::NoFocus);
        }
    }
    auto recognize = root->findChild<QPushButton*>("recognizeButton");
    if (recognize) {
        recognize->setText(QStringLiteral("识别文字"));
        recognize->setProperty("primary", true);
        recognize->setProperty("graphitePrimary", true);
        recognize->setFixedHeight(44);
    }
    for (auto id : {"copyResultButton", "exportButton"}) {
        if (auto button = root->findChild<QPushButton*>(id)) {
            button->setProperty("primary", false);
            button->setProperty("graphitePrimary", false);
            button->setMinimumHeight(42);
        }
    }
    if (auto privacy = root->findChild<QLabel*>("privacyHint")) {
        privacy->setText(QStringLiteral("离线识别 · 图片无需上传"));
        privacy->setContentsMargins(0, 10, 0, 2);
    }

    // The mark controls remain real tools; options appear only while drawing.
    auto toolFrame = root->findChild<QFrame*>("tools");
    if (toolFrame) {
        if (auto l = toolFrame->layout()) {
            l->setContentsMargins(6, 4, 6, 4);
            l->setSpacing(2);
        }
        for (auto b : toolFrame->findChildren<QToolButton*>()) b->setFixedSize(32, 32);
        for (int i = 0; i < left->count(); ++i) {
            auto row = qobject_cast<QHBoxLayout*>(left->itemAt(i)->layout());
            if (row && row->indexOf(toolFrame) >= 0) row->setContentsMargins(8, 0, 8, 5);
        }
    }
    QHBoxLayout* options = nullptr;
    QHBoxLayout* footer = nullptr;
    for (int i = 0; i < left->count(); ++i) {
        auto row = qobject_cast<QHBoxLayout*>(left->itemAt(i)->layout());
        if (!row) continue;
        for (int j = 0; j < row->count(); ++j) {
            if (qobject_cast<QComboBox*>(row->itemAt(j)->widget())) options = row;
            if (auto b = qobject_cast<QPushButton*>(row->itemAt(j)->widget()))
                if (b->property("glyph").toString() == "minus") footer = row;
        }
    }
    if (options) {
        int index = 0;
        for (int i = 0; i < left->count(); ++i)
            if (left->itemAt(i)->layout() == options) index = i;
        left->removeItem(options);
        auto optionsWidget = new QWidget(area);
        optionsWidget->setObjectName("annotationOptions");
        optionsWidget->setLayout(options);
        options->setContentsMargins(17, 0, 17, 5);
        left->insertWidget(index, optionsWidget);
        optionsWidget->hide();
        if (toolFrame) {
            auto refreshOptions = [toolFrame, optionsWidget] {
                bool drawing = false;
                for (auto b : toolFrame->findChildren<QToolButton*>())
                    if (b->isCheckable() && b->isChecked() && b->property("glyph").toString() != "move") drawing = true;
                optionsWidget->setVisible(drawing);
            };
            for (auto b : toolFrame->findChildren<QToolButton*>())
                if (b->isCheckable()) QObject::connect(b, &QToolButton::toggled, optionsWidget, refreshOptions);
        }
    }
    if (footer) {
        footer->setContentsMargins(16, 1, 14, 10);
        footer->setSpacing(7);
        auto fit = new QPushButton(QStringLiteral("适应窗口"), area);
        fit->setObjectName("fitImageButton");
        fit->setProperty("glyph", "fit");
        fit->setCursor(Qt::PointingHandCursor);
        fit->setToolTip(QStringLiteral("使图像完整显示在当前窗口"));
        footer->insertWidget(3, fit);
        for (int i = 0; i < footer->count(); ++i) {
            auto w = footer->itemAt(i)->widget();
            if (auto b = qobject_cast<QPushButton*>(w)) {
                b->setMinimumHeight(28);
                b->setMaximumHeight(28);
                const auto glyph = b->property("glyph").toString();
                if (glyph == "minus" || glyph == "plus") b->setFixedWidth(32);
            }
        }
    }
    for (auto child : area->findChildren<QWidget*>()) {
        if (QString(child->metaObject()->className()) == "Canvas") {
            auto p = child->palette(); p.setColor(QPalette::Window, QColor("#20242a"));
            child->setPalette(p);
            child->setProperty("studyCanvasInset", 24);
            child->setMinimumSize(260, 150);
            child->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
    }

    auto dock = new QFrame(root);
    dock->setObjectName("graphiteFilmDock");
    dock->setFixedHeight(137);
    auto dockLayout = new QVBoxLayout(dock);
    dockLayout->setContentsMargins(16, 7, 16, 5);
    dockLayout->setSpacing(3);
    auto dockHead = new QHBoxLayout;
    dockHead->setSpacing(10);
    auto recent = new QLabel(QStringLiteral("最近截图"), dock);
    recent->setObjectName("filmTitle");
    dockHead->addWidget(recent);
    dockHead->addStretch();
    const auto sideButtons = side->findChildren<QPushButton*>();
    for (auto b : sideButtons) {
        if (!b->text().contains(QStringLiteral("偏好"))) continue;
        side->layout()->removeWidget(b);
        b->setParent(dock);
        b->setObjectName("filmPreferences");
        b->setMinimumHeight(23);
        b->setMaximumHeight(23);
        dockHead->addWidget(b);
    }
    dockLayout->addLayout(dockHead);
    side->layout()->removeWidget(history);
    history->setParent(dock);
    history->setFlow(QListView::LeftToRight);
    history->setWrapping(false);
    history->setResizeMode(QListView::Adjust);
    history->setMovement(QListView::Static);
    history->setViewMode(QListView::IconMode);
    history->setSelectionMode(QAbstractItemView::SingleSelection);
    history->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    history->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    history->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    history->setSpacing(6);
    history->setGridSize(QSize(134, 97));
    history->setMinimumHeight(97);
    dockLayout->addWidget(history, 1);
    auto arrangeCards = [history] {
        for (int i = 0; i < history->count(); ++i) {
            auto item = history->item(i);
            item->setSizeHint(QSize(128, 94));
            auto card = history->itemWidget(item);
            if (!card) continue;
            if (card->layout()) {
                card->layout()->setContentsMargins(6, 3, 6, 2);
                card->layout()->setSpacing(1);
                card->layout()->setAlignment(Qt::AlignTop);
            }
            const auto labels = card->findChildren<QLabel*>();
            if (!labels.isEmpty()) {
                labels.first()->setFixedHeight(50);
                const auto thumbnail = labels.first()->pixmap();
                if (!thumbnail.isNull()) labels.first()->setPixmap(thumbnail.scaled(116,50,Qt::KeepAspectRatio,Qt::SmoothTransformation));
            }
            for (int k = 1; k < labels.size(); ++k) {
                labels[k]->setFixedHeight(15);
                const auto fullText = labels[k]->toolTip().isEmpty() ? labels[k]->text() : labels[k]->toolTip();
                labels[k]->setToolTip(fullText);
                labels[k]->setText(QFontMetrics(labels[k]->font()).elidedText(fullText, Qt::ElideRight, 112));
                labels[k]->setStyleSheet(k == 1 ? "color:#b8dcf7;font-size:11px;" : "color:#b9c3ce;font-size:11px;");
                if (k > 1) labels[k]->hide();
            }
        }
        history->doItemsLayout();
    };
    arrangeCards();
    QObject::connect(history->model(), &QAbstractItemModel::rowsInserted, history,
                     [history, arrangeCards] { QTimer::singleShot(0, history, arrangeCards); });
    shell->addWidget(dock);
    // Keep the original status label live and add the reference's quiet shortcut rail.
    if (statusRow) {
        workLayout->removeItem(statusRow);
        statusRow->setContentsMargins(1, 4, 1, 4);
        auto keys = new QLabel(QStringLiteral("新截图  F1    │    识别文字  Ctrl + Enter    │    复制结果  Ctrl + C    │    导出文件  Ctrl + S"), root);
        keys->setObjectName("shortcutHint");
        keys->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        statusRow->addWidget(keys);
        shell->addLayout(statusRow);
    }
    auto p = window->palette();
    p.setColor(QPalette::Window, QColor("#181b20"));
    p.setColor(QPalette::WindowText, QColor("#e8ecf1"));
    p.setColor(QPalette::Base, QColor("#20242a"));
    p.setColor(QPalette::Text, QColor("#e8ecf1"));
    p.setColor(QPalette::Button, QColor("#292e35"));
    p.setColor(QPalette::ButtonText, QColor("#e8ecf1"));
    p.setColor(QPalette::Highlight, QColor("#38566e"));
    p.setColor(QPalette::HighlightedText, QColor("#eef7ff"));
    window->setPalette(p);
    window->setStyleSheet(QStringLiteral(R"QSS(
        /* finesse · register=product · shell=source+inspector+filmstrip · palette=graphite+icy-blue · motion=native-feedback */
        QWidget { color:#e8ecf1; font-family:'Microsoft YaHei UI'; font-size:13px; }
        QWidget#root { background:#181b20; }
        QLabel { background:transparent; border:0; }
        QLabel#muted, QLabel#privacyHint, QLabel#editorHint { color:#adb7c4; font-size:11px; }
        QLabel#preserveHint { color:#b4beca; font-size:11px; }
        QLabel#shortcutHint { color:#aeb7c3; font-size:11px; }
        QLabel#filmTitle { color:#dde4ed; font-size:12px; }
        QFrame#workbench { background:transparent; border:0; border-radius:0; }
        QWidget#canvasArea { background:#20242a; border:1px solid #3c424b; border-radius:5px; }
        QFrame#sourceHeader { background:#20242a; border:0; border-bottom:1px solid #3a414a; border-radius:0; }
        QWidget#readingPanel { background:#272c33; border:1px solid #3c434c; border-radius:5px; }
        QFrame#graphiteFilmDock { background:#242930; border:1px solid #3c434c; border-radius:5px; }
        QWidget#annotationOptions { background:#20242a; border:0; }
        QFrame#tools { background:#30363e; border:1px solid #4a535d; border-radius:6px; }
        QPushButton { background:#292e35; color:#e8edf3; border:1px solid #515b67;
                      border-radius:5px; padding:5px 12px; font-size:13px; font-weight:500; }
        QPushButton:hover { background:#353e49; border-color:#8092a5; }
        QPushButton:pressed { background:#1e252e; border-color:#8bbce5; }
        QPushButton:focus, QToolButton:focus, QComboBox:focus { border:1px solid #8bbce5; }
        QPushButton:disabled { background:#282e35; color:#929eac; border-color:#3e4854; }
        QPushButton#quiet, QPushButton#filmPreferences { background:transparent; border-color:transparent; padding:4px 7px; }
        QPushButton#quiet:hover, QPushButton#filmPreferences:hover { background:#343d48; border-color:#536478; }
        QPushButton#filmPreferences { font-size:11px; }
        QPushButton#fitImageButton { font-size:12px; padding:3px 9px; }
        QPushButton[graphitePrimary="true"] { background:#8bbce5; color:#152331; border-color:#9bcdf2; font-weight:600; }
        QPushButton[graphitePrimary="true"]:hover { background:#a3d1f3; border-color:#b5dcf8; }
        QPushButton[graphitePrimary="true"]:pressed { background:#72a7d0; border-color:#8bbce5; }
        QPushButton[graphitePrimary="true"]:disabled { background:#42627d; color:#d2dee8; border-color:#577994; }
        QToolButton { background:transparent; border:1px solid transparent; border-radius:4px; padding:0; }
        QToolButton:hover { background:#414b57; }
        QToolButton:checked { background:#354f65; border-color:#456e8e; }
        QToolButton:disabled { background:transparent; }
        QComboBox { background:#2b323b; border:1px solid #505c69; border-radius:4px; padding:4px 9px; min-width:52px; }
        QComboBox::drop-down { border:0; width:17px; }
        QComboBox QAbstractItemView { background:#2b333d; color:#edf3fa; selection-background-color:#38566e; }
        QSplitter::handle { background:#181b20; border:0; }
        QTabWidget::pane { border:0; background:transparent; }
        QTabBar { background:transparent; border:0; }
        QTabBar::tab { background:transparent; border:0; border-radius:0; border-bottom:2px solid transparent;
                        padding:9px 17px; color:#afb9c6; margin-right:5px; font-size:14px; }
        QTabBar::tab:selected { color:#99d4fc; border-bottom-color:#86ccf8; }
        QTabBar::tab:hover { color:#d6efff; background:#303945; }
        QPlainTextEdit#result { background:#1f242b; color:#eef1f5; border:1px solid #39414b;
                               border-radius:5px; padding:14px; font-size:14px; selection-background-color:#385e7b; }
        QTableWidget { background:#20252c; alternate-background-color:#292f38; border:1px solid #3c4653;
                       border-radius:4px; gridline-color:#414b59; selection-background-color:#38566e; selection-color:#eef7ff; }
        QHeaderView::section { background:#303844; color:#d8e1eb; border:0; border-right:1px solid #455365; padding:6px; }
        QTableCornerButton::section { background:#303844; border:0; }
        QListWidget#history { background:transparent; border:0; outline:0; padding:0; }
        QListWidget#history::item { background:#20252c; border:1px solid #414b57; border-radius:3px; padding:0; }
        QListWidget#history::item:hover { background:#2d3947; border-color:#7899b4; }
        QListWidget#history::item:selected { background:#263646; border:2px solid #8ed3ff; }
        QListWidget#history QWidget { background:transparent; }
        QScrollBar:vertical { background:#20252c; width:7px; margin:0; }
        QScrollBar:horizontal { background:#20252c; height:6px; margin:0; }
        QScrollBar::handle { background:#626f7f; border-radius:3px; min-height:24px; min-width:24px; }
        QScrollBar::add-line, QScrollBar::sub-line { width:0; height:0; }
        QScrollBar::add-page, QScrollBar::sub-page { background:none; }
        QToolTip { background:#dfeaf4; color:#1e2d3c; border:1px solid #9bb6ce; padding:5px; }
        QMenu { background:#29323d; color:#eef3fa; border:1px solid #5b718a; padding:5px; }
        QMenu::item { padding:7px 23px; }
        QMenu::item:selected { background:#3b5874; }
    )QSS"));
}
