#include "capture_toolbar.h"
#include "canvas.h"
#include <QtWidgets>
#include <cmath>

namespace {
enum Icon {
    ShapeGlyph = 100, Undo, Redo, Cancel, Pin, Save, Copy,
    Width, Fill, Rectangle, Ellipse, Swatch, PaletteColor
};
const QColor Ink("#4c504f");
const QColor Blue("#008fe7");

// These paths deliberately use their own 20-unit, one-pixel optical grid.
// Workbench icon weights and rounded corners do not bleed into the capture rail.
void paintGlyph(QPainter& p, int icon, QColor ink, QColor background)
{
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(ink, 1.1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    p.setBrush(Qt::NoBrush);
    switch (icon) {
    case ShapeGlyph:
        p.drawRect(QRectF(2.5, 4.5, 11, 9));
        p.setBrush(background);
        p.drawEllipse(QRectF(9.5, 10.5, 8, 5));
        break;
    case 7: // Fine, angular polyline, rather than a rounded generic stroke.
        p.drawPolyline(QPolygonF{{2.5, 15.5}, {6.5, 9.5}, {10.5, 12.5}, {17, 4}});
        break;
    case 3:
        p.drawLine(QPointF(3, 17), QPointF(17, 3));
        p.drawPolyline(QPolygonF{{11, 4}, {17, 3}, {16, 9}});
        break;
    case 4: // The reference's pointed fountain-pen nib.
        p.drawPolygon(QPolygonF{{3, 17}, {6.5, 9}, {13.5, 3}, {17, 6.5}, {11, 13.5}});
        p.drawLine(QPointF(3, 17), QPointF(10.5, 9.5));
        p.drawLine(QPointF(10.5, 5.5), QPointF(14.5, 9.5));
        p.setBrush(ink); p.drawEllipse(QPointF(10.5, 9.5), 1, 1);
        break;
    case 6:
        p.setRenderHint(QPainter::Antialiasing, false);
        p.drawRect(QRectF(3.5, 3.5, 13, 13));
        p.fillRect(QRectF(4, 4, 6, 6), ink);
        p.fillRect(QRectF(10, 10, 6, 6), ink);
        break;
    case 5:
        p.setPen(QPen(ink, 1.5, Qt::SolidLine, Qt::SquareCap));
        p.drawLine(QPointF(3.5, 4), QPointF(16.5, 4));
        p.drawLine(QPointF(10, 4), QPointF(10, 17));
        break;
    case 10:
        p.drawPolygon(QPolygonF{{4, 12}, {11.5, 4.5}, {17, 10}, {9.5, 17.5}});
        p.drawLine(QPointF(7, 9), QPointF(12.5, 14.5));
        p.setBrush(ink); p.drawPolygon(QPolygonF{{4, 12}, {7, 9}, {12.5, 14.5}, {9.5, 17.5}});
        break;
    case Undo:
    case Redo: {
        if (icon == Redo) { p.translate(20, 0); p.scale(-1, 1); }
        QPainterPath path;
        path.moveTo(3, 8); path.lineTo(9, 2.5); path.lineTo(9, 6);
        path.cubicTo(17, 5, 18.5, 11.5, 15, 17);
        path.cubicTo(16, 10.5, 12.5, 9, 9, 10);
        path.lineTo(9, 13); path.closeSubpath();
        p.setPen(Qt::NoPen); p.setBrush(ink); p.drawPath(path);
        break;
    }
    case Cancel:
        p.drawLine(QPointF(5.5, 5.5), QPointF(14.5, 14.5));
        p.drawLine(QPointF(14.5, 5.5), QPointF(5.5, 14.5));
        break;
    case Pin:
        p.translate(10, 9); p.rotate(42);
        p.setBrush(ink); p.setPen(Qt::NoPen);
        p.drawPolygon(QPolygonF{{-4,-6},{4,-6},{2,-3},{2,1},{5,3},{-5,3},{-2,1},{-2,-3}});
        p.setPen(QPen(ink, 1.3)); p.drawLine(QPointF(0, 3), QPointF(0, 10));
        break;
    case Save:
        p.setPen(Qt::NoPen); p.setBrush(ink);
        p.drawPolygon(QPolygonF{{3,3},{14,3},{17,6},{17,17},{3,17}});
        p.fillRect(QRectF(6, 3, 7, 5), background);
        p.fillRect(QRectF(6, 11, 8, 6), background);
        p.fillRect(QRectF(10.5, 3.5, 1, 3.5), ink);
        break;
    case Copy:
        p.drawPolyline(QPolygonF{{13.5,4.5},{13.5,2.5},{3.5,2.5},{3.5,14.5},{5.5,14.5}});
        p.drawRect(QRectF(6.5, 5.5, 10, 12));
        break;
    case Rectangle: p.drawRect(QRectF(2.5, 5.5, 15, 10)); break;
    case Ellipse: p.drawEllipse(QRectF(2.5, 5.5, 15, 10)); break;
    default: break;
    }
}

class RailButton final : public QAbstractButton {
public:
    int icon;
    QColor swatch;
    qreal stroke = 3;
    bool filled = false;
    explicit RailButton(int glyph, const QString& tip, QWidget* parent = nullptr)
        : QAbstractButton(parent), icon(glyph)
    {
        setFixedSize(32, 30);
        setToolTip(tip); setAccessibleName(tip);
        setCursor(Qt::PointingHandCursor); setFocusPolicy(Qt::NoFocus);
        setAttribute(Qt::WA_Hover);
    }
protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        const bool selected = isChecked();
        QColor bg = selected ? Blue : (isDown() ? QColor("#d8dce0") : underMouse() ? QColor("#e1e7ec") : QColor("#f2f2f2"));
        p.fillRect(rect(), bg);
        QColor ink = selected ? QColor(Qt::white) : Ink;
        if (!isEnabled()) ink.setAlpha(95);
        if (icon == PaletteColor || icon == Swatch) {
            const QRect box = rect().adjusted(icon == Swatch ? 2 : 1, icon == Swatch ? 3 : 1,
                                              icon == Swatch ? -2 : -1, icon == Swatch ? -3 : -1);
            for (int y = box.top(); y <= box.bottom(); y += 4)
                for (int x = box.left(); x <= box.right(); x += 4)
                    p.fillRect(QRect(x, y, 4, 4).intersected(box), ((x-box.left())/4+(y-box.top())/4)%2 ? QColor("#bdbdbd") : QColor("#fafafa"));
            p.fillRect(box, swatch);
            p.setPen(QPen(icon == PaletteColor ? QColor("#999999") : QColor("#555555"), 1));
            p.drawRect(box.adjusted(0, 0, -1, -1));
            if (icon == Swatch) {
                // The multicolor lower corner cues the full color/alpha dialog.
                const QPointF br(box.right()-1, box.bottom()-1);
                p.setPen(Qt::NoPen); p.setBrush(QColor("#3bd86e"));
                p.drawPolygon(QPolygonF{br,br-QPointF(7,0),br-QPointF(0,7)});
                p.setBrush(QColor("#2a86df"));
                p.drawPolygon(QPolygonF{br-QPointF(1,1),br-QPointF(5,1),br-QPointF(1,5)});
            }
            return;
        }
        p.translate((width()-20)/2.0, (height()-20)/2.0);
        if (icon == Width) {
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen); p.setBrush(ink);
            qreal radius = qBound(1.0, stroke/2, 4.0);
            p.drawEllipse(QPointF(9, 10), radius, radius);
        } else if (icon == Fill) {
            p.setRenderHint(QPainter::Antialiasing, false);
            if (filled) p.fillRect(QRect(3,4,13,12), ink);
            else {
                for(int y=4;y<16;++y) for(int x=3;x<16;++x)
                    if((x+y)%2==0) p.fillRect(QRect(x,y,1,1), ink);
            }
        } else paintGlyph(p, icon, ink, bg);
        if (icon == Width || icon == Fill) {
            p.setPen(Qt::NoPen); p.setBrush(ink);
            p.drawPolygon(QPolygonF{{16,16},{20,16},{20,12}});
        }
    }
};

