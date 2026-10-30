#pragma once
#include <QRect>
#include <QVector>
class QScreen;
struct CaptureTarget {QRect bounds;int root=0;bool element=false;};
QVector<CaptureTarget> captureTargets(QScreen* screen);
QRect captureTargetAt(const QVector<CaptureTarget>& targets,const QPoint& point,bool elements);
