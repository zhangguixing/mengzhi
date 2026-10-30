#include "ui_study.h"
#include "canvas.h"
#include "motion.h"
#include <QtWidgets>
#include <memory>

namespace {
class ToolPlate : public QObject {
 QWidget* tools;QFrame* plate;QPropertyAnimation* animation;
 void place(bool animated){
  QToolButton* selected=nullptr;for(auto button:tools->findChildren<QToolButton*>())if(button->isChecked()){selected=button;break;}
  if(!selected)return;const QRect target=selected->geometry();animation->stop();
  if(!animated||reducedMotion()||plate->geometry().isEmpty())plate->setGeometry(target);
  else {animation->setStartValue(plate->geometry());animation->setEndValue(target);animation->start();}
  plate->show();plate->lower();
 }
public:
 explicit ToolPlate(QWidget* target):QObject(target),tools(target){
  plate=new QFrame(target);plate->setObjectName("studioToolPlate");plate->setAttribute(Qt::WA_TransparentForMouseEvents);plate->setStyleSheet("QFrame{background:rgba(139,188,229,36);border:1px solid rgba(139,188,229,65);border-radius:5px;}");
  animation=new QPropertyAnimation(plate,"geometry",this);animation->setDuration(140);animation->setEasingCurve(QEasingCurve::OutCubic);target->installEventFilter(this);
  for(auto button:target->findChildren<QToolButton*>())if(button->isCheckable())connect(button,&QToolButton::toggled,this,[this](bool checked){if(checked)place(true);});
  QTimer::singleShot(0,this,[this]{place(false);});
 }
 bool eventFilter(QObject* object,QEvent* event)override{if(object==tools&&(event->type()==QEvent::Resize||event->type()==QEvent::LayoutRequest))QTimer::singleShot(0,this,[this]{place(false);});return QObject::eventFilter(object,event);}
};
}

void installStudioInteractions(QWidget* window){
 if(window->property("studioInteractions").toBool())return;window->setProperty("studioInteractions",true);
 auto canvas=window->findChild<Canvas*>();auto result=window->findChild<QWidget*>("readingPanel");auto dock=window->findChild<QFrame*>("graphiteFilmDock");auto split=window->findChild<QSplitter*>("canvasSplit");
 auto focus=window->findChild<QPushButton*>("focusCanvasButton");
 if(canvas&&focus&&result&&split){
  QObject::disconnect(focus,nullptr,nullptr,nullptr);focus->setCheckable(true);focus->setToolTip(QStringLiteral("专注原图  Tab（画布内）"));
  auto sizes=std::make_shared<QList<int>>();
  QObject::connect(focus,&QPushButton::toggled,window,[window,focus,result,dock,split,sizes](bool focused){
   if(focused){*sizes=split->sizes();result->hide();if(dock)dock->hide();focus->setText(QStringLiteral("退出专注"));}
   else {result->show();if(dock)dock->show();if(!sizes->isEmpty())split->setSizes(*sizes);focus->setText(QStringLiteral("专注模式"));reveal(result,160);if(dock)reveal(dock,130);}
   window->setProperty("canvasFocused",focused);
  });
  auto tab=new QShortcut(QKeySequence(Qt::Key_Tab),canvas);tab->setContext(Qt::WidgetWithChildrenShortcut);QObject::connect(tab,&QShortcut::activated,focus,&QPushButton::toggle);
  auto escape=new QShortcut(QKeySequence(Qt::Key_Escape),window);escape->setEnabled(false);QObject::connect(focus,&QPushButton::toggled,escape,&QShortcut::setEnabled);QObject::connect(escape,&QShortcut::activated,window,[focus]{focus->setChecked(false);});
 }
 if(canvas){
  if(auto fit=window->findChild<QPushButton*>("fitImageButton"))QObject::connect(fit,&QPushButton::clicked,canvas,&Canvas::fitToWindow);
  auto fitKey=new QShortcut(QKeySequence("Ctrl+0"),canvas);fitKey->setContext(Qt::WidgetWithChildrenShortcut);QObject::connect(fitKey,&QShortcut::activated,canvas,&Canvas::fitToWindow);
  if(auto history=window->findChild<QListWidget*>("history"))QObject::connect(history,&QListWidget::currentRowChanged,canvas,[canvas]{reveal(canvas,150);});
 }
 if(auto tools=window->findChild<QFrame*>("tools"))new ToolPlate(tools);
 if(auto recognize=window->findChild<QPushButton*>("recognizeButton")){
  recognize->setText(QStringLiteral("识别文字"));
  auto key=new QShortcut(QKeySequence("Ctrl+Return"),window);QObject::connect(key,&QShortcut::activated,recognize,&QPushButton::click);
 }
 if(auto exportFile=window->findChild<QPushButton*>("exportButton")){auto key=new QShortcut(QKeySequence::Save,window);QObject::connect(key,&QShortcut::activated,exportFile,&QPushButton::click);}
 if(auto copy=window->findChild<QPushButton*>("copyResultButton")){auto key=new QShortcut(QKeySequence("Ctrl+Shift+C"),window);QObject::connect(key,&QShortcut::activated,copy,&QPushButton::click);}
}

void showStudioAppearance(QWidget* window){
 QDialog dialog(window);dialog.setWindowTitle(QStringLiteral("工作台外观与交互"));dialog.setMinimumWidth(380);auto layout=new QVBoxLayout(&dialog);layout->setContentsMargins(24,22,24,22);layout->setSpacing(18);
 auto title=new QLabel(QStringLiteral("石墨光台"));title->setStyleSheet("font-size:19px;font-weight:600;");layout->addWidget(title);
 auto details=new QLabel(QStringLiteral("深色画布 · 冰蓝操作色 · 原图优先"));details->setWordWrap(true);layout->addWidget(details);
 auto motion=new QCheckBox(QStringLiteral("减少动态效果"));motion->setChecked(QSettings().value("reduceMotion",false).toBool());layout->addWidget(motion);QObject::connect(motion,&QCheckBox::toggled,&dialog,[](bool checked){QSettings().setValue("reduceMotion",checked);});
 auto transparency=new QCheckBox(QStringLiteral("减少透明效果"));transparency->setChecked(QSettings().value("reduceTransparency",false).toBool());layout->addWidget(transparency);QObject::connect(transparency,&QCheckBox::toggled,&dialog,[](bool checked){QSettings().setValue("reduceTransparency",checked);});
 auto hint=new QLabel(QStringLiteral("Ctrl + 滚轮：围绕鼠标缩放\n鼠标中键拖动：移动画布\nCtrl + 0：适应窗口\nTab：专注原图\nCtrl + Enter：识别文字"));hint->setWordWrap(true);layout->addWidget(hint);
 auto done=new QDialogButtonBox(QDialogButtonBox::Close);done->button(QDialogButtonBox::Close)->setText(QStringLiteral("完成"));QObject::connect(done,&QDialogButtonBox::rejected,&dialog,&QDialog::accept);layout->addWidget(done);dialog.exec();
}