RailButton* railButton(int icon, const QString& tip, QHBoxLayout* layout)
{
    auto button = new RailButton(icon, tip);
    layout->addWidget(button);
    return button;
}
void separator(QHBoxLayout* layout)
{
    auto holder = new QWidget;
    holder->setFixedSize(5, 30);
    auto line = new QFrame(holder);
    line->setGeometry(2, 7, 1, 16);
    line->setStyleSheet("background:#c1c4c6;border:0;border-radius:0;");
    layout->addWidget(holder);
}
QIcon widthIcon(int width)
{
    QPixmap image(40,40); image.setDevicePixelRatio(2); image.fill(Qt::transparent);
    QPainter p(&image); p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen); p.setBrush(Ink); p.drawEllipse(QPointF(10,10), width/2.0, width/2.0);
    return QIcon(image);
}
}

CaptureToolbar::CaptureToolbar(Canvas* editor, QWidget* parent)
    : QFrame(parent), m_editor(editor)
{
    if (editor) editor->installEventFilter(this);
    setObjectName("captureToolbar");
    setAttribute(Qt::WA_StyledBackground, false);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setStyleSheet(QStringLiteral(
        "QFrame#captureToolbar{background:transparent;border:0;border-radius:0;}"
        "QFrame#captureMainRail,QFrame#capturePropertyRail{background:#f2f2f2;border:1px solid #b8bdc2;border-radius:0;}"
        "QMenu{background:#f5f5f5;color:#353a3e;border:1px solid #b8bdc2;border-radius:0;padding:3px;font-size:12px;}"
        "QMenu::item{padding:5px 15px;}QMenu::item:selected{background:#dceeff;color:#20272d;}"
        "QToolTip{background:#343434;color:#fafafa;border:0;padding:4px 6px;font-size:12px;}"));
    auto outer = new QVBoxLayout(this);
    outer->setContentsMargins(0,0,0,0); outer->setSpacing(2);
    outer->setSizeConstraint(QLayout::SetFixedSize);
    auto main = new QFrame;
    main->setObjectName("captureMainRail");
    auto row = new QHBoxLayout(main);
    row->setContentsMargins(1,1,1,1); row->setSpacing(0);
    m_shape = railButton(ShapeGlyph, QStringLiteral("形状：矩形 / 椭圆  R / O"), row);
    m_shape->setCheckable(true);
    connect(m_shape, &QAbstractButton::clicked, this, [this]{ selectTool(m_shapeTool); });
    const QVector<int> tools = {7,3,4,6,5,10};
    const QStringList tips = {QStringLiteral("直线 / 折线  L · 右键完成折线"), QStringLiteral("箭头  A"),
        QStringLiteral("画笔  P · 右键选择马克笔"), QStringLiteral("马赛克  M · 右键选择高斯模糊"),
        QStringLiteral("文字  T"), QStringLiteral("橡皮擦  E")};
    for (int i=0;i<tools.size();++i) {
        auto button = railButton(tools[i], tips[i], row);
        button->setCheckable(true); m_tools.insert(tools[i], button);
        connect(button, &QAbstractButton::clicked, this, [this,id=tools[i]]{selectTool(id);});
        if (tools[i] == 4 || tools[i] == 6) {
            button->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(button, &QWidget::customContextMenuRequested, this,
                    [this,button](const QPoint& at){showMoreMenu(button->mapToGlobal(at));});
        }
    }
    separator(row);
    auto undo = railButton(Undo, QStringLiteral("撤销  Ctrl + Z"), row);
    auto redo = railButton(Redo, QStringLiteral("重做  Ctrl + Y"), row);
    undo->setEnabled(false); redo->setEnabled(false);
    if (editor) {
        connect(undo, &QAbstractButton::clicked, editor, &Canvas::undo);
        connect(redo, &QAbstractButton::clicked, editor, &Canvas::redo);
        connect(editor, &Canvas::historyChanged, this, [undo,redo](bool u,bool r){undo->setEnabled(u);redo->setEnabled(r);});
    }
    separator(row);
    auto cancel = railButton(Cancel, QStringLiteral("取消  Esc"), row);
    connect(cancel, &QAbstractButton::clicked, this, [this]{emit actionTriggered(6);});
    m_pin = railButton(Pin, QStringLiteral("贴到屏幕  Ctrl + T"), row);
    connect(m_pin, &QAbstractButton::clicked, this, [this]{emit actionTriggered(m_editingPin ? 7 : 3);});
    auto save = railButton(Save, QStringLiteral("保存  Ctrl + S · 右键更多"), row);
    connect(save, &QAbstractButton::clicked, this, [this]{emit actionTriggered(4);});
    auto copy = railButton(Copy, QStringLiteral("复制  Enter / Ctrl + C · 右键更多"), row);
    connect(copy, &QAbstractButton::clicked, this, [this]{emit actionTriggered(0);});
    outer->addWidget(main, 0, Qt::AlignLeft);

    auto properties = new QFrame;
    m_properties = properties;
    properties->setObjectName("capturePropertyRail");
    properties->setFixedSize(304,34);
    auto lower = new QHBoxLayout(properties);
    lower->setContentsMargins(1,1,1,1); lower->setSpacing(0);
    m_width = railButton(Width, QStringLiteral("线宽"), lower);
    m_width->setFixedSize(32,30);
    m_width->setCheckable(true); m_width->setChecked(true);
    connect(m_width, &QAbstractButton::clicked, this, [this] {
        QMenu menu(this);
        for(int width : {1,2,3,5,8}) {
            auto action = menu.addAction(widthIcon(width), QStringLiteral("%1 像素").arg(width));
            action->setCheckable(true); action->setChecked(m_editor && qFuzzyCompare(m_editor->lineWidth,qreal(width)));
            connect(action,&QAction::triggered,this,[this,width]{if(m_editor)m_editor->lineWidth=width;refreshProperties();});
        }
        menu.exec(m_width->mapToGlobal(QPoint(0,m_width->height())));
        m_width->setChecked(true);
    });
    m_fill = railButton(Fill, QStringLiteral("填充方式"), lower);
    m_fill->setFixedSize(36,30);
    connect(m_fill,&QAbstractButton::clicked,this,[this] {
        QMenu menu(this);
        for (bool fill : {false,true}) {
            auto action=menu.addAction(fill ? QStringLiteral("实心填充") : QStringLiteral("仅边框"));
            action->setCheckable(true); action->setChecked(m_editor && m_editor->filled==fill);
            connect(action,&QAction::triggered,this,[this,fill]{if(m_editor)m_editor->filled=fill;refreshProperties();});
        }
        menu.exec(m_fill->mapToGlobal(QPoint(0,m_fill->height())));
    });
    m_rectangle = railButton(Rectangle, QStringLiteral("矩形  R"), lower);
    m_ellipse = railButton(Ellipse, QStringLiteral("椭圆  O"), lower);
    m_rectangle->setCheckable(true); m_ellipse->setCheckable(true);
    connect(m_rectangle,&QAbstractButton::clicked,this,[this]{selectTool(1);});
    connect(m_ellipse,&QAbstractButton::clicked,this,[this]{selectTool(2);});
    lower->addSpacing(3);
    m_swatch = railButton(Swatch, QStringLiteral("颜色与透明度…"), lower);
    m_swatch->setFixedSize(27,30);
    connect(m_swatch,&QAbstractButton::clicked,this,[this] {
        if(!m_editor)return;
        auto color=QColorDialog::getColor(m_editor->color,this,QStringLiteral("颜色与透明度"),QColorDialog::ShowAlphaChannel);
        if(color.isValid()){m_editor->color=color;refreshProperties();}
    });
    auto palette = new QWidget;
    palette->setFixedSize(136,26);
    auto grid = new QGridLayout(palette);
    grid->setContentsMargins(0,0,0,0); grid->setSpacing(1);
    // Ten columns, two rows: neutral values and the familiar saturated spectrum.
    const QStringList colors = {"#000000","#808080","#800000","#ff0000","#ff8000","#ffff00","#008000","#00b6b6","#0000ff","#800080",
                                "#ffffff","#c0c0c0","#b87a57","#ff8080","#ffd080","#ffff80","#80ff80","#80ffff","#8080ff","#d080d0"};
    for(int i=0;i<colors.size();++i) {
        auto button=new RailButton(PaletteColor,QStringLiteral("颜色 ")+colors[i],palette);
        button->setFixedSize(12,12); button->swatch=QColor(colors[i]);
        grid->addWidget(button,i/10,i%10);
        connect(button,&QAbstractButton::clicked,this,[this,color=QColor(colors[i])] {
            if(!m_editor)return;
            const int alpha=m_editor->color.alpha();m_editor->color=color;m_editor->color.setAlpha(alpha);refreshProperties();
        });
    }
    lower->addWidget(palette,0,Qt::AlignVCenter);
    outer->addWidget(properties,0,Qt::AlignLeft);
    properties->hide();
    for(auto widget : {static_cast<QWidget*>(main),static_cast<QWidget*>(save),static_cast<QWidget*>(copy)}) {
        widget->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(widget,&QWidget::customContextMenuRequested,this,[this,widget](const QPoint& at){showMoreMenu(widget->mapToGlobal(at));});
    }
    refreshProperties();
}

