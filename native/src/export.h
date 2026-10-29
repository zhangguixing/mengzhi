#pragma once
#include "model.h"
namespace Export {
 QByteArray zip(const QList<QPair<QString,QByteArray>>& files);
 QByteArray word(const Capture& document);
 QByteArray excel(const QList<QStringList>& table);
 bool pdf(const QString& path,const QImage& image,QString* error);
 QList<QStringList> inferTable(const QVector<OcrBlock>& blocks);
}
