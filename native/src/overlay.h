#pragma once
#include <QWidget>
#include <QImage>
#include "model.h"
class QScreen;class QFrame;class Canvas;
class Overlay : public QWidget {
 Q_OBJECT
public:Overlay(QScreen* screen,QImage image);
signals:void selected(QImage image,int action);void canceled();
protected:void paintEvent(QPaintEvent*)override;void mousePressEvent(QMouseEvent*)override;void mouseMoveEvent(QMouseEvent*)override;void mouseReleaseEvent(QMouseEvent*)override;void mouseDoubleClickEvent(QMouseEvent*)override;void keyReleaseEvent(QKeyEvent*)override;void keyPressEvent(QKeyEvent*)override;void closeEvent(QCloseEvent*)override;
private:QImage m_image;QPoint m_start,m_cursor,m_last;QRect m_selection,m_initial;bool m_dragging=false,m_committed=false,m_space=false,m_moving=false,m_alt=false,m_rgb=false;int m_resize=-1;QFrame* m_toolbar;QWidget* m_drawTools;Canvas* m_editor;Capture m_capture;void commit(int action);void prepareEditor();void placeToolbar();QRect sourceRect()const;
};
class PinWindow : public QWidget {
 Q_OBJECT
public:explicit PinWindow(QImage image);void setClickThrough(bool enabled);QString group=QStringLiteral("默认");
signals:void editRequested(QImage image);
protected:void paintEvent(QPaintEvent*)override;void mousePressEvent(QMouseEvent*)override;void mouseMoveEvent(QMouseEvent*)override;void mouseDoubleClickEvent(QMouseEvent*)override;void contextMenuEvent(QContextMenuEvent*)override;void closeEvent(QCloseEvent*)override;void keyPressEvent(QKeyEvent*)override;void wheelEvent(QWheelEvent*)override;void enterEvent(QEnterEvent*)override;void leaveEvent(QEvent*)override;
private:QImage m_image,m_original;QPoint m_offset;QWidget* m_controls;QSize m_fullSize;bool m_thumbnail=false,m_destroy=false,m_clickThrough=false;void reset();void transform(int action);
};
