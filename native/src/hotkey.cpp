#include "hotkey.h"
#include <QCoreApplication>
#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
static OSStatus received(EventHandlerCallRef,EventRef event,void* data){auto hotkey=static_cast<Hotkey*>(data);EventHotKeyID id;GetEventParameter(event,kEventParamDirectObject,typeEventHotKeyID,nullptr,sizeof(id),nullptr,&id);if(id.id!=quint32(hotkey->id))return eventNotHandledErr;QMetaObject::invokeMethod(hotkey,"triggered",Qt::QueuedConnection);return noErr;}
static int macKey(int key){if(key>=Qt::Key_F1&&key<=Qt::Key_F20){const int codes[]={122,120,99,118,96,97,98,100,101,109,103,111,105,107,113,106,64,79,80,90};return codes[key-Qt::Key_F1];}const char* letters="ASDFHGZXCVBQWERTY123465=97-80]OU[IP\\LJK;'.,/NM`";const int codes[]={0,1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,37,38,40,41,39,43,47,44,45,46,50};for(int i=0;letters[i];i++)if(key==letters[i])return codes[i];if(key==Qt::Key_Space)return kVK_Space;return -1;}
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#endif
QString Hotkey::validationError(const QKeySequence& sequence){
 if(sequence.isEmpty())return QStringLiteral("快捷键不能为空。");
 if(sequence.count()!=1)return QStringLiteral("请使用一个按键组合。");
 auto combination=sequence[0];int key=combination.key();bool function=key>=Qt::Key_F1&&key<=Qt::Key_F20;
 bool supported=function||(key>=Qt::Key_A&&key<=Qt::Key_Z)||(key>=Qt::Key_0&&key<=Qt::Key_9)||key==Qt::Key_Space;
 if(!supported)return QStringLiteral("请使用 F1–F20、字母、数字或空格组合。");
 if(!function&&combination.keyboardModifiers()==Qt::NoModifier)return QStringLiteral("字母、数字或空格需要搭配修饰键，避免影响正常输入。");
 return {};
}
Hotkey::Hotkey(const QKeySequence& sequence,QObject* parent):QObject(parent){static int counter=100;id=++counter;if(!validationError(sequence).isEmpty())return;auto combination=sequence[0];int key=combination.key();auto modifiers=combination.keyboardModifiers();
#ifdef Q_OS_MAC
 EventTypeSpec event={kEventClassKeyboard,kEventHotKeyPressed};EventHandlerRef handler=nullptr;InstallApplicationEventHandler(received,1,&event,this,&handler);m_handler=handler;UInt32 flags=0;if(modifiers&Qt::ControlModifier)flags|=cmdKey;if(modifiers&Qt::MetaModifier)flags|=controlKey;if(modifiers&Qt::ShiftModifier)flags|=shiftKey;if(modifiers&Qt::AltModifier)flags|=optionKey;int code=macKey(key);if(code>=0){EventHotKeyRef result=nullptr;EventHotKeyID identifier={0x4D5A534E,quint32(id)};registered=RegisterEventHotKey(code,flags,identifier,GetApplicationEventTarget(),0,&result)==noErr;m_handle=result;}
#endif
#ifdef Q_OS_WIN
 UINT flags=MOD_NOREPEAT;if(modifiers&Qt::ControlModifier)flags|=MOD_CONTROL;if(modifiers&Qt::MetaModifier)flags|=MOD_WIN;if(modifiers&Qt::ShiftModifier)flags|=MOD_SHIFT;if(modifiers&Qt::AltModifier)flags|=MOD_ALT;UINT vk=(key>=Qt::Key_F1&&key<=Qt::Key_F24)?VK_F1+key-Qt::Key_F1:key;registered=RegisterHotKey(nullptr,id,flags,vk);QCoreApplication::instance()->installNativeEventFilter(this);
#endif
}
Hotkey::~Hotkey(){
#ifdef Q_OS_MAC
 if(m_handle)UnregisterEventHotKey(static_cast<EventHotKeyRef>(m_handle));if(m_handler)RemoveEventHandler(static_cast<EventHandlerRef>(m_handler));
#endif
#ifdef Q_OS_WIN
 UnregisterHotKey(nullptr,id);QCoreApplication::instance()->removeNativeEventFilter(this);
#endif
}
bool Hotkey::nativeEventFilter(const QByteArray&,void* message,qintptr*){
#ifdef Q_OS_WIN
 auto msg=static_cast<MSG*>(message);if(msg->message==WM_HOTKEY&&msg->wParam==WPARAM(id)){emit triggered();return true;}
#else
 Q_UNUSED(message)
#endif
 return false;
}
