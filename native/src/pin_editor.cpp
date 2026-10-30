#include "overlay.h"
#include "canvas.h"
#include "capture_icons.h"
#include "capture_toolbar.h"
#include "capture_clipboard.h"
#include <QtWidgets>

QImage PinWindow::currentImage()const{return m_editing&&m_pinEditor?m_pinEditor->composed():m_image;}

void PinWindow::createEditToolbar(){
 m_pinEditor=new Canvas(this);m_pinEditor->setObjectName("pinEditor");m_pinEditor->setMinimumSize(0,0);m_pinEditor->setDirectMode(true);m_pinEditor->hide();m_pinEditor->installEventFilter(this);
 auto rail=new CaptureToolbar(m_pinEditor,this);m_editToolbar=rail;rail->setEditingPin(true);rail->setObjectName("pinEditToolbar");rail->setWindowFlags(Qt::Tool|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);rail->setAttribute(Qt::WA_TranslucentBackground);rail->setWindowTitle(QStringLiteral("贴图标注"));
 connect(rail,&CaptureToolbar::toolChanged,this,[this](int){placeEditToolbar();});
 connect(rail,&CaptureToolbar::actionTriggered,this,[this](int action){if(action==0){m_pinEditor->finishStroke();copyCaptureImage(currentImage(),imageGeometry());}else if(action==4)saveImage();else if(action==5)quickSaveImage();else if(action==6)finishEditing(false);else if(action==7)finishEditing();else if(action==1){finishEditing();emit editRequested(m_image);}else if(action==2){finishEditing();emit recognizeRequested(m_image);}});
 rail->hide();
}
void PinWindow::startEditing(){
 if(m_editing)return;if(m_clickThrough)setClickThrough(false);
 if(!m_pinEditor)createEditToolbar();
 if(!m_hasEditCapture){m_editCapture={};m_editCapture.image=m_image;m_editCapture.image.setDevicePixelRatio(1);m_hasEditCapture=true;}
 m_beforeEdit=m_editCapture.marks;m_editing=true;m_controls->hide();m_pinEditor->setGeometry(imageRect());m_pinEditor->setCapture(&m_editCapture);
 if(auto rail=qobject_cast<CaptureToolbar*>(m_editToolbar))rail->selectTool(1);
 m_pinEditor->show();m_pinEditor->raise();m_editToolbar->show();placeEditToolbar();activateWindow();m_pinEditor->setFocus();
}

void PinWindow::finishEditing(bool apply){
 if(!m_editing)return;
 m_pinEditor->finishStroke(apply);
 if(apply)m_image=m_pinEditor->composed();else m_editCapture.marks=m_beforeEdit;
 m_editing=false;m_pinEditor->hide();m_editToolbar->hide();setFocus();update();
}

void PinWindow::placeEditToolbar(){
 if(!m_editing||!m_editToolbar)return;m_editToolbar->adjustSize();const auto content=imageGeometry();auto screen=QGuiApplication::screenAt(content.center());if(!screen)screen=QGuiApplication::primaryScreen();if(!screen)return;const auto bounds=screen->availableGeometry();
 int x=qBound(bounds.left(),content.right()-m_editToolbar->width()+1,qMax(bounds.left(),bounds.right()-m_editToolbar->width()+1));
 int y=content.bottom()+8;if(y+m_editToolbar->height()>bounds.bottom())y=content.top()-m_editToolbar->height()-8;
 m_editToolbar->move(x,qMax(bounds.top(),y));m_editToolbar->raise();
}

bool PinWindow::eventFilter(QObject* watched,QEvent* event){
 if(watched==m_pinEditor&&m_editing){
  if(event->type()==QEvent::KeyPress){auto key=static_cast<QKeyEvent*>(event);if(key->key()==Qt::Key_Escape||key->key()==Qt::Key_Space||key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter||key->modifiers()&Qt::ControlModifier){keyPressEvent(key);return key->isAccepted();}}
  if(event->type()==QEvent::ContextMenu){contextMenuEvent(static_cast<QContextMenuEvent*>(event));return true;}
  if(event->type()==QEvent::MouseButtonDblClick)return true;
 }
 return QWidget::eventFilter(watched,event);
}

void PinWindow::saveImage(){auto path=QFileDialog::getSaveFileName(this,QStringLiteral("保存贴图"),QStringLiteral("贴图.png"),"PNG (*.png)");if(path.isEmpty())return;if(!path.endsWith(".png",Qt::CaseInsensitive))path+=".png";if(!currentImage().save(path,"PNG"))QMessageBox::warning(this,QStringLiteral("保存失败"),QStringLiteral("无法保存贴图，请检查文件位置。"));}
void PinWindow::moveEvent(QMoveEvent* event){QWidget::moveEvent(event);placeEditToolbar();}
void PinWindow::resizeEvent(QResizeEvent* event){QWidget::resizeEvent(event);if(m_pinEditor)m_pinEditor->setGeometry(imageRect());placeEditToolbar();}
void PinWindow::hideEvent(QHideEvent* event){finishEditing();if(m_editToolbar)m_editToolbar->hide();QWidget::hideEvent(event);}

void PinWindow::quickSaveImage(){QString directory=QSettings().value("captureSaveDirectory",QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)+"/Mengzhi").toString();QDir().mkpath(directory);QString path=directory+"/Mengzhi_"+QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz")+".png";QSaveFile file(path);if(!file.open(QIODevice::WriteOnly)||!currentImage().save(&file,"PNG")||!file.commit())QMessageBox::warning(this,QStringLiteral("保存失败"),QStringLiteral("无法保存贴图，请检查文件位置。"));}
