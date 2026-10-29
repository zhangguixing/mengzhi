#pragma once
#include "model.h"
#include <QWidget>
class Canvas : public QWidget {
 Q_OBJECT
 Q_PROPERTY(qreal scanProgress READ scanProgress WRITE setScanProgress)
public:
 explicit Canvas(QWidget* parent=nullptr);
 void setCapture(Capture* capture);
 void setTool(int value);void setDirectMode(bool value){m_direct=value;update();}
 void setZoom(qreal value);
 void undo(); void redo();void setScanning(bool enabled);qreal scanProgress()const{return m_scanProgress;}void setScanProgress(qreal value){m_scanProgress=value;update();}
 QImage composed() const;
 QColor color=QColor("#dc4c4c"); qreal lineWidth=3;
 int tool() const {return m_tool;} qreal zoom() const{return m_zoom;}
 static void paintMark(QImage& image,const Mark& mark,const QImage* original=nullptr);
signals:
 void changed();void zoomChanged(qreal);
protected:
 void paintEvent(QPaintEvent*) override;
 void mousePressEvent(QMouseEvent*) override;
 void mouseMoveEvent(QMouseEvent*) override;
 void mouseReleaseEvent(QMouseEvent*) override;
 void keyPressEvent(QKeyEvent*) override;
 void wheelEvent(QWheelEvent*) override;
 void resizeEvent(QResizeEvent*) override;
private:
 QRectF imageRect() const;QPointF toImage(const QPointF&) const;
 Capture* m_capture=nullptr;int m_tool=0;qreal m_zoom=1;QPointF m_pan,m_startPan,m_down;
 bool m_drawing=false,m_panning=false,m_scanning=false,m_direct=false;qreal m_scanProgress=0;class QPropertyAnimation* m_scanAnimation=nullptr;Mark m_draft;QVector<Mark> m_redo;QImage m_composed;
 void rebuild();
};
