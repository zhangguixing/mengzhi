#include <QtTest>
#include <QPainter>
#include <QJsonArray>
#include <QFontDatabase>
#include "../src/ocr_review.h"
#include "../src/ocr.h"

class OcrTest : public QObject {
 Q_OBJECT
private slots:
 void flagsAmbiguousNumericTokensWithoutRewritingIds() {
  QTextDocument document;OcrReviewHighlighter review(&document);
  const QString original="123i5 iPhone1 Intel i I l 1";document.setPlainText(original);review.rehighlight();
  QCOMPARE(document.toPlainText(),original);
  const auto formats=document.firstBlock().layout()->formats();
  QVERIFY(!formats.isEmpty());QCOMPARE(formats.first().start,0);QCOMPARE(formats.first().length,5);
  for(const auto& range:formats)QVERIFY(range.start!=6&&range.start!=14);
 }
 void keepsLinesColumnsAndParagraphs() {
  auto block=[](QString text,double x,double y,double width,double height=.04){return QJsonObject{{"text",text},{"x",x},{"y",y},{"width",width},{"height",height}};};
  const auto text=Ocr::layoutText({block("Name",.1,.1,.08),block("Count",.5,.1,.1),block("Monitor",.1,.16,.14),block("12",.5,.16,.04),block("Total",.2,.4,.1)});
  const auto rows=text.split('\n');QCOMPARE(rows.size(),4);
  QVERIFY(rows[0].indexOf("Count")>10);QCOMPARE(rows[0].indexOf("Count"),rows[1].indexOf("12"));
  QVERIFY(rows[2].isEmpty());QVERIFY(rows[3].startsWith("     "));
 }
 void keepsPunctuationOnItsLine() {
  const QJsonArray words={QJsonObject{{"text","123"},{"x",.1},{"y",.1},{"width",.09},{"height",.08}},QJsonObject{{"text","."},{"x",.19},{"y",.16},{"width",.01},{"height",.02}}};
  QVERIFY(!Ocr::layoutText(words).contains('\n'));
 }
 void preparesDarkSmallTextWithoutChangingSource() {
  QImage image(200,100,QImage::Format_RGB32);image.fill(QColor("#202020"));
  QPainter painter(&image);painter.fillRect(30,20,20,50,QColor("#eeeeee"));painter.end();
  const auto prepared=Ocr::prepareForRecognition(image);
  QCOMPARE(image.pixelColor(0,0),QColor("#202020"));QCOMPARE(prepared.size(),QSize(600,300));
  QVERIFY(qGray(prepared.pixel(0,0))>240);QVERIFY(qGray(prepared.pixel(120,120))<20);
 }
 void rejectsEmptyImage() {
  const auto result = Ocr::recognize(QImage());
  QVERIFY(!result.error.isEmpty());
 }
 void recognizesRealImage() {
  // The offscreen Qt backend does not enumerate Windows system fonts.
  QVERIFY(QFontDatabase::addApplicationFont(qEnvironmentVariable("SystemRoot") + "/Fonts/arial.ttf") >= 0);
  QImage image(900, 160, QImage::Format_RGB32);
  image.fill(Qt::white);
  QPainter painter(&image);
  QFont font("Arial");
  font.setPixelSize(60);
  painter.setFont(font);
  painter.setPen(Qt::black);
  painter.drawText(image.rect(), Qt::AlignCenter, "MENGZHI OCR 12345");
  painter.end();
  const auto result = Ocr::recognize(image);
  QVERIFY2(result.error.isEmpty(), qPrintable(result.error));
  QVERIFY2(result.data.value("text").toString().contains("12345"),
           qPrintable(result.data.value("text").toString()));
  QVERIFY(!result.data.value("blocks").toArray().isEmpty());
  QCOMPARE(result.data.value("engine").toString(), QString("Windows OCR"));
 }
 void preservesHelperError() {
  QImage image(16384, 32, QImage::Format_RGB32);
  image.fill(Qt::white);
  const auto result = Ocr::recognize(image);
  QVERIFY2(result.error.contains(QStringLiteral("图片尺寸超过系统 OCR 限制")),
           qPrintable(result.error));
 }
};
QTEST_MAIN(OcrTest)
#include "ocr_test.moc"
