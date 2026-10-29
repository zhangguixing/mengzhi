#include <QtTest>
#include "../src/export.h"
#include <QTemporaryDir>
#include <QFile>
class ExportTest:public QObject {
Q_OBJECT
private slots:
 void preservesCells(){auto x=Export::excel({{"编号","备注"},{"001","=SUM(A1:A3)"},{"002","<&\""}});QVERIFY(x.startsWith("PK\x03\x04"));QVERIFY(x.contains("001"));QVERIFY(x.contains("t=\"inlineStr\""));QVERIFY(!x.contains("<f>"));QVERIFY(x.contains("&lt;&amp;&quot;"));}
 void wordIncludesEditableTextAndTable(){Capture d;d.text=QStringLiteral("中文段落 & <示例>");d.table={{"设备","数量"},{"显示器","2"}};auto x=Export::word(d);QVERIFY(x.startsWith("PK"));QVERIFY(x.contains(QStringLiteral("中文段落").toUtf8()));QVERIFY(x.contains("<w:tbl>"));QVERIFY(x.contains("&amp; &lt;"));}
 void alignsSparseGeometry(){QVector<OcrBlock> b={{"数量",QRectF(.5,.1,.1,.03),-1},{"设备",QRectF(.1,.1,.1,.03),-1},{"显示器",QRectF(.1,.2,.1,.03),-1},{"2",QRectF(.5,.21,.1,.03),-1},{"合计",QRectF(.1,.3,.1,.03),-1}};auto t=Export::inferTable(b);QCOMPARE(t.size(),3);QCOMPARE(t[0],QStringList({"设备","数量"}));QCOMPARE(t[1],QStringList({"显示器","2"}));QCOMPARE(t[2],QStringList({"合计",""}));}
 void writesRealPdf(){QTemporaryDir dir;QImage image(200,100,QImage::Format_RGB32);image.fill(Qt::red);QString error;auto path=dir.filePath("image.pdf");QVERIFY(Export::pdf(path,image,&error));QFile file(path);QVERIFY(file.open(QIODevice::ReadOnly));auto data=file.readAll();QVERIFY(data.startsWith("%PDF"));QVERIFY(data.contains("/Type /Page"));QVERIFY(data.size()>500);}
 void rejectsEmptyPdf(){QString error;QVERIFY(!Export::pdf("unused.pdf",QImage(),&error));QVERIFY(!error.isEmpty());}
};
QTEST_MAIN(ExportTest)
#include "export_test.moc"
