#include "material.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
bool systemReducesTransparency(){return false;}
void applyNativeMaterial(QWidget* widget,bool enabled){
 // Public DWM system backdrop on Windows 11; older systems retain the opaque fallback.
 const int backdrop=enabled?2:1;const int corner=2;
 DwmSetWindowAttribute(reinterpret_cast<HWND>(widget->winId()),38,&backdrop,sizeof(backdrop));
 DwmSetWindowAttribute(reinterpret_cast<HWND>(widget->winId()),33,&corner,sizeof(corner));
}
#else
bool systemReducesTransparency(){return false;}
void applyNativeMaterial(QWidget*,bool){}
#endif
