#include "glassbutton.h"
#include "overlay.h"
#include "capture_clipboard.h"
#include <cmath>
#include "capture_icons.h"
#include "capture_toolbar.h"
#include "motion.h"
#include "canvas.h"
#include "theme.h"
#include <QtWidgets>

Overlay::Overlay(QScreen* screen,QImage image):m_image(image){
 setObjectName("captureOverlay");setWindowTitle(QStringLiteral("萌知截图"));
 setAttribute(Qt::WA_DeleteOnClose);setWindowFlags(Qt::Tool|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
 setGeometry(screen->geometry());setCursor(Qt::CrossCursor);setMouseTracking(true);
 m_targets=captureTargets(screen);m_cursor=mapFromGlobal(QCursor::pos());
 m_editor=new Canvas(this);m_editor->setDirectMode(true);m_editor->setMinimumSize(0,0);
 m_editor->hide();m_editor->setAttribute(Qt::WA_TransparentForMouseEvents);m_editor->installEventFilter(this);
 auto rail=new CaptureToolbar(m_editor,this);m_toolbar=rail;rail->setObjectName("captureToolbar");
 connect(rail,&CaptureToolbar::actionTriggered,this,[this](int action){if(action==6)close();else commit(action);});
 connect(rail,&CaptureToolbar::toolChanged,this,[this](int tool){m_editor->setAttribute(Qt::WA_TransparentForMouseEvents,tool==0);if(tool==0)setFocus();if(m_locked)placeToolbar();});
 rail->selectTool(0);rail->hide();
 detectAtCursor();show();raise();activateWindow();setFocus();
}
void Overlay::detectAtCursor(){
 if(m_locked||m_dragging)return;
 m_selection=captureTargetAt(m_targets,m_cursor,m_elements);update();
}
void Overlay::clearSelection(){
 m_locked=false;m_dragging=false;m_selection={};selectTool(0);m_editor->hide();m_toolbar->hide();detectAtCursor();setFocus();update();
}
void Overlay::setInitialSelection(const QRect& selection){
 m_selection=selection.intersected(rect());if(m_selection.width()<8||m_selection.height()<8)return;
 m_locked=true;prepareEditor();placeToolbar();update();
}
void Overlay::selectTool(int tool){if(auto rail=qobject_cast<CaptureToolbar*>(m_toolbar))rail->selectTool(tool);}
QRect Overlay::sourceRect()const{
 const int left=qRound(m_selection.left()*m_image.width()/double(width())),top=qRound(m_selection.top()*m_image.height()/double(height()));
 const int right=qRound((m_selection.left()+m_selection.width())*m_image.width()/double(width())),bottom=qRound((m_selection.top()+m_selection.height())*m_image.height()/double(height()));
 return QRect(left,top,right-left,bottom-top).intersected(m_image.rect());
}
void Overlay::prepareEditor(){
 m_capture={};m_capture.image=m_image.copy(sourceRect());m_capture.image.setDevicePixelRatio(1);
 m_editor->setGeometry(m_selection);m_editor->setCapture(&m_capture);selectTool(0);m_editor->show();m_toolbar->raise();
}
void Overlay::placeToolbar(){
 m_toolbar->adjustSize();int x=qBound(0,m_selection.right()-m_toolbar->width(),qMax(0,width()-m_toolbar->width()));
 int y=m_selection.bottom()+8;if(y+m_toolbar->height()>height())y=m_selection.top()-m_toolbar->height()-8;
 if(y<0)y=qMax(0,height()-m_toolbar->height()-8);m_toolbar->move(x,y);m_toolbar->show();m_toolbar->raise();
}
void Overlay::paintEvent(QPaintEvent*){
 QPainter p(this);p.drawImage(rect(),m_image);p.fillRect(rect(),QColor(0,0,0,100));
 if(m_selection.isValid()){
  p.drawImage(m_selection,m_image,sourceRect());p.setPen(QPen(QColor("#169de5"),1));p.drawRect(m_selection.adjusted(0,0,-1,-1));
  if(m_locked){p.setBrush(Qt::white);for(auto point:{m_selection.topLeft(),m_selection.topRight(),m_selection.bottomLeft(),m_selection.bottomRight(),QPoint(m_selection.center().x(),m_selection.top()),QPoint(m_selection.center().x(),m_selection.bottom()),QPoint(m_selection.left(),m_selection.center().y()),QPoint(m_selection.right(),m_selection.center().y())})p.drawRect(QRect(point-QPoint(2,2),QSize(5,5)));}
  const auto source=sourceRect();const QString size=QString("%1 × %2").arg(source.width()).arg(source.height());
  QRect label(m_selection.left(),qMax(2,m_selection.top()-25),140,22);p.fillRect(label,QColor(35,35,35,225));p.setPen(Qt::white);p.drawText(label.adjusted(6,0,0,0),Qt::AlignVCenter,size);
 }
 if(!m_locked){QRect help(24,24,qMin(660,width()-48),29);p.fillRect(help,QColor(35,35,35,220));p.setPen(Qt::white);p.drawText(help.adjusted(8,0,-8,0),Qt::AlignVCenter,m_targets.isEmpty()?QStringLiteral("拖动框选 · C 取色 · R 上次选区 · Esc 退出"):QStringLiteral("单击选中窗口 · 拖动框选 · Tab 切换窗口/控件 · R 上次选区 · Esc 退出"));}
 if(m_dragging||m_alt||!m_locked){
  int x=qBound(0,qRound(m_cursor.x()*m_image.width()/double(width())),m_image.width()-1),y=qBound(0,qRound(m_cursor.y()*m_image.height()/double(height())),m_image.height()-1);
  QRect lens(qBound(4,m_cursor.x()+22,qMax(4,width()-134)),qBound(4,m_cursor.y()+22,qMax(4,height()-154)),126,148);
  p.fillRect(lens,QColor("#fafafa"));auto patch=m_image.copy(QRect(x-7,y-7,15,15).intersected(m_image.rect()));p.setRenderHint(QPainter::SmoothPixmapTransform,false);p.drawImage(lens.adjusted(3,3,-3,-25),patch);
  p.setPen(QPen(QColor("#169de5"),1));p.drawRect(lens.left()+58,lens.top()+58,9,9);auto color=m_image.pixelColor(x,y);
  const QString value=m_rgb?QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()):color.name().toUpper();p.setPen(QColor("#333333"));p.drawText(QRect(lens.left(),lens.bottom()-24,lens.width(),24),Qt::AlignCenter,value);
 }
}
void Overlay::mousePressEvent(QMouseEvent* event){
 m_cursor=event->position().toPoint();
 if(event->button()==Qt::MiddleButton&&m_selection.isValid()){commit(3);return;}
 if(event->button()==Qt::RightButton){if(m_editor->tool()!=0)selectTool(0);else if(m_locked)clearSelection();else close();return;}
 if(event->button()!=Qt::LeftButton)return;
 m_pressedLocked=m_locked;m_manualDrag=false;m_toolbar->hide();m_editor->hide();m_start=m_cursor;m_last=m_cursor;m_initial=m_selection;m_resize=-1;m_moving=false;
 if(m_locked&&m_selection.isValid()){
  const auto r=m_selection;const QVector<QPoint> handles={r.topLeft(),r.topRight(),r.bottomLeft(),r.bottomRight(),QPoint(r.center().x(),r.top()),QPoint(r.center().x(),r.bottom()),QPoint(r.left(),r.center().y()),QPoint(r.right(),r.center().y())};
  for(int i=0;i<handles.size();++i)if(QLineF(handles[i],m_cursor).length()<8)m_resize=i;
  if(m_resize<0&&m_selection.contains(m_cursor))m_moving=true;
 }
 m_dragging=true;update();
}
void Overlay::mouseMoveEvent(QMouseEvent* event){
 m_cursor=event->position().toPoint();
 if(!m_dragging){if(!m_locked)detectAtCursor();else if(m_editor->tool()==0){
  const auto r=m_selection;const QVector<QPoint> handles={r.topLeft(),r.topRight(),r.bottomLeft(),r.bottomRight(),QPoint(r.center().x(),r.top()),QPoint(r.center().x(),r.bottom()),QPoint(r.left(),r.center().y()),QPoint(r.right(),r.center().y())};int handle=-1;for(int i=0;i<handles.size();++i)if(QLineF(handles[i],m_cursor).length()<8)handle=i;
  setCursor(handle==0||handle==3?Qt::SizeFDiagCursor:handle==1||handle==2?Qt::SizeBDiagCursor:handle==4||handle==5?Qt::SizeVerCursor:handle==6||handle==7?Qt::SizeHorCursor:m_selection.contains(m_cursor)?Qt::SizeAllCursor:Qt::CrossCursor);
 }update();return;}
 if((m_cursor-m_start).manhattanLength()>=3)m_manualDrag=true;
 if(!m_manualDrag)return;
 if(m_moving){m_selection=m_initial.translated(m_cursor-m_start);m_selection.moveLeft(qBound(0,m_selection.left(),qMax(0,width()-m_selection.width())));m_selection.moveTop(qBound(0,m_selection.top(),qMax(0,height()-m_selection.height())));}
 else if(m_resize>=0){auto r=m_initial;const auto delta=m_cursor-m_start;if(m_resize==0||m_resize==2||m_resize==6)r.setLeft(r.left()+delta.x());if(m_resize==1||m_resize==3||m_resize==7)r.setRight(r.right()+delta.x());if(m_resize==0||m_resize==1||m_resize==4)r.setTop(r.top()+delta.y());if(m_resize==2||m_resize==3||m_resize==5)r.setBottom(r.bottom()+delta.y());m_selection=r.normalized().intersected(rect());}
 else {if(m_space)m_start+=m_cursor-m_last;m_selection=QRect(m_start,m_cursor).normalized().intersected(rect());}
 m_last=m_cursor;update();
}
void Overlay::mouseReleaseEvent(QMouseEvent* event){
 if(event->button()!=Qt::LeftButton||!m_dragging)return;m_dragging=false;
 if(!m_manualDrag&&m_pressedLocked){m_editor->show();placeToolbar();update();return;}
 if(!m_manualDrag&&!m_pressedLocked)m_selection=captureTargetAt(m_targets,m_cursor,m_elements);
 if(m_selection.width()<8||m_selection.height()<8){clearSelection();return;}
 m_locked=true;prepareEditor();if(m_autoCopy){commit(0);return;}placeToolbar();update();
}
void Overlay::mouseDoubleClickEvent(QMouseEvent* event){if(event->button()==Qt::LeftButton&&m_selection.contains(event->position().toPoint()))commit(0);}
void Overlay::keyReleaseEvent(QKeyEvent* event){if(event->key()==Qt::Key_Alt){m_alt=false;update();}if(event->key()==Qt::Key_Space)m_space=false;}
bool Overlay::eventFilter(QObject* watched,QEvent* event){
 if(watched==m_editor&&event->type()==QEvent::MouseButtonDblClick){auto mouse=static_cast<QMouseEvent*>(event);if(mouse->button()==Qt::LeftButton&&m_locked&&m_editor->rect().contains(mouse->position().toPoint())){commit(0);return true;}}
 if(watched==m_editor&&event->type()==QEvent::KeyPress){auto key=static_cast<QKeyEvent*>(event);if(key->key()==Qt::Key_Escape||key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter||key->modifiers()&Qt::ControlModifier){keyPressEvent(key);return key->isAccepted();}}
 if(watched==m_editor&&event->type()==QEvent::MouseButtonPress){auto mouse=static_cast<QMouseEvent*>(event);if(mouse->button()==Qt::RightButton&&(m_editor->tool()!=7||!m_editor->isDrawing())){selectTool(0);return true;}}
 return QWidget::eventFilter(watched,event);
}
void Overlay::keyPressEvent(QKeyEvent* event){
 const bool ctrl=event->modifiers()&Qt::ControlModifier;
 if(event->key()==Qt::Key_Escape){close();return;}
 if(event->key()==Qt::Key_Alt){m_alt=true;update();return;}
 if(event->key()==Qt::Key_Shift&&!m_dragging){m_rgb=!m_rgb;update();return;}
 if(event->key()==Qt::Key_Tab){if(!m_locked){m_elements=!m_elements;detectAtCursor();}else if(m_editor->tool()==3||m_editor->tool()==7)selectTool(m_editor->tool()==3?7:3);return;}
 if(event->key()==Qt::Key_Space){if(m_dragging)m_space=true;else if(m_locked)m_toolbar->setVisible(!m_toolbar->isVisible());return;}
 if(event->key()==Qt::Key_R&&!ctrl){auto last=QSettings().value("lastCaptureRect").toRect();if(last.isValid())setInitialSelection(QRect(mapFromGlobal(last.topLeft()),last.size()));return;}
 if(event->matches(QKeySequence::Undo)){m_editor->undo();return;}if(event->matches(QKeySequence::Redo)){m_editor->redo();return;}
 if(event->key()==Qt::Key_C&&!ctrl){int x=qBound(0,qRound(m_cursor.x()*m_image.width()/double(width())),m_image.width()-1),y=qBound(0,qRound(m_cursor.y()*m_image.height()/double(height())),m_image.height()-1);auto color=m_image.pixelColor(x,y);QApplication::clipboard()->setText(m_rgb?QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue()):color.name().toUpper());return;}
 if(m_selection.isValid()){
  if(event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter||event->matches(QKeySequence::Copy)){commit(0);return;}
  if(ctrl&&event->key()==Qt::Key_S){commit(event->modifiers()&Qt::ShiftModifier?5:4);return;}
  if(ctrl&&event->key()==Qt::Key_T){commit(3);return;}
  const int dx=event->key()==Qt::Key_Left?-1:event->key()==Qt::Key_Right?1:0,dy=event->key()==Qt::Key_Up?-1:event->key()==Qt::Key_Down?1:0;
  if(dx||dy){m_locked=true;if(ctrl)m_selection.adjust(dx<0?-1:0,dy<0?-1:0,dx>0?1:0,dy>0?1:0);else if(event->modifiers()&Qt::ShiftModifier)m_selection.adjust(dx<0?1:0,dy<0?1:0,dx>0?-1:0,dy>0?-1:0);else m_selection.translate(dx,dy);m_selection=m_selection.intersected(rect());prepareEditor();placeToolbar();update();return;}
 }
 const int dx=event->key()==Qt::Key_A?-1:event->key()==Qt::Key_D?1:0,dy=event->key()==Qt::Key_W?-1:event->key()==Qt::Key_S?1:0;
 if(dx||dy){QCursor::setPos(QCursor::pos()+QPoint(dx,dy));return;}event->ignore();
}
void Overlay::commit(int action){
 m_editor->finishStroke();
 if(m_committed||!m_selection.isValid())return;QImage image=m_locked?m_editor->composed():QImage();if(image.isNull())image=m_image.copy(sourceRect());image.setDevicePixelRatio(1);
 QSettings().setValue("lastCaptureRect",QRect(mapToGlobal(m_selection.topLeft()),m_selection.size()));m_committed=true;emit selected(image,action,QRect(mapToGlobal(m_selection.topLeft()),m_selection.size()));close();
}
void Overlay::closeEvent(QCloseEvent*){if(!m_committed)emit canceled();}

