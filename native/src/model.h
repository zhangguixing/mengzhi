#pragma once
#include <QImage>
#include <QColor>
#include <QPointF>
#include <QVector>
#include <QStringList>
struct Mark { int tool=0; QVector<QPointF> points; QColor color=QColor("#dc4c4c"); qreal width=3; QString text; bool filled=false; };
struct OcrBlock { QString text; QRectF bounds; double confidence=-1; };
struct Capture { QString name; QImage image; QVector<Mark> marks; QString text; QList<QStringList> table; QVector<OcrBlock> blocks; QString engine; bool sample=false; bool ocrComplete=false; QByteArray encoded; };
