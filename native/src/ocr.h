#pragma once
#include <QImage>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QThreadPool>
namespace Ocr {
inline QThreadPool* pool(){static QThreadPool worker;static const bool initialized=[] {worker.setMaxThreadCount(1);worker.setExpiryTimeout(-1);return true;}();Q_UNUSED(initialized);return &worker;}
struct Result { QJsonObject data; QString error; };
QString layoutText(const QJsonArray& blocks);
#ifdef Q_OS_WIN
// Prepares a separate OCR image; capture pixels and normalized coordinates stay intact.
QImage prepareForRecognition(const QImage& image);
#endif
// Blocking engine call. Invoke on a worker thread, never the GUI thread.
Result recognize(const QImage& image);
}
