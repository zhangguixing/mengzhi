#include "canvas.h"
#include "theme.h"
#include "motion.h"
#include <QPainter>
#include <QPainterPath>
#include <QLineEdit>
#include <cmath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QInputDialog>
#include <QVariantAnimation>
#include <QSignalBlocker>
#include <QtMath>
Canvas::Canvas(QWidget* p):QWidget(p){setFocusPolicy(Qt::StrongFocus);setMouseTracking(true);setMinimumSize(320,250);}
void Canvas::setCapture(Capture* c){stopZoomAnimation();finishStroke(false);setScanning(false);m_capture=c;m_redo.clear();m_pan={};m_zoom=m_zoomTarget=1;rebuild();emit zoomChanged(m_zoom);}
void Canvas::finishStroke(bool apply){
 if(!m_drawing&&!m_panning)return;
 bool committed=false;
 if(m_drawing&&apply&&m_capture&&m_draft.points.size()>1&&(QLineF(m_draft.points.first(),m_draft.points.last()).length()>2||m_draft.points.size()>3)){m_capture->marks.push_back(m_draft);m_redo.clear();committed=true;}
 m_drawing=false;m_panning=false;m_draft={};if(QWidget::mouseGrabber()==this)releaseMouse();rebuild();if(committed)emit changed();
}
void Canvas::setTool(int v){if(v!=m_tool)finishStroke();m_tool=v;setCursor(v==0?Qt::OpenHandCursor:v==5?Qt::IBeamCursor:Qt::CrossCursor);}
void Canvas::stopZoomAnimation(){if(m_zoomAnimation)m_zoomAnimation->stop();m_zoomTarget=m_zoom;}
void Canvas::applyZoomAt(qreal value,const QPointF& anchor){
 const auto before=imageRect();
 QPointF relative;
 const bool anchored=!m_direct&&!before.isEmpty();
 if(anchored)relative=QPointF((anchor.x()-before.x())/before.width(),(anchor.y()-before.y())/before.height());
 m_zoom=value;
 if(anchored){const auto after=imageRect();m_pan+=anchor-QPointF(after.x()+relative.x()*after.width(),after.y()+relative.y()*after.height());}
 update();emit zoomChanged(m_zoom);
}
void Canvas::setZoomAt(qreal value,const QPointF& anchor,bool animated){
 if(!std::isfinite(value))return;
 stopZoomAnimation();m_zoomTarget=qBound(.25,value,8.0);m_zoomAnchor=anchor;
 if(!animated||reducedMotion()||m_direct||qFuzzyCompare(m_zoom,m_zoomTarget)){applyZoomAt(m_zoomTarget,anchor);return;}
 if(!m_zoomAnimation){
  m_zoomAnimation=new QVariantAnimation(this);m_zoomAnimation->setDuration(140);m_zoomAnimation->setEasingCurve(QEasingCurve::OutCubic);
  connect(m_zoomAnimation,&QVariantAnimation::valueChanged,this,[this](const QVariant& value){applyZoomAt(value.toReal(),m_zoomAnchor);});
 }
 // Reconfiguring a stopped animation at its old time must not paint a stale frame.
 {QSignalBlocker blocker(m_zoomAnimation);m_zoomAnimation->setStartValue(m_zoom);m_zoomAnimation->setEndValue(m_zoomTarget);m_zoomAnimation->setCurrentTime(0);}
 m_zoomAnimation->start();
}
void Canvas::setZoom(qreal value){setZoomAt(value,QRectF(rect()).center());}
void Canvas::fitToWindow(){stopZoomAnimation();m_pan={};m_zoom=m_zoomTarget=1;update();emit zoomChanged(m_zoom);}
QPointF Canvas::imagePointAt(const QPointF& point)const{if(!m_capture||imageRect().isEmpty())return {};return toImage(point);}
void Canvas::rebuild(){m_composed=m_capture?m_capture->image:QImage();if(m_capture)for(const auto& a:m_capture->marks)paintMark(m_composed,a,&m_capture->image);emit historyChanged(m_capture&&!m_capture->marks.isEmpty(),!m_redo.isEmpty());update();}
QImage Canvas::composed()const{return m_composed;}
void Canvas::undo(){if(!m_capture||m_capture->marks.isEmpty())return;m_redo.push_back(m_capture->marks.takeLast());rebuild();emit changed();}
void Canvas::redo(){if(!m_capture||m_redo.isEmpty())return;m_capture->marks.push_back(m_redo.takeLast());rebuild();emit changed();}
QRectF Canvas::imageRect()const{if(!m_capture||m_capture->image.isNull())return {};if(m_direct)return QRectF(rect());auto sz=QSizeF(m_capture->image.size());const auto inset=property("studyCanvasInset");const qreal reserveX=inset.isValid()?2*qMax(0.0,inset.toReal()):80.0;const qreal reserveY=inset.isValid()?2*qMax(0.0,inset.toReal()):90.0;qreal fit=qMax(.001,qMin((width()-reserveX)/sz.width(),(height()-reserveY)/sz.height()));sz*=qMin(1.0,fit)*m_zoom;return QRectF(QPointF((width()-sz.width())/2,(height()-sz.height())/2)+m_pan,sz);}
QPointF Canvas::toImage(const QPointF& p)const{const auto r=imageRect();return {qBound(0.0,(p.x()-r.x())/r.width()*m_capture->image.width(),double(m_capture->image.width())),qBound(0.0,(p.y()-r.y())/r.height()*m_capture->image.height(),double(m_capture->image.height()))};}
void Canvas::paintMark(QImage& img,const Mark& a,const QImage* original){if(a.points.isEmpty())return;auto p=a.points.first(),q=a.points.last();auto r=QRectF(p,q).normalized();if(a.tool==6){QRect rect=r.toAlignedRect().intersected(img.rect());if(rect.isEmpty())return;auto pix=img.copy(rect).scaled(qMax(1,rect.width()/14),qMax(1,rect.height()/14),Qt::IgnoreAspectRatio,Qt::FastTransformation).scaled(rect.size(),Qt::IgnoreAspectRatio,Qt::FastTransformation);QPainter painter(&img);painter.drawImage(rect.topLeft(),pix);return;}
 if(a.tool==9){QRect area=r.toAlignedRect().intersected(img.rect());if(area.isEmpty())return;auto blurred=img.copy(area).scaled(qMin(400,area.width()),qMin(400,area.height()),Qt::IgnoreAspectRatio,Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);auto pass=[&](QImage input,bool horizontal){QImage output(input.size(),input.format());const double k[]={.06136,.24477,.38774,.24477,.06136};for(int y=0;y<input.height();y++){auto line=reinterpret_cast<QRgb*>(output.scanLine(y));for(int x=0;x<input.width();x++){double red=0,green=0,blue=0,alpha=0;for(int n=-2;n<=2;n++){int xx=qBound(0,x+(horizontal?n:0),input.width()-1),yy=qBound(0,y+(horizontal?0:n),input.height()-1);auto pixel=reinterpret_cast<const QRgb*>(input.constScanLine(yy))[xx];red+=qRed(pixel)*k[n+2];green+=qGreen(pixel)*k[n+2];blue+=qBlue(pixel)*k[n+2];alpha+=qAlpha(pixel)*k[n+2];}line[x]=qRgba(qRound(red),qRound(green),qRound(blue),qRound(alpha));}}return output;};for(int i=0;i<3;i++){blurred=pass(blurred,true);blurred=pass(blurred,false);}QPainter painter(&img);painter.setRenderHint(QPainter::SmoothPixmapTransform);painter.drawImage(area,blurred);return;}if(a.tool==10&&original){QPainterPath path;path.moveTo(p);for(auto point:a.points)path.lineTo(point);QPainterPathStroker stroke;stroke.setWidth(a.width*8);stroke.setCapStyle(Qt::RoundCap);stroke.setJoinStyle(Qt::RoundJoin);QPainter painter(&img);painter.setClipPath(stroke.createStroke(path));painter.drawImage(0,0,*original);return;}QPainter painter(&img);painter.setRenderHint(QPainter::Antialiasing);painter.setPen(QPen(a.color,a.width,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
 if(a.filled&&(a.tool==1||a.tool==2))painter.setBrush(a.color);if(a.tool==1)painter.drawRect(r);if(a.tool==2)painter.drawEllipse(r);
 if(a.tool==3){painter.drawLine(p,q);qreal angle=std::atan2(q.y()-p.y(),q.x()-p.x()),size=a.width*4+7;QPolygonF head;head<<q<<QPointF(q.x()-size*std::cos(angle-.45),q.y()-size*std::sin(angle-.45))<<QPointF(q.x()-size*std::cos(angle+.45),q.y()-size*std::sin(angle+.45));painter.setBrush(a.color);painter.drawPolygon(head);}
 if(a.tool==4||a.tool==7||a.tool==8){if(a.tool==8){auto high=a.color;high.setAlphaF(.32);painter.setPen(QPen(high,a.width*6,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));}QPainterPath path;path.moveTo(p);for(auto pt:a.points)path.lineTo(pt);painter.drawPath(path);}
 if(a.tool==5){QFont f=painter.font();f.setPixelSize(qMax(22,int(a.width*7)));f.setWeight(QFont::Medium);painter.setFont(f);painter.drawText(p,a.text);}
}
void Canvas::paintEvent(QPaintEvent*){QPainter p(this);p.fillRect(rect(),palette().color(QPalette::Window));if(!m_capture||m_capture->image.isNull()){p.setPen(QColor(Theme::current().muted));p.drawText(rect(),Qt::AlignCenter,QStringLiteral("从一张截图开始\n\n导入图片，或按 ⌘/Ctrl + Shift + 2 截图"));return;}const auto r=imageRect();p.setPen(QColor(35,47,66,22));p.setBrush(QColor(35,47,66,10));p.drawRect(r.adjusted(-1,-1,1,2));p.setRenderHint(QPainter::SmoothPixmapTransform);if(m_drawing){auto img=m_composed;paintMark(img,m_draft,&m_capture->image);p.drawImage(r,img);}else p.drawImage(r,m_composed);if(m_scanning&&!reducedMotion()){p.save();p.setClipRect(r);qreal y=r.top()+r.height()*m_scanProgress;QLinearGradient light(0,y-45,0,y+5);light.setColorAt(0,QColor(75,149,245,0));light.setColorAt(.85,QColor(75,149,245,32));light.setColorAt(1,QColor(75,149,245,0));p.fillRect(QRectF(r.left(),y-45,r.width(),50),light);p.setPen(QPen(QColor(75,149,245,105),1));p.drawLine(QPointF(r.left(),y),QPointF(r.right(),y));p.restore();}}
void Canvas::mousePressEvent(QMouseEvent* e){if(!m_capture)return;stopZoomAnimation();if(e->button()==Qt::MiddleButton&&!m_direct){if(m_drawing)return;setFocus();m_panning=true;m_panButton=Qt::MiddleButton;m_down=e->position();m_startPan=m_pan;setCursor(Qt::ClosedHandCursor);e->accept();return;}if(e->button()==Qt::RightButton){if(m_drawing&&m_tool==7){m_draft.points.removeLast();m_capture->marks.push_back(m_draft);m_drawing=false;m_redo.clear();rebuild();emit changed();}return;}if(e->button()!=Qt::LeftButton)return;setFocus();if(m_tool==0){m_panning=true;m_panButton=Qt::LeftButton;m_down=e->position();m_startPan=m_pan;setCursor(Qt::ClosedHandCursor);return;}if(!imageRect().contains(e->position()))return;auto pt=toImage(e->position());if(m_tool==7&&m_drawing){m_draft.points.last()=pt;m_draft.points.push_back(pt);return;}m_draft={m_tool,{pt,pt},color,lineWidth,{},filled};if(m_tool==5){bool ok=false;auto text=QInputDialog::getText(this,QStringLiteral("添加标注"),QStringLiteral("标注文字"),QLineEdit::Normal,{},&ok);if(ok&&!text.isEmpty()){m_draft.text=text;m_capture->marks.push_back(m_draft);m_redo.clear();rebuild();emit changed();}return;}m_drawing=true;if(m_tool!=7)grabMouse();}
void Canvas::mouseMoveEvent(QMouseEvent* e){if(m_panning){m_pan=m_startPan+e->position()-m_down;update();return;}if(!m_drawing)return;auto p=toImage(e->position());if(e->modifiers()&Qt::ShiftModifier){auto first=m_draft.points.first();auto delta=p-first;if(m_tool==1||m_tool==2){auto size=qMax(qAbs(delta.x()),qAbs(delta.y()));p=first+QPointF(delta.x()<0?-size:size,delta.y()<0?-size:size);}else if(m_tool==3||m_tool==7||m_tool==8){auto length=std::hypot(delta.x(),delta.y());auto angle=std::round(std::atan2(delta.y(),delta.x())/(M_PI/4))*(M_PI/4);p=first+QPointF(std::cos(angle)*length,std::sin(angle)*length);}}if(m_tool==4||m_tool==8||m_tool==10)m_draft.points.push_back(p);else m_draft.points.last()=p;update();}
void Canvas::mouseReleaseEvent(QMouseEvent* e){if(m_panning&&e->button()==m_panButton){m_panning=false;m_panButton=Qt::NoButton;setTool(m_tool);e->accept();return;}if(e->button()!=Qt::LeftButton)return;if(m_drawing&&m_tool!=7){m_drawing=false;releaseMouse();if(QLineF(m_draft.points.first(),m_draft.points.last()).length()>2||m_draft.points.size()>3){m_capture->marks.push_back(m_draft);m_redo.clear();emit changed();}rebuild();}}
void Canvas::mouseDoubleClickEvent(QMouseEvent* e){if(!m_direct&&m_tool==0&&e->button()==Qt::LeftButton){m_panning=false;m_panButton=Qt::NoButton;setTool(m_tool);fitToWindow();e->accept();return;}QWidget::mouseDoubleClickEvent(e);}
void Canvas::keyPressEvent(QKeyEvent* e){if(e->key()==Qt::Key_Escape){if(m_drawing){m_drawing=false;releaseMouse();update();}m_panning=false;setTool(0);}else QWidget::keyPressEvent(e);}
void Canvas::wheelEvent(QWheelEvent* e){if(m_tool!=0&&!(e->modifiers()&Qt::ControlModifier)){lineWidth=qBound(1.0,lineWidth+(e->angleDelta().y()>0?1:-1),20.0);return;}if(e->modifiers()&Qt::ControlModifier){const qreal delta=e->angleDelta().y()!=0?e->angleDelta().y():e->pixelDelta().y();if(delta!=0){const qreal target=m_zoomAnimation&&m_zoomAnimation->state()==QAbstractAnimation::Running?m_zoomTarget:m_zoom;setZoomAt(target*std::pow(1.15,delta/120.0),e->position(),!m_drawing);}e->accept();}else{stopZoomAnimation();m_pan+=e->pixelDelta().isNull()?QPointF(e->angleDelta().x()/3.0,e->angleDelta().y()/3.0):QPointF(e->pixelDelta());update();e->accept();}}
void Canvas::resizeEvent(QResizeEvent*){update();}

void Canvas::setScanning(bool enabled){m_scanning=enabled;if(m_scanAnimation){m_scanAnimation->stop();m_scanAnimation->deleteLater();m_scanAnimation=nullptr;}if(enabled&&!reducedMotion()){m_scanAnimation=new QPropertyAnimation(this,"scanProgress",this);m_scanAnimation->setDuration(1700);m_scanAnimation->setStartValue(0.0);m_scanAnimation->setEndValue(1.0);m_scanAnimation->setLoopCount(-1);m_scanAnimation->setEasingCurve(QEasingCurve::InOutSine);m_scanAnimation->start();}update();}
