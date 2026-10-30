#include "capture_targets.h"
#include <QScreen>
#include <QGuiApplication>
#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
namespace {
struct Monitor {QString name;QRect logical;bool primary=false;RECT bounds{};bool found=false;};
BOOL CALLBACK findMonitor(HMONITOR monitor,HDC,LPRECT,LPARAM data){
 auto& target=*reinterpret_cast<Monitor*>(data);MONITORINFOEXW info{};info.cbSize=sizeof(info);
 if(!GetMonitorInfoW(monitor,reinterpret_cast<MONITORINFO*>(&info)))return TRUE;
 // Qt 6 may expose the monitor's friendly name rather than \\.\DISPLAYn.
 // On Windows Qt preserves physical screen origins while scaling their sizes.
 const bool sameOrigin=info.rcMonitor.left==target.logical.left()&&info.rcMonitor.top==target.logical.top();
 if(QString::fromWCharArray(info.szDevice).endsWith(target.name,Qt::CaseInsensitive)
    ||(target.primary&&(info.dwFlags&MONITORINFOF_PRIMARY))||(!target.primary&&sameOrigin)){
  target.bounds=info.rcMonitor;target.found=true;return FALSE;
 }
 return TRUE;
}
struct Context {QVector<CaptureTarget> targets;QRect screen;RECT physical;int root=0;int children=0;};
QRect logicalRect(const RECT& bounds,const Context& context){
 const double sx=context.screen.width()/double(context.physical.right-context.physical.left);
 const double sy=context.screen.height()/double(context.physical.bottom-context.physical.top);
 const int x=qRound((bounds.left-context.physical.left)*sx),y=qRound((bounds.top-context.physical.top)*sy);
 const int right=qRound((bounds.right-context.physical.left)*sx),bottom=qRound((bounds.bottom-context.physical.top)*sy);
 return QRect(x,y,right-x,bottom-y).intersected(QRect(QPoint(),context.screen.size()));
}
BOOL CALLBACK childWindow(HWND window,LPARAM data){
 auto& context=*reinterpret_cast<Context*>(data);if(context.children++>400)return FALSE;
 if(!IsWindowVisible(window))return TRUE;RECT bounds{};if(!GetWindowRect(window,&bounds))return TRUE;
 auto rect=logicalRect(bounds,context);if(rect.width()>=8&&rect.height()>=8)context.targets.append({rect,context.root,true});return TRUE;
}
BOOL CALLBACK topWindow(HWND window,LPARAM data){
 if(!IsWindowVisible(window)||IsIconic(window))return TRUE;
 BOOL cloaked=FALSE;DwmGetWindowAttribute(window,DWMWA_CLOAKED,&cloaked,sizeof(cloaked));if(cloaked)return TRUE;
 if(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TRANSPARENT)return TRUE;
 auto& context=*reinterpret_cast<Context*>(data);RECT bounds{};
 if(FAILED(DwmGetWindowAttribute(window,DWMWA_EXTENDED_FRAME_BOUNDS,&bounds,sizeof(bounds)))&&!GetWindowRect(window,&bounds))return TRUE;
 const auto rect=logicalRect(bounds,context);if(rect.width()<8||rect.height()<8)return TRUE;
 ++context.root;context.targets.append({rect,context.root,false});context.children=0;
 EnumChildWindows(window,childWindow,data);return TRUE;
}
}
#endif
QVector<CaptureTarget> captureTargets(QScreen* screen){
#ifdef Q_OS_WIN
 if(!screen)return {};Monitor monitor;monitor.name=screen->name();monitor.logical=screen->geometry();monitor.primary=screen==QGuiApplication::primaryScreen();EnumDisplayMonitors(nullptr,nullptr,findMonitor,reinterpret_cast<LPARAM>(&monitor));
 if(!monitor.found)return {};
 Context context;context.screen=screen->geometry();context.physical=monitor.bounds;
 EnumWindows(topWindow,reinterpret_cast<LPARAM>(&context));return context.targets;
#else
 Q_UNUSED(screen);return {};
#endif
}
QRect captureTargetAt(const QVector<CaptureTarget>& targets,const QPoint& point,bool elements){
 for(const auto& window:targets){
  if(window.element||!window.bounds.contains(point))continue;
  QRect result=window.bounds;
  if(elements)for(const auto& child:targets)if(child.element&&child.root==window.root&&child.bounds.contains(point)
     &&qint64(child.bounds.width())*child.bounds.height()<qint64(result.width())*result.height())result=child.bounds;
  return result;
 }
 return {};
}
