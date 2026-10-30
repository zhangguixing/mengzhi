#pragma once
#include <QWidget>
#include <QImage>
#include "model.h"
#include "capture_targets.h"
class QScreen;class QFrame;class Canvas;class QButtonGroup;
class Overlay : public QWidget {
 Q_OBJECT
public:Overlay(QScreen* screen,QImage image);void setInitialSelection(const QRect& selection);void setAutoCopy(bool enabled){m_autoCopy=enabled;}void finish(){commit(0);}
signals:void selected(QImage image,int action,QRect captureGeometry);void canceled();
protected:bool eventFilter(QObject*,QEvent*)override;void paintEvent(QPaintEvent*)override;void mousePressEvent(QMouseEvent*)override;void mouseMoveEvent(QMouseEvent*)override;void mouseReleaseEvent(QMouseEvent*)override;void mouseDoubleClickEvent(QMouseEvent*)override;void keyReleaseEvent(QKeyEvent*)override;void keyPressEvent(QKeyEvent*)override;void closeEvent(QCloseEvent*)override;
private:QVector<CaptureTarget> m_targets;bool m_autoCopy=false,m_locked=false,m_pressedLocked=false,m_elements=false,m_manualDrag=false;void detectAtCursor();void clearSelection();void selectTool(int tool);QButtonGroup* m_tools=nullptr;QWidget* m_properties=nullptr;QImage m_image;QPoint m_start,m_cursor,m_last;QRect m_selection,m_initial;bool m_dragging=false,m_committed=false,m_space=false,m_moving=false,m_alt=false,m_rgb=false;int m_resize=-1;QFrame* m_toolbar;QWidget* m_drawTools;Canvas* m_editor;Capture m_capture;void commit(int action);void prepareEditor();void placeToolbar();QRect sourceRect()const;
};
class PinWindow : public QWidget {
 Q_OBJECT
public:explicit PinWindow(QImage image,const QRect& placement=QRect());QRect imageGeometry()const;void setClickThrough(bool enabled);void startEditing();void finishEditing(bool apply=true);QString group=QStringLiteral("默认");
signals:void editRequested(QImage image);void recognizeRequested(QImage image);
protected:bool eventFilter(QObject*,QEvent*)override;void paintEvent(QPaintEvent*)override;void mousePressEvent(QMouseEvent*)override;void mouseMoveEvent(QMouseEvent*)override;void mouseDoubleClickEvent(QMouseEvent*)override;void contextMenuEvent(QContextMenuEvent*)override;void closeEvent(QCloseEvent*)override;void keyPressEvent(QKeyEvent*)override;void wheelEvent(QWheelEvent*)override;void enterEvent(QEnterEvent*)override;void leaveEvent(QEvent*)override;void moveEvent(QMoveEvent*)override;void resizeEvent(QResizeEvent*)override;void hideEvent(QHideEvent*)override;
private:static constexpr int ShadowMargin=20;QRect imageRect()const;void setImageGeometry(const QRect& geometry);void scaleImageAt(double factor,const QPoint& anchor);QImage m_image,m_original;QPoint m_offset;QWidget* m_controls=nullptr;QSize m_fullSize,m_originalDisplaySize;bool m_thumbnail=false,m_destroy=false,m_clickThrough=false,m_editing=false,m_hasEditCapture=false;Canvas* m_pinEditor=nullptr;QFrame* m_editToolbar=nullptr;Capture m_editCapture;QVector<Mark> m_beforeEdit;void createEditToolbar();void placeEditToolbar();QImage currentImage()const;void saveImage();void quickSaveImage();void reset();void transform(int action);
};
