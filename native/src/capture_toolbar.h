#pragma once

#include <QFrame>
#include <QHash>
#include <QPointer>

class Canvas;
class QAbstractButton;

// Snipaste-style capture rail shared by selection overlays and editable pins.
// Actions: 0 copy, 1 workbench, 2 OCR, 3 pin, 4 save, 5 quick save,
//          6 cancel, 7 finish editing a pin.
class CaptureToolbar final : public QFrame {
    Q_OBJECT
public:
    explicit CaptureToolbar(Canvas* editor, QWidget* parent = nullptr);
    ~CaptureToolbar() override;
    void setEditingPin(bool editing);
    QSize sizeHint() const override;

public slots:
    void selectTool(int tool);

signals:
    void actionTriggered(int action);
    void toolChanged(int tool);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QPointer<Canvas> m_editor;
    QWidget* m_properties = nullptr;
    QAbstractButton* m_shape = nullptr;
    QAbstractButton* m_rectangle = nullptr;
    QAbstractButton* m_ellipse = nullptr;
    QAbstractButton* m_width = nullptr;
    QAbstractButton* m_fill = nullptr;
    QAbstractButton* m_swatch = nullptr;
    QAbstractButton* m_pin = nullptr;
    QHash<int, QAbstractButton*> m_tools;
    int m_shapeTool = 1;
    bool m_editingPin = false;
    void refreshProperties();
    void showMoreMenu(const QPoint& globalPosition);
};
