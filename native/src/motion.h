#pragma once
#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSettings>
#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#endif
inline bool reducedMotion(){
 if(QSettings().value("reduceMotion",false).toBool())return true;
#ifdef Q_OS_MAC
 CFPropertyListRef value=CFPreferencesCopyAppValue(CFSTR("reduceMotion"),CFSTR("com.apple.universalaccess"));bool reduced=value&&CFGetTypeID(value)==CFBooleanGetTypeID()&&CFBooleanGetValue(static_cast<CFBooleanRef>(value));if(value)CFRelease(value);return reduced;
#elif defined(Q_OS_WIN)
 BOOL enabled=TRUE;SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION,0,&enabled,0);return !enabled;
#else
 return false;
#endif
}
inline void reveal(QWidget* widget,int duration=140){
 if(reducedMotion())return;
 auto effect=new QGraphicsOpacityEffect(widget);widget->setGraphicsEffect(effect);auto animation=new QPropertyAnimation(effect,"opacity",effect);animation->setDuration(duration);animation->setStartValue(.15);animation->setEndValue(1.0);animation->setEasingCurve(QEasingCurve::OutCubic);QObject::connect(animation,&QPropertyAnimation::finished,widget,[widget,effect]{if(widget->graphicsEffect()==effect)widget->setGraphicsEffect(nullptr);});animation->start(QAbstractAnimation::DeleteWhenStopped);
}
