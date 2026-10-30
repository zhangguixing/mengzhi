#pragma once
#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QPainterPath>

// One 24-unit optical grid. Qt renders paths at the requested monitor scale.
class CaptureIconEngine final : public QIconEngine {
 int m_tool;QColor m_ink;
public:
 CaptureIconEngine(int tool,QColor ink):m_tool(tool),m_ink(ink){}
 QIconEngine* clone() const override{return new CaptureIconEngine(m_tool,m_ink);}
 QString key() const override{return QStringLiteral("MengzhiCaptureIcon");}
 void paint(QPainter* painter,const QRect& rect,QIcon::Mode mode,QIcon::State state) override{
  auto ink=m_ink;if(mode==QIcon::Disabled)ink.setAlpha(80);
  else if(state==QIcon::On&&ink.lightness()<180)ink=QColor("#146b9f");
  painter->save();painter->translate(rect.topLeft());painter->scale(rect.width()/24.0,rect.height()/24.0);
  painter->setRenderHint(QPainter::Antialiasing);painter->setBrush(Qt::NoBrush);
  painter->setPen(QPen(ink,1.75,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));auto& p=*painter;
  if(m_tool==0)p.drawPolyline(QPolygonF{{5,3},{5,20},{10,15},{14,21},{17,19},{13,13},{20,12},{5,3}});
  else if(m_tool==1)p.drawRoundedRect(QRectF(3.5,4.5,17,15),1.5,1.5);
  else if(m_tool==2)p.drawEllipse(QRectF(3.5,4.5,17,15));
  else if(m_tool==3){p.drawLine(QPointF(5,19),QPointF(19,5));p.drawPolyline(QPolygonF{{10,5},{19,5},{19,14}});}
  else if(m_tool==7){p.drawLine(QPointF(5,19),QPointF(19,5));p.setBrush(ink);p.drawEllipse(QPointF(5,19),1.2,1.2);p.drawEllipse(QPointF(19,5),1.2,1.2);}
  else if(m_tool==4){p.drawPolygon(QPolygonF{{4,20},{5.5,14.5},{16.5,3.5},{20.5,7.5},{9.5,18.5}});p.drawLine(QPointF(13.5,6.5),QPointF(17.5,10.5));p.drawLine(QPointF(5.5,14.5),QPointF(9.5,18.5));}
  else if(m_tool==8){p.drawPolyline(QPolygonF{{5,16},{4,19},{7,18},{9,16}});p.drawPolygon(QPolygonF{{6,13},{15,4},{20,9},{11,18}});p.drawLine(4,22,18,22);}
  else if(m_tool==5){p.drawPolyline(QPolygonF{{4,7},{4,4},{20,4},{20,7}});p.drawLine(12,4,12,20);p.drawLine(8,20,16,20);}
  else if(m_tool==6){for(int y=4;y<=16;y+=6)for(int x=4;x<=16;x+=6){p.save();if(((x+y)/6)%2){auto fill=ink;fill.setAlpha(140);p.setBrush(fill);}p.drawRoundedRect(QRectF(x,y,4,4),.4,.4);p.restore();}}
  else if(m_tool==9){for(int y=5;y<=19;y+=7)for(int x=5;x<=19;x+=7){auto color=ink;color.setAlpha((x+y)%3?185:70);p.setPen(Qt::NoPen);p.setBrush(color);p.drawEllipse(QPointF(x,y),1.6,1.6);}}
  else if(m_tool==10){p.drawPolygon(QPolygonF{{3.5,13},{13,3.5},{20.5,11},{12,19.5},{9,19.5}});p.drawLine(QPointF(7,9.5),QPointF(15,17.5));p.drawLine(12,20,21,20);}
  else if(m_tool==11||m_tool==12){if(m_tool==12){p.translate(24,0);p.scale(-1,1);}QPainterPath path;path.moveTo(4,9);path.lineTo(14,9);path.cubicTo(22,9,22,20,13,20);p.drawPath(path);p.drawPolyline(QPolygonF{{9,4},{4,9},{9,14}});}
  else if(m_tool==13){p.drawPolyline(QPolygonF{{8,3},{17,3},{15,7},{15,11},{19,15},{5,15},{9,11},{9,7},{8,3}});p.drawLine(12,15,12,22);}
  else if(m_tool==14){p.drawPolyline(QPolygonF{{4,3},{17,3},{21,7},{21,21},{3,21},{3,3},{4,3}});p.drawRect(QRectF(7,3,9,6));p.drawRoundedRect(QRectF(7,14,10,7),.5,.5);p.drawLine(13,4,13,8);}
  else if(m_tool==15){p.drawRoundedRect(QRectF(8,8,13,13),2,2);p.drawPolyline(QPolygonF{{5,16},{3,16},{3,3},{16,3},{16,5}});}
  else if(m_tool==16){p.drawLine(6,6,18,18);p.drawLine(6,18,18,6);}
  else if(m_tool==17){p.setPen(Qt::NoPen);p.setBrush(ink);for(int x:{5,12,19})p.drawEllipse(QPointF(x,12),1.4,1.4);}
  else if(m_tool==18){p.drawPolyline(QPolygonF{{3,8},{3,3},{8,3}});p.drawPolyline(QPolygonF{{16,3},{21,3},{21,8}});p.drawPolyline(QPolygonF{{3,16},{3,21},{8,21}});p.drawPolyline(QPolygonF{{16,21},{21,21},{21,16}});p.drawLine(7,9,17,9);p.drawLine(7,13,17,13);p.drawLine(7,17,13,17);}
  else if(m_tool==19){p.drawLine(12,3,12,16);p.drawPolyline(QPolygonF{{7,11},{12,16},{17,11}});p.drawPolyline(QPolygonF{{4,15},{4,21},{20,21},{20,15}});}
  else if(m_tool==20){p.drawLine(5,12,19,12);p.drawLine(12,5,12,19);}
  else if(m_tool==21)p.drawLine(5,12,19,12);
  else if(m_tool==22){p.drawRoundedRect(QRectF(3,3,18,18),2,2);p.drawEllipse(QPointF(8,8),1.4,1.4);p.drawPolyline(QPolygonF{{4,18},{9,13},{12,16},{17,10},{21,14}});}
  else if(m_tool==23){p.drawLine(4,6,20,6);p.drawPolyline(QPolygonF{{6,6},{7,21},{17,21},{18,6}});p.drawPolyline(QPolygonF{{9,6},{9,3},{15,3},{15,6}});p.drawLine(10,10,10,17);p.drawLine(14,10,14,17);}
  else if(m_tool==24){p.drawLine(4,6,20,6);p.drawLine(4,12,20,12);p.drawLine(4,18,20,18);p.setBrush(QColor("#fafafa"));p.drawEllipse(QPointF(9,6),2.5,2.5);p.drawEllipse(QPointF(16,12),2.5,2.5);p.drawEllipse(QPointF(8,18),2.5,2.5);}
  else if(m_tool==25)p.drawPolyline(QPolygonF{{4,12},{9,17},{20,6}});
  else if(m_tool==26){p.drawPolyline(QPolygonF{{3,8},{3,3},{8,3}});p.drawPolyline(QPolygonF{{16,3},{21,3},{21,8}});p.drawPolyline(QPolygonF{{3,16},{3,21},{8,21}});p.drawPolyline(QPolygonF{{16,21},{21,21},{21,16}});}
  else if(m_tool==27){p.drawRoundedRect(QRectF(3,4,18,16),2,2);p.drawPolyline(QPolygonF{{7,9},{7,7},{10,7}});p.drawPolyline(QPolygonF{{14,17},{17,17},{17,14}});}
  painter->restore();
 }
 QPixmap pixmap(const QSize& size,QIcon::Mode mode,QIcon::State state) override{
  QPixmap image(size);image.fill(Qt::transparent);QPainter painter(&image);paint(&painter,QRect(QPoint(),size),mode,state);return image;
 }
};
inline QIcon captureIcon(int tool,QColor ink=QColor("#46515e")){return QIcon(new CaptureIconEngine(tool,ink));}
