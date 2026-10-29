#pragma once
#include "motion.h"
#include <QPushButton>
#include <QToolButton>
#include <QVariantAnimation>
#include <QPainter>
#include <QPainterPath>
#include <QEnterEvent>

// Lightweight glass highlight; no idle animation or rendering engine.
template<class Base> class GlassControl : public Base {
public:
 explicit GlassControl(const QString& text={},QWidget* parent=nullptr):Base(parent){
  this->setText(text);this->setCursor(Qt::PointingHandCursor);
  motion.setDuration(150);motion.setEasingCurve(QEasingCurve::OutCubic);
  QObject::connect(&motion,&QVariantAnimation::valueChanged,this,[this](const QVariant& v){light=v.toReal();this->update();});
 }
protected:
 void enterEvent(QEnterEvent* e)override{animate(1);Base::enterEvent(e);}
 void leaveEvent(QEvent* e)override{animate(0);Base::leaveEvent(e);}
 void mousePressEvent(QMouseEvent* e)override{animate(.2);Base::mousePressEvent(e);}
 void mouseReleaseEvent(QMouseEvent* e)override{Base::mouseReleaseEvent(e);animate(this->underMouse()?1:0);}
 void paintEvent(QPaintEvent* e)override{
  Base::paintEvent(e);if(!this->isEnabled()||light<=0)return;
  QPainter p(this);p.setRenderHint(QPainter::Antialiasing);QPainterPath clip;clip.addRoundedRect(QRectF(this->rect()).adjusted(1,1,-1,-1),qMin(17,this->height()/2),qMin(17,this->height()/2));p.setClipPath(clip);
  QLinearGradient shine(0,0,0,this->height());shine.setColorAt(0,QColor(255,255,255,qRound(24*light)));shine.setColorAt(1,QColor(255,255,255,0));p.fillRect(this->rect(),shine);
 }
private:
 QVariantAnimation motion;qreal light=0;
 void animate(qreal target){motion.stop();if(reducedMotion()){light=target;this->update();return;}motion.setStartValue(light);motion.setEndValue(target);motion.start();}
};
using GlassButton=GlassControl<QPushButton>;
using GlassToolButton=GlassControl<QToolButton>;
