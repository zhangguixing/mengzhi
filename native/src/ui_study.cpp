#include "ui_study.h"
#include "window.h"
#include "canvas.h"
#include "capture_icons.h"
#include <QtWidgets>
void installStudioInteractions(QWidget* window);

namespace {
void recolor(QWidget* window){
 const auto ink=QColor(window->property("studyInk").toString());
 const auto accent=QColor(window->property("studyAccent").toString());
 const auto onAccent=QColor(window->property("studyOnAccent").toString());
 static const QHash<QString,int> icons={{"move",0},{"rect",1},{"ellipse",2},{"arrow",3},{"pen",4},{"text",5},{"mosaic",6},{"line",7},{"marker",8},{"blur",9},{"eraser",10},{"undo",11},{"redo",12},{"pin",13},{"copy",15},{"scan",18},{"download",19},{"plus",20},{"minus",21},{"image",22},{"trash",23},{"settings",24},{"focus",26},{"fit",27}};
 for(auto button:window->findChildren<QAbstractButton*>()){
  const auto name=button->property("glyph").toString();if(!icons.contains(name))continue;
  const bool primary=button->property("primary").toBool()||button->objectName()=="recognizeButton";
  button->setIcon(captureIcon(icons.value(name),primary?onAccent:button->isChecked()?accent:ink));
 }
}
}
void applyUiStudy(QWidget* window,const QString& id){
 window->setProperty("uiStudy",id);
 // Remove incidental per-label size overrides so each system owns typography.
 for(auto label:window->findChildren<QLabel*>())label->setStyleSheet(QString());
 if(id=="graphite"){applyUiStudyGraphite(window);installStudioInteractions(window);}
 else if(id=="folio")applyUiStudyFolio(window);
 else applyUiStudyPearl(window);
 const QColor canvasColor(id=="graphite"?"#20242a":id=="folio"?"#EEEDE9":"#EDF1EE");
 for(auto canvas:window->findChildren<Canvas*>()){canvas->setStyleSheet(QString("background:%1;border:0;").arg(canvasColor.name()));auto palette=canvas->palette();palette.setColor(QPalette::Window,canvasColor);canvas->setPalette(palette);}
 recolor(window);
 for(auto button:window->findChildren<QAbstractButton*>())QObject::connect(button,&QAbstractButton::toggled,window,[window]{QTimer::singleShot(0,window,[window]{recolor(window);});});
 window->setWindowTitle(QStringLiteral("萌知 · 图像工作台"));
}
bool renderUiStudies(const QString& directory){
 QDir().mkpath(directory);bool success=true;
 for(const auto& id:{QString("pearl"),QString("graphite"),QString("folio")}){
  Window window;applyUiStudy(&window,id);window.resize(1360,860);window.show();QApplication::processEvents();
  success=window.grab().save(directory+"/"+id+".png")&&success;
  window.resize(1024,720);QApplication::processEvents();success=window.grab().save(directory+"/"+id+"-compact.png")&&success;
  window.hide();
 }
 return success;
}