CaptureToolbar::~CaptureToolbar() = default;

bool CaptureToolbar::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_editor && event->type() == QEvent::KeyPress) {
        auto key = static_cast<QKeyEvent*>(event);
        if (key->modifiers() == Qt::NoModifier) {
            static const QHash<int,int> keys = {{Qt::Key_R,1},{Qt::Key_O,2},{Qt::Key_L,7},
                {Qt::Key_A,3},{Qt::Key_P,4},{Qt::Key_M,6},{Qt::Key_T,5},
                {Qt::Key_E,10},{Qt::Key_H,8},{Qt::Key_B,9}};
            if (keys.contains(key->key())) { selectTool(keys.value(key->key())); key->accept(); return true; }
        }
    }
    return QFrame::eventFilter(watched,event);
}

QSize CaptureToolbar::sizeHint() const
{
    return QSize(428, m_properties && !m_properties->isHidden() ? 68 : 32);
}

void CaptureToolbar::setEditingPin(bool editing)
{
    m_editingPin=editing;
    m_pin->setToolTip(editing ? QStringLiteral("完成编辑 · 复制按钮仍只复制图片") : QStringLiteral("贴到屏幕  Ctrl + T"));
    m_pin->setAccessibleName(editing ? QStringLiteral("完成编辑") : QStringLiteral("贴到屏幕"));
}

