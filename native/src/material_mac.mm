#include "material.h"
#include <QGuiApplication>
#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>
#import <objc/runtime.h>
static char materialKey;
bool systemReducesTransparency(){return NSWorkspace.sharedWorkspace.accessibilityDisplayShouldReduceTransparency;}
void applyNativeMaterial(QWidget* widget,bool enabled){
 if(QGuiApplication::platformName()!=QStringLiteral("cocoa"))return;
 NSView* view=(__bridge NSView*)(reinterpret_cast<void*>(widget->winId()));NSWindow* window=view.window;if(!window)return;
 NSView* previous=(NSView*)objc_getAssociatedObject(window,&materialKey);
 if(previous){[previous removeFromSuperview];objc_setAssociatedObject(window,&materialKey,nil,OBJC_ASSOCIATION_RETAIN_NONATOMIC);}
 if(!enabled||systemReducesTransparency()){window.opaque=YES;window.backgroundColor=NSColor.windowBackgroundColor;return;}
 window.opaque=NO;window.backgroundColor=NSColor.clearColor;
 window.titlebarAppearsTransparent=YES;window.hasShadow=YES;
 NSView* glass;
 if(@available(macOS 26.0,*)){
   NSGlassEffectView* effect=[[NSGlassEffectView alloc] initWithFrame:view.frame];effect.style=NSGlassEffectViewStyleRegular;effect.cornerRadius=22;
   glass=effect;
 }else{
   NSVisualEffectView* effect=[[NSVisualEffectView alloc] initWithFrame:view.frame];effect.material=NSVisualEffectMaterialSidebar;effect.blendingMode=NSVisualEffectBlendingModeBehindWindow;effect.state=NSVisualEffectStateActive;
   effect.wantsLayer=YES;effect.layer.cornerRadius=20;effect.layer.masksToBounds=YES;glass=effect;
 }
 glass.autoresizingMask=NSViewWidthSizable|NSViewHeightSizable;
 [view.superview addSubview:glass positioned:NSWindowBelow relativeTo:view];
 objc_setAssociatedObject(window,&materialKey,glass,OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}
