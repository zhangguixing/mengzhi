#pragma once
#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>
#include <QRect>
#include <QUuid>

// Metadata accompanies the standard image payload; other applications still see a normal image.
inline void copyCaptureImage(const QImage& image,const QRect& geometry) {
 if(image.isNull())return;
 auto mime=new QMimeData;
 mime->setImageData(image);
 if(geometry.isValid()) {
  QJsonObject value{{"version",1},{"uuid",QUuid::createUuid().toString(QUuid::WithoutBraces)},
                    {"x",geometry.x()},{"y",geometry.y()},{"width",geometry.width()},{"height",geometry.height()}};
  mime->setData("application/x-mengzhi-capture-geometry",QJsonDocument(value).toJson(QJsonDocument::Compact));
 }
 QApplication::clipboard()->setMimeData(mime);
}

inline QRect captureClipboardGeometry(const QMimeData* mime) {
 if(!mime||!mime->hasImage())return {};
 auto data=mime->data("application/x-mengzhi-capture-geometry");
 if(data.isEmpty()||data.size()>2048)return {};
 auto value=QJsonDocument::fromJson(data).object();
 if(value.value("version").toInt()!=1||QUuid(value.value("uuid").toString()).isNull())return {};
 for(const auto& key:{"x","y","width","height"})if(!value.value(key).isDouble())return {};
 int x=value.value("x").toInt(),y=value.value("y").toInt();
 int w=value.value("width").toInt(),h=value.value("height").toInt();
 if(w<1||h<1||w>100000||h>100000||x < -100000||x>100000||y < -100000||y>100000)return {};
 return QRect(x,y,w,h);
}

inline QString captureClipboardId(const QMimeData* mime) {
 if(!captureClipboardGeometry(mime).isValid())return {};
 return QJsonDocument::fromJson(mime->data("application/x-mengzhi-capture-geometry")).object().value("uuid").toString();
}
