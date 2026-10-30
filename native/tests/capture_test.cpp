#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFrame>
#include <QPushButton>
#include <QPainter>
#include "../src/capture_toolbar.h"
#include "../src/capture_clipboard.h"
#include <QMenu>
#include <QClipboard>
#include <QContextMenuEvent>
#include "../src/overlay.h"
#include "../src/canvas.h"

class CaptureTest : public QObject {
 Q_OBJECT
 QTemporaryDir settings;
private slots:
 void initTestCase(){QSettings::setDefaultFormat(QSettings::IniFormat);QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settings.path());QSettings().setValue("reduceMotion",true);}
 void respectsFrontWindowAndDetectsChild(){
  QVector<CaptureTarget> targets={{{10,10,400,300},1,false},{{30,30,100,50},1,true},{{0,0,800,600},2,false},{{20,20,20,20},2,true}};
  QCOMPARE(captureTargetAt(targets,{40,40},false),QRect(10,10,400,300));
  QCOMPARE(captureTargetAt(targets,{40,40},true),QRect(30,30,100,50));
  QCOMPARE(captureTargetAt(targets,{600,400},true),QRect(0,0,800,600));
  QVERIFY(captureTargetAt(targets,{-10,-10},true).isNull());
 }
 void exposesToolsWithoutExtraAnnotationClick(){
  QImage image(800,600,QImage::Format_RGB32);image.fill(QColor("#eef0f3"));
  {QPainter painter(&image);painter.fillRect(QRect(30,30,500,300),Qt::white);painter.fillRect(QRect(30,30,500,38),QColor("#e8edf2"));painter.setPen(QColor("#445464"));painter.drawText(QPoint(48,54),"Capture tools");}
  auto overlay=new Overlay(QGuiApplication::primaryScreen(),image);overlay->resize(800,600);overlay->setInitialSelection(QRect(30,30,500,300));
  auto toolbar=overlay->findChild<QFrame*>("captureToolbar");QVERIFY(toolbar);QVERIFY(toolbar->isVisible());
  QStringList tools;for(auto button:toolbar->findChildren<QAbstractButton*>())if(!button->accessibleName().isEmpty())tools<<button->accessibleName();
  QVERIFY(tools[0].startsWith(QStringLiteral("形状")));QVERIFY(tools[1].startsWith(QStringLiteral("直线")));QVERIFY(tools[2].startsWith(QStringLiteral("箭头")));QVERIFY(tools[3].startsWith(QStringLiteral("画笔")));
  const auto artifactDirectory=QGuiApplication::platformName()=="windows"?qEnvironmentVariable("MENGZHI_CAPTURE_ARTIFACT_DIR"):QString();
  if(!artifactDirectory.isEmpty()){QDir().mkpath(artifactDirectory);overlay->grab().save(artifactDirectory+"/capture-toolbar.png");}
  auto shape=toolbar->findChildren<QAbstractButton*>().first();QTest::mouseClick(shape,Qt::LeftButton);QVERIFY(shape->isChecked());
  auto properties=overlay->findChild<QWidget*>("capturePropertyRail");QVERIFY(properties&&properties->isVisible());
  if(!artifactDirectory.isEmpty()){overlay->grab().save(artifactDirectory+"/capture-toolbar-active.png");toolbar->grab().save(artifactDirectory+"/toolbar-detail.png");}
  QSignalSpy selected(overlay,&Overlay::selected);auto editor=overlay->findChild<Canvas*>();
  QTest::keyClick(editor,Qt::Key_C,Qt::ControlModifier);QCOMPARE(selected.size(),1);
  QCOMPARE(qvariant_cast<QImage>(selected[0][0]).size(),QSize(500,300));QCOMPARE(selected[0][1].toInt(),0);
  QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
 }
 void detectsRealNativeWindow(){
  if(QGuiApplication::platformName()!="windows")QSKIP("Requires native Windows backend");
  QWidget fixture;fixture.setWindowFlags(Qt::Window|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);fixture.setWindowTitle("Mengzhi capture boundary test");
  auto screen=QGuiApplication::primaryScreen();fixture.setGeometry(QRect(screen->availableGeometry().center()-QPoint(160,120),QSize(320,240)));fixture.show();fixture.raise();QVERIFY(QTest::qWaitForWindowExposed(&fixture));
  const auto candidates=captureTargets(screen);QVERIFY2(!candidates.isEmpty(),qPrintable(screen->name()));
  const auto expected=QRect(fixture.mapToGlobal(QPoint())-screen->geometry().topLeft(),fixture.size());
  QCOMPARE(captureTargetAt(candidates,expected.center(),false),expected);
 }
 void doubleClickCopiesAnnotatedSelection_data(){
  QTest::addColumn<bool>("activeTool");QTest::newRow("selection")<<false;QTest::newRow("annotation-tool")<<true;
 }
 void doubleClickCopiesAnnotatedSelection(){
  QFETCH(bool,activeTool);
  QImage image(800,600,QImage::Format_RGB32);image.fill(Qt::white);
  auto overlay=new Overlay(QGuiApplication::primaryScreen(),image);overlay->resize(800,600);overlay->setInitialSelection(QRect(30,30,500,300));
  auto editor=overlay->findChild<Canvas*>();QVERIFY(editor);
  auto toolbar=overlay->findChild<QFrame*>("captureToolbar");QTest::mouseClick(toolbar->findChildren<QAbstractButton*>().first(),Qt::LeftButton);
  QTest::mousePress(editor,Qt::LeftButton,Qt::NoModifier,QPoint(50,50));QTest::mouseMove(editor,QPoint(150,130));QTest::mouseRelease(editor,Qt::LeftButton,Qt::NoModifier,QPoint(150,130));
  const auto annotated=editor->composed();QVERIFY(annotated!=image.copy(QRect(30,30,500,300)));
  if(!activeTool)QTest::mouseClick(editor,Qt::RightButton);
  QSignalSpy selected(overlay,&Overlay::selected);
  if(activeTool)QTest::mouseDClick(editor,Qt::LeftButton,Qt::NoModifier,QPoint(220,160));
  else {QTest::mouseClick(overlay,Qt::LeftButton,Qt::NoModifier,QPoint(250,190));QTest::mouseDClick(overlay,Qt::LeftButton,Qt::NoModifier,QPoint(250,190));}
  QCOMPARE(selected.size(),1);QCOMPARE(selected[0][1].toInt(),0);QCOMPARE(qvariant_cast<QImage>(selected[0][0]),annotated);QVERIFY(!overlay->isVisible());
  QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
 }
 void pinSupportsInPlaceEditingFromContextMenu(){
  QImage source(1000,600,QImage::Format_RGB32);source.fill(Qt::white);
  PinWindow pin(source);pin.resize(500,300);const QRect position=pin.geometry();QSignalSpy workbench(&pin,&PinWindow::editRequested);
  bool entered=false;
  QTimer::singleShot(0,&pin,[&]{auto menu=qobject_cast<QMenu*>(QApplication::activePopupWidget());if(menu){menu->actions().first()->trigger();entered=true;menu->close();}});
  QContextMenuEvent context(QContextMenuEvent::Mouse,QPoint(20,20),pin.mapToGlobal(QPoint(20,20)));QApplication::sendEvent(&pin,&context);
  QVERIFY(entered);QCOMPARE(workbench.size(),0);
  auto editor=pin.findChild<Canvas*>("pinEditor");auto toolbar=pin.findChild<QFrame*>("pinEditToolbar");QVERIFY(editor&&editor->isVisible());QVERIFY(toolbar&&toolbar->isVisible());
  QTest::mousePress(editor,Qt::LeftButton,Qt::NoModifier,QPoint(40,40));QTest::mouseMove(editor,QPoint(160,110));QTest::mouseRelease(editor,Qt::LeftButton,Qt::NoModifier,QPoint(160,110));
  const auto annotated=editor->composed();QCOMPARE(annotated.size(),source.size());QVERIFY(annotated!=source);
  QTest::keyClick(editor,Qt::Key_Z,Qt::ControlModifier);QCOMPARE(editor->composed(),source);
  QTest::keyClick(editor,Qt::Key_Y,Qt::ControlModifier);QCOMPARE(editor->composed(),annotated);
  if(QGuiApplication::platformName()=="windows"&&!qEnvironmentVariable("MENGZHI_CAPTURE_ARTIFACT_DIR").isEmpty()){const auto directory=qEnvironmentVariable("MENGZHI_CAPTURE_ARTIFACT_DIR");pin.grab().save(directory+"/pin-editor.png");toolbar->grab().save(directory+"/pin-editor-toolbar.png");}
  QTest::keyClick(editor,Qt::Key_Space);QVERIFY(pin.isVisible());QVERIFY(!toolbar->isVisible());QCOMPARE(pin.geometry(),position);
  QTest::keyClick(&pin,Qt::Key_C,Qt::ControlModifier);QCOMPARE(QApplication::clipboard()->image(),annotated);
  QTest::keyClick(&pin,Qt::Key_Space);QVERIFY(editor->isVisible());
  QTest::mousePress(editor,Qt::LeftButton,Qt::NoModifier,QPoint(200,160));QTest::mouseMove(editor,QPoint(300,230));QTest::mouseRelease(editor,Qt::LeftButton,Qt::NoModifier,QPoint(300,230));
  QVERIFY(editor->composed()!=annotated);QTest::keyClick(editor,Qt::Key_Escape);QVERIFY(pin.isVisible());QVERIFY(!toolbar->isVisible());
  QTest::keyClick(&pin,Qt::Key_C,Qt::ControlModifier);QCOMPARE(QApplication::clipboard()->image(),annotated);
  QTest::keyClick(&pin,Qt::Key_Space);pin.hide();QVERIFY(!toolbar->isVisible());
 }
 void pinKeepsCapturePositionAndShadowOutOfClipboard(){
  QImage source(400,200,QImage::Format_RGB32);source.fill(QColor("#fafafa"));
  const auto screen=QGuiApplication::primaryScreen();const QRect placement(screen->availableGeometry().topLeft()+QPoint(100,110),QSize(200,100));
  PinWindow pin(source,placement);QCOMPARE(pin.imageGeometry(),placement);QVERIFY(pin.geometry().contains(placement));QVERIFY(pin.geometry().size()!=placement.size());
  QTest::keyClick(&pin,Qt::Key_C,Qt::ControlModifier);QCOMPARE(QApplication::clipboard()->image(),source);
  const auto mime=QApplication::clipboard()->mimeData();QCOMPARE(captureClipboardGeometry(mime),placement);QVERIFY(!captureClipboardId(mime).isEmpty());
  if(QGuiApplication::platformName()=="windows"&&!qEnvironmentVariable("MENGZHI_CAPTURE_ARTIFACT_DIR").isEmpty())pin.grab().save(qEnvironmentVariable("MENGZHI_CAPTURE_ARTIFACT_DIR")+"/pin-shadow.png");
  pin.hide();
 }
};
QTEST_MAIN(CaptureTest)
#include "capture_test.moc"
