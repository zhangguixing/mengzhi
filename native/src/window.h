#pragma once
#include "model.h"
#include <QMainWindow>
#include <QPointer>
#include <memory>
#include <vector>
class Canvas;class QListWidget;class QPlainTextEdit;class QTableWidget;class QTabWidget;class QPushButton;class QLabel;class QSystemTrayIcon;class QProcess;class QTemporaryDir;class Hotkey;class QTimer;class PinWindow;
class Window : public QMainWindow {
 Q_OBJECT
public:
 explicit Window();
 ~Window();
 void openImage(const QString& path);
 void startCapture();
 void saveScreenshot(const QString& path);
 void renderPreview(const QString& directory);
protected:
 void closeEvent(QCloseEvent*)override;
 void dragEnterEvent(QDragEnterEvent*)override;
 void dropEvent(QDropEvent*)override;
private:
 Canvas* canvas;QListWidget* history;QPlainTextEdit* text;QTableWidget* table;QTabWidget* tabs;QPushButton* recognizeButton;QPushButton* tableReference;QPushButton* exportButton;QPushButton* removeButton;QLabel* title;QLabel* engine;QLabel* status;QLabel* zoomLabel;QLabel* dimensions;QSystemTrayIcon* tray;QProcess* process=nullptr;std::unique_ptr<QTemporaryDir> temp;QVector<Hotkey*> hotkeys;QVector<QPointer<PinWindow>> pinWindows;QTimer* saveTimer;std::vector<std::unique_ptr<Capture>> captures;Capture* current=nullptr;bool ocrBusy=false;bool updating=false,capturing=false,quitting=false;int pins=0;QString lastPastedCaptureId;
 void customCapture();void startCaptureMode(bool autoCopy,const QRect& initial=QRect());void addCapture(Capture capture);void select(int row);void refreshHistory();void refreshResult();void configureHotkeys();void pasteClipboard();void togglePins();void restoreClickable();void importImage();void recognize();void exportFile();void settings();void setTheme(const QString& id);void appearance();void sample();void copyText();void pin(const QImage& image,const QRect& geometry=QRect());void syncTable();void removeCurrent();void saveHistory();void persistHistory();void loadHistory();void toast(const QString& message);void toggleVisible();
};
