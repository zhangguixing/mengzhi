#pragma once
#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QKeySequence>
class Hotkey : public QObject,public QAbstractNativeEventFilter {
 Q_OBJECT
public:explicit Hotkey(const QKeySequence& sequence=QKeySequence("F1"),QObject* parent=nullptr);~Hotkey();bool registered=false;
 bool nativeEventFilter(const QByteArray&,void*,qintptr*) override;
 static QString validationError(const QKeySequence& sequence);
 int id=0;
signals:void triggered();
private:void* m_handle=nullptr;void* m_handler=nullptr;
};
