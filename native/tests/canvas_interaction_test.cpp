#include <QtTest>
#include <QSettings>
#include <QSignalSpy>
#include <QWheelEvent>
#include <cmath>
#include "../src/canvas.h"

class CanvasInteractionTest : public QObject {
 Q_OBJECT
 QVariant originalMotion;
 bool hadMotionSetting=false;
 QTemporaryDir settingsDirectory;
 static Capture sample(){Capture capture;capture.image=QImage(1000,700,QImage::Format_ARGB32_Premultiplied);capture.image.fill(QColor("#edf1ee"));return capture;}
 static bool close(const QPointF& a,const QPointF& b){return QLineF(a,b).length()<.00001;}
 static void middleDrag(Canvas& canvas,const QPointF& from,const QPointF& to){
  QMouseEvent press(QEvent::MouseButtonPress,from,from,Qt::MiddleButton,Qt::MiddleButton,Qt::NoModifier);
  QApplication::sendEvent(&canvas,&press);
  QMouseEvent move(QEvent::MouseMove,to,to,Qt::NoButton,Qt::MiddleButton,Qt::NoModifier);
  QApplication::sendEvent(&canvas,&move);
  QMouseEvent release(QEvent::MouseButtonRelease,to,to,Qt::MiddleButton,Qt::NoButton,Qt::NoModifier);
  QApplication::sendEvent(&canvas,&release);
 }
private slots:
 void initTestCase(){QCoreApplication::setOrganizationName("MengzhiTests");QSettings::setDefaultFormat(QSettings::IniFormat);QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settingsDirectory.path());QSettings settings;hadMotionSetting=settings.contains("reduceMotion");originalMotion=settings.value("reduceMotion");}
 void init(){QSettings().setValue("reduceMotion",false);}
 void cleanupTestCase(){QSettings settings;if(hadMotionSetting)settings.setValue("reduceMotion",originalMotion);else settings.remove("reduceMotion");}
 void anchoredZoomKeepsSourcePixel(){
  auto capture=sample();Canvas canvas;canvas.resize(900,700);canvas.setCapture(&capture);
  const QPointF anchor(350,280);const auto source=canvas.imagePointAt(anchor);
  QSignalSpy zoomed(&canvas,&Canvas::zoomChanged);
  canvas.setZoomAt(2.75,anchor);
  QVERIFY(close(canvas.imagePointAt(anchor),source));
  canvas.setZoomAt(.7,anchor);
  QVERIFY(close(canvas.imagePointAt(anchor),source));
  QCOMPARE(zoomed.count(),2);
 }
 void fitResetsPanAndZoomAndBounds(){
  auto capture=sample();Canvas canvas;canvas.resize(900,700);canvas.setCapture(&capture);
  const QPointF probe(310,250);const auto fittedSource=canvas.imagePointAt(probe);
  canvas.setZoomAt(2.5,probe);middleDrag(canvas,QPointF(450,350),QPointF(510,395));
  QVERIFY(!close(canvas.imagePointAt(probe),fittedSource));
  canvas.fitToWindow();QCOMPARE(canvas.zoom(),1.0);
  QVERIFY(close(canvas.imagePointAt(probe),fittedSource));
  canvas.setZoom(100);QCOMPARE(canvas.zoom(),8.0);
  canvas.setZoom(.01);QCOMPARE(canvas.zoom(),.25);
 }
 void middlePanPreservesDrawingTool(){
  auto capture=sample();Canvas canvas;canvas.resize(900,700);canvas.setCapture(&capture);canvas.setTool(1);
  const QPointF from(400,300),offset(50,35);const auto source=canvas.imagePointAt(from);
  middleDrag(canvas,from,from+offset);
  QVERIFY(close(canvas.imagePointAt(from+offset),source));
  QCOMPARE(canvas.tool(),1);QVERIFY(capture.marks.isEmpty());QVERIFY(!canvas.isDrawing());
  QTest::mousePress(&canvas,Qt::LeftButton,Qt::NoModifier,QPoint(420,310));
  QMouseEvent move(QEvent::MouseMove,QPointF(470,360),QPointF(470,360),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
  QApplication::sendEvent(&canvas,&move);
  QTest::mouseRelease(&canvas,Qt::LeftButton,Qt::NoModifier,QPoint(470,360));
  QCOMPARE(capture.marks.size(),1);QCOMPARE(capture.marks.first().tool,1);
 }
 void rapidWheelAccumulatesAndAnchors(){
  auto capture=sample();Canvas canvas;canvas.resize(900,700);canvas.setCapture(&capture);
  const QPointF anchor(350,280);const auto source=canvas.imagePointAt(anchor);
  for(int i=0;i<3;++i){QWheelEvent wheel(anchor,anchor,QPoint(),QPoint(0,120),Qt::NoButton,Qt::ControlModifier,Qt::NoScrollPhase,false);QApplication::sendEvent(&canvas,&wheel);}
  const qreal expected=std::pow(1.15,3);
  QTRY_VERIFY_WITH_TIMEOUT(qAbs(canvas.zoom()-expected)<.00001,500);
  QVERIFY(close(canvas.imagePointAt(anchor),source));
 }
 void reducedMotionIsImmediateAndFitCancelsAnimation(){
  auto capture=sample();Canvas canvas;canvas.resize(900,700);canvas.setCapture(&capture);
  const QPointF anchor(350,280);const auto source=canvas.imagePointAt(anchor);
  QSettings().setValue("reduceMotion",true);canvas.setZoomAt(2.5,anchor,true);
  QCOMPARE(canvas.zoom(),2.5);QVERIFY(close(canvas.imagePointAt(anchor),source));
  QSettings().setValue("reduceMotion",false);canvas.setZoomAt(4,anchor,true);canvas.fitToWindow();
  QTest::qWait(180);QCOMPARE(canvas.zoom(),1.0);
  QVERIFY(close(canvas.imagePointAt(QPointF(450,350)),QPointF(500,350)));
 }
 void directModeDoubleClickDoesNotFit(){
  auto capture=sample();Canvas canvas;canvas.resize(900,700);canvas.setCapture(&capture);canvas.setDirectMode(true);canvas.setZoom(2);
  QTest::mouseDClick(&canvas,Qt::LeftButton,Qt::NoModifier,QPoint(400,300));QCOMPARE(canvas.zoom(),2.0);
  canvas.setDirectMode(false);QTest::mouseDClick(&canvas,Qt::LeftButton,Qt::NoModifier,QPoint(400,300));QCOMPARE(canvas.zoom(),1.0);
 }
};

QTEST_MAIN(CanvasInteractionTest)
#include "canvas_interaction_test.moc"