void CaptureToolbar::selectTool(int tool)
{
    if(!m_editor)return;
    if(tool==1 || tool==2)m_shapeTool=tool;
    m_editor->finishStroke();
    m_editor->setTool(tool);
    m_shape->setChecked(tool==1 || tool==2);
    for(auto it=m_tools.cbegin();it!=m_tools.cend();++it)
        it.value()->setChecked(tool==it.key() || (tool==8 && it.key()==4) || (tool==9 && it.key()==6));
    m_rectangle->setChecked(m_shapeTool==1);
    m_ellipse->setChecked(m_shapeTool==2);
    m_properties->setVisible(tool!=0);
    refreshProperties();
    layout()->activate();
    adjustSize();
    if(tool!=0)m_editor->setFocus(Qt::OtherFocusReason);
    emit toolChanged(tool);
}

void CaptureToolbar::refreshProperties()
{
    if(!m_editor)return;
    static_cast<RailButton*>(m_width)->stroke=m_editor->lineWidth;
    static_cast<RailButton*>(m_fill)->filled=m_editor->filled;
    static_cast<RailButton*>(m_swatch)->swatch=m_editor->color;
    m_width->update();m_fill->update();m_swatch->update();
    m_rectangle->setChecked(m_shapeTool==1);
    m_ellipse->setChecked(m_shapeTool==2);
}

void CaptureToolbar::showMoreMenu(const QPoint& position)
{
    QMenu menu(this);
    menu.addAction(QStringLiteral("马克笔  H"),this,[this]{selectTool(8);});
    menu.addAction(QStringLiteral("高斯模糊  B"),this,[this]{selectTool(9);});
    menu.addSeparator();
    menu.addAction(QStringLiteral("识别文字"),this,[this]{emit actionTriggered(2);});
    menu.addAction(QStringLiteral("打开工作台"),this,[this]{emit actionTriggered(1);});
    menu.addAction(QStringLiteral("快速保存  Ctrl + Shift + S"),this,[this]{emit actionTriggered(5);});
    if(m_editingPin) {
        menu.addSeparator();
        menu.addAction(QStringLiteral("完成编辑"),this,[this]{emit actionTriggered(7);});
    }
    menu.exec(position);
}
