#include "window.h"
#include <QtWidgets>

void Window::customCapture(){
 if(capturing)return;
 QDialog dialog(this);dialog.setWindowTitle(QStringLiteral("自定义截图"));auto layout=new QVBoxLayout(&dialog);
 auto form=new QFormLayout;auto delay=new QSpinBox;delay->setRange(0,60);delay->setSuffix(QStringLiteral(" 秒"));delay->setValue(QSettings().value("captureDelay",3).toInt());form->addRow(QStringLiteral("延迟截图"),delay);layout->addLayout(form);
 auto fixed=new QGroupBox(QStringLiteral("指定选区位置与尺寸"));fixed->setCheckable(true);fixed->setChecked(false);auto regionForm=new QFormLayout(fixed);
 auto screen=QGuiApplication::screenAt(QCursor::pos());if(!screen)screen=QGuiApplication::primaryScreen();const auto bounds=screen->geometry();
 auto x=new QSpinBox;auto y=new QSpinBox;auto width=new QSpinBox;auto height=new QSpinBox;x->setRange(-32768,32768);y->setRange(-32768,32768);width->setRange(8,32768);height->setRange(8,32768);
 auto last=QSettings().value("lastCaptureRect",QRect(bounds.topLeft(),QSize(640,480))).toRect();x->setValue(last.x());y->setValue(last.y());width->setValue(last.width());height->setValue(last.height());
 regionForm->addRow(QStringLiteral("左侧位置"),x);regionForm->addRow(QStringLiteral("顶部位置"),y);regionForm->addRow(QStringLiteral("宽度"),width);regionForm->addRow(QStringLiteral("高度"),height);layout->addWidget(fixed);
 auto automatic=new QCheckBox(QStringLiteral("选定后自动复制"));layout->addWidget(automatic);
 auto buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("开始截图"));buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));layout->addWidget(buttons);
 connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]{if(fixed->isChecked()){const QRect region(x->value(),y->value(),width->value(),height->value());auto screen=QGuiApplication::screenAt(region.center());if(!screen||!screen->geometry().contains(region)){QMessageBox::information(&dialog,QStringLiteral("请调整选区"),QStringLiteral("指定区域需要位于同一块显示器内，请检查位置与尺寸。"));return;}}dialog.accept();});
 connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
 if(dialog.exec()!=QDialog::Accepted)return;
 const bool autoCopy=automatic->isChecked();const QRect region=fixed->isChecked()?QRect(x->value(),y->value(),width->value(),height->value()):QRect();
 QSettings().setValue("captureDelay",delay->value());
 if(delay->value()==0){startCaptureMode(autoCopy,region);return;}
 const bool visible=isVisible();hide();capturing=true;
 auto progress=new QProgressDialog(QStringLiteral("即将开始截图…"),QStringLiteral("取消"),0,delay->value());progress->setWindowTitle(QStringLiteral("延迟截图"));progress->setWindowFlags(Qt::Tool|Qt::WindowStaysOnTopHint);progress->setAttribute(Qt::WA_DeleteOnClose);progress->setAutoClose(false);progress->setMinimumDuration(0);progress->setValue(0);progress->show();
 auto timer=new QTimer(progress);timer->setInterval(1000);
 connect(progress,&QProgressDialog::canceled,this,[this,timer,progress,visible]{timer->stop();capturing=false;progress->deleteLater();if(visible)toggleVisible();});
 connect(timer,&QTimer::timeout,this,[this,progress,timer,autoCopy,region]{progress->setValue(progress->value()+1);if(progress->value()>=progress->maximum()){timer->stop();progress->hide();progress->deleteLater();capturing=false;startCaptureMode(autoCopy,region);}});timer->start();
}