// Outer geometry includes the shadow. Image coordinates always use this inset.
QRect PinWindow::imageRect()const{return rect().adjusted(ShadowMargin,ShadowMargin,-ShadowMargin,-ShadowMargin);}
QRect PinWindow::imageGeometry()const{return QRect(mapToGlobal(imageRect().topLeft()),imageRect().size());}
void PinWindow::setImageGeometry(const QRect& geometry){if(geometry.isValid())setGeometry(geometry.adjusted(-ShadowMargin,-ShadowMargin,ShadowMargin,ShadowMargin));}
void PinWindow::scaleImageAt(double factor,const QPoint& anchor){
 const auto before=imageGeometry();if(!before.isValid()||m_image.isNull())return;
 const int w=qBound(24,qRound(before.width()*factor),4000),h=qMax(1,qRound(w*double(m_image.height())/m_image.width()));
 const double rx=(anchor.x()-before.x())/double(before.width()),ry=(anchor.y()-before.y())/double(before.height());
 setImageGeometry(QRect(anchor-QPoint(qRound(rx*w),qRound(ry*h)),QSize(w,h)));m_thumbnail=false;
}
PinWindow::PinWindow(QImage image,const QRect& placement):m_image(image),m_original(image){
 setAttribute(Qt::WA_DeleteOnClose);setAttribute(Qt::WA_TranslucentBackground);
 setWindowFlags(Qt::Tool|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::NoDropShadowWindowHint);
 setMinimumSize(ShadowMargin*2+1,ShadowMargin*2+1);
 m_controls=new QWidget(this);auto h=new QHBoxLayout(m_controls);h->setContentsMargins(4,3,4,3);
 auto slider=new QSlider(Qt::Horizontal);slider->setRange(20,100);slider->setValue(100);slider->setFixedWidth(70);slider->setToolTip(QStringLiteral("透明度"));
 connect(slider,&QSlider::valueChanged,this,[this](int v){setWindowOpacity(v/100.0);});
 auto closeButton=new GlassButton(QStringLiteral("×"));closeButton->setFixedSize(23,23);connect(closeButton,&QPushButton::clicked,this,&QWidget::close);
 h->addWidget(slider);h->addWidget(closeButton);m_controls->setStyleSheet("QWidget{background:#fcfcfd;color:#23272f;border-radius:4px;}QPushButton{border:0;}");m_controls->adjustSize();m_controls->hide();
 m_originalDisplaySize=placement.isValid()?placement.size():QSize(qMax(1,qRound(image.width()/qMax(qreal(1),image.devicePixelRatio()))),qMax(1,qRound(image.height()/qMax(qreal(1),image.devicePixelRatio()))));
 if(placement.isValid())setImageGeometry(placement);
 else {
  auto screen=QGuiApplication::screenAt(QCursor::pos());if(!screen)screen=QGuiApplication::primaryScreen();
  const auto bounds=screen?screen->availableGeometry():QRect(0,0,1280,800);
  const qreal dpr=qMax(qreal(1),image.devicePixelRatio());
  QSize display(qMax(1,qRound(image.width()/dpr)),qMax(1,qRound(image.height()/dpr)));
  const QSize limit(qMax(1,qMin(650,bounds.width()-2*ShadowMargin)),qMax(1,qMin(500,bounds.height()-2*ShadowMargin)));
  if(display.width()>limit.width()||display.height()>limit.height())display.scale(limit,Qt::KeepAspectRatio);
  const int x=qBound(bounds.left()+ShadowMargin,QCursor::pos().x(),qMax(bounds.left()+ShadowMargin,bounds.right()-display.width()-ShadowMargin+1));
  const int y=qBound(bounds.top()+ShadowMargin,QCursor::pos().y(),qMax(bounds.top()+ShadowMargin,bounds.bottom()-display.height()-ShadowMargin+1));
  setImageGeometry(QRect(QPoint(x,y),display));
 }
 // The first visible frame already has its final global content bounds.
 show();
}
void PinWindow::paintEvent(QPaintEvent*){
 QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.setRenderHint(QPainter::SmoothPixmapTransform);
 const QRectF content=imageRect();p.setPen(Qt::NoPen);
 // Gaussian falloff, displaced downwards. This remains separate from the image payload.
 for(int spread=16;spread>=0;--spread){const int alpha=qMax(1,qRound(8*std::exp(-spread*spread/60.0)));p.setBrush(QColor(20,27,35,alpha));p.drawRoundedRect(content.translated(0,4).adjusted(-spread,-spread,spread,spread),spread*.7,spread*.7);}
 p.drawImage(content,m_image);p.setBrush(Qt::NoBrush);p.setPen(QPen(QColor(23,32,43,62),1));p.drawRect(content.adjusted(-.5,-.5,.5,.5));
}
void PinWindow::mousePressEvent(QMouseEvent*e){if(e->button()==Qt::MiddleButton){reset();return;}if(e->button()==Qt::LeftButton)m_offset=e->globalPosition().toPoint()-pos();}
void PinWindow::mouseMoveEvent(QMouseEvent*e){if(e->buttons()&Qt::LeftButton)move(e->globalPosition().toPoint()-m_offset);}
void PinWindow::setClickThrough(bool enabled){const auto content=imageGeometry();m_clickThrough=enabled;setWindowFlag(Qt::WindowTransparentForInput,enabled);setImageGeometry(content);show();}
void PinWindow::reset(){finishEditing();setWindowOpacity(1);if(auto slider=m_controls->findChild<QSlider*>())slider->setValue(100);m_thumbnail=false;QSize natural=m_originalDisplaySize;if(m_image.width()==m_original.height()&&m_image.height()==m_original.width()&&m_image.size()!=m_original.size())natural.transpose();setImageGeometry(QRect(imageGeometry().topLeft(),natural));update();}
void PinWindow::transform(int action){finishEditing();m_hasEditCapture=false;if(action==1||action==2){const auto content=imageGeometry();QTransform matrix;matrix.rotate(action==1?-90:90);m_image=m_image.transformed(matrix);setImageGeometry(QRect(content.topLeft(),QSize(content.height(),content.width())));}else m_image=m_image.mirrored(action==3,action==4);update();}
void PinWindow::mouseDoubleClickEvent(QMouseEvent*e){if(e->button()!=Qt::LeftButton||m_editing)return;if(e->modifiers()&Qt::ShiftModifier){const auto content=imageGeometry();if(m_thumbnail){setImageGeometry(QRect(content.topLeft(),m_fullSize));m_thumbnail=false;}else{m_fullSize=content.size();setImageGeometry(QRect(content.topLeft(),m_image.size().scaled(180,130,Qt::KeepAspectRatio)));m_thumbnail=true;}}else hide();}
void PinWindow::closeEvent(QCloseEvent* e){finishEditing();if(m_destroy)e->accept();else{e->ignore();hide();}}
void PinWindow::contextMenuEvent(QContextMenuEvent* event){
 QMenu menu;
 menu.addAction(m_editing?QStringLiteral("完成标注  Space"):QStringLiteral("标注 / 编辑图片  Space"),this,[this]{if(m_editing)finishEditing();else startEditing();});
 if(m_editing)menu.addAction(QStringLiteral("取消本次编辑  Esc"),this,[this]{finishEditing(false);});
 menu.addSeparator();menu.addAction(QStringLiteral("复制图片  Ctrl+C"),this,[this]{copyCaptureImage(currentImage(),imageGeometry());});menu.addAction(QStringLiteral("保存图片…  Ctrl+S"),this,&PinWindow::saveImage);
 menu.addAction(QStringLiteral("在工作台中打开"),this,[this]{finishEditing();emit editRequested(m_image);});menu.addSeparator();
 menu.addAction(QStringLiteral("左转  1"),this,[this]{transform(1);});menu.addAction(QStringLiteral("右转  2"),this,[this]{transform(2);});menu.addAction(QStringLiteral("水平镜像  3"),this,[this]{transform(3);});menu.addAction(QStringLiteral("垂直镜像  4"),this,[this]{transform(4);});menu.addAction(QStringLiteral("恢复原始大小与透明度"),this,&PinWindow::reset);
 auto click=menu.addAction(QStringLiteral("鼠标穿透（Ctrl+Alt+P 恢复）"));click->setCheckable(true);click->setChecked(m_clickThrough);connect(click,&QAction::toggled,this,[this](bool enabled){finishEditing();setClickThrough(enabled);});
 menu.addSeparator();menu.addAction(QStringLiteral("隐藏  Esc"),this,&QWidget::hide);menu.addAction(QStringLiteral("销毁  Shift+Esc"),this,[this]{m_destroy=true;close();});menu.exec(event->globalPos());
}
void PinWindow::keyPressEvent(QKeyEvent* event){
 const bool control=event->modifiers()&Qt::ControlModifier;
 if(m_editing){
  if(event->key()==Qt::Key_Escape){finishEditing(false);return;}
  if(event->key()==Qt::Key_Space||event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter){finishEditing();return;}
  if(event->matches(QKeySequence::Undo)){m_pinEditor->undo();return;}
  if(event->matches(QKeySequence::Redo)){m_pinEditor->redo();return;}
 }
 if(event->key()==Qt::Key_Escape){if(event->modifiers()&Qt::ShiftModifier){m_destroy=true;close();}else hide();}
 else if(event->matches(QKeySequence::Close))hide();
 else if(event->matches(QKeySequence::Copy)){if(m_editing)m_pinEditor->finishStroke();copyCaptureImage(currentImage(),imageGeometry());}
 else if(control&&event->key()==Qt::Key_S){if(m_editing)m_pinEditor->finishStroke();if(event->modifiers()&Qt::ShiftModifier)quickSaveImage();else saveImage();}
 else if(event->key()>=Qt::Key_1&&event->key()<=Qt::Key_4)transform(event->key()-Qt::Key_0);
 else if(event->key()==Qt::Key_Space)startEditing();
 else if(event->key()==Qt::Key_Plus||event->key()==Qt::Key_Equal||event->key()==Qt::Key_Minus){int direction=event->key()==Qt::Key_Minus?-1:1;if(control)setWindowOpacity(qBound(.1,windowOpacity()+direction*.05,1.0));else scaleImageAt(direction>0?1.08:1/1.08,imageGeometry().center());}
 else if(event->key()==Qt::Key_C&&(event->modifiers()&Qt::AltModifier)){const auto content=imageRect();const auto point=mapFromGlobal(QCursor::pos())-content.topLeft();int x=qBound(0,point.x()*m_image.width()/qMax(1,content.width()),m_image.width()-1),y=qBound(0,point.y()*m_image.height()/qMax(1,content.height()),m_image.height()-1);QApplication::clipboard()->setText(currentImage().pixelColor(x,y).name().toUpper());}
 else event->ignore();
}
void PinWindow::wheelEvent(QWheelEvent*e){if(e->modifiers()&Qt::ControlModifier){setWindowOpacity(qBound(.1,windowOpacity()+(e->angleDelta().y()>0?.05:-.05),1.0));return;}if(e->angleDelta().y()==0){e->ignore();return;}scaleImageAt(e->angleDelta().y()>0?1.08:1/1.08,e->globalPosition().toPoint());e->accept();}
void PinWindow::enterEvent(QEnterEvent*){if(m_editing)return;const auto content=imageRect();m_controls->move(qMax(content.left()+2,content.right()-m_controls->width()-4),content.top()+5);m_controls->show();}
void PinWindow::leaveEvent(QEvent*){m_controls->hide();}
