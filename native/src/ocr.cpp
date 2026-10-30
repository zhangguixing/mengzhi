#include "ocr.h"
#include <QTemporaryDir>
#include <QProcess>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QElapsedTimer>
#include <QPainter>
#include <array>
#include <cmath>

QImage Ocr::prepareForRecognition(const QImage& source){
 if(source.isNull())return {};
 QImage image(source.size(),QImage::Format_RGB32);image.fill(Qt::white);
 {QPainter painter(&image);painter.drawImage(0,0,source);}
 std::array<qint64,256> histogram{};
 for(int y=0;y<image.height();++y){const auto row=reinterpret_cast<const QRgb*>(image.constScanLine(y));for(int x=0;x<image.width();++x)++histogram[qGray(row[x])];}
 const qint64 count=qint64(image.width())*image.height(),tail=qMax<qint64>(1,count/1000);
 qint64 sum=0;int low=0,high=255,background=0;
 for(int i=0;i<256;++i){if(histogram[i]>histogram[background])background=i;}
 for(;low<255;++low){sum+=histogram[low];if(sum>=tail)break;}
 sum=0;for(;high>low;--high){sum+=histogram[high];if(sum>=tail)break;}
 // Avoid amplifying near-uniform noise; retain antialiasing instead of binarizing.
 const bool stretch=high-low>=16;
 for(int y=0;y<image.height();++y){auto row=reinterpret_cast<QRgb*>(image.scanLine(y));for(int x=0;x<image.width();++x){int gray=qGray(row[x]);if(stretch)gray=qBound(0,(gray-low)*255/(high-low),255);if(background<128)gray=255-gray;row[x]=qRgb(gray,gray,gray);}}
 const int edge=qMax(image.width(),image.height());
 const double desired=edge<=1000?3.0:2.0;
 const double scale=qMin(desired,qMin(10000.0/edge,std::sqrt(12000000.0/count)));
 if(scale>1.05)image=image.scaled(qRound(image.width()*scale),qRound(image.height()*scale),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
 return image;
}

Ocr::Result Ocr::recognize(const QImage& image){
 QElapsedTimer elapsed;elapsed.start();QTemporaryDir temporary;
 auto path=temporary.filePath("ocr.bmp");
 const auto prepared=prepareForRecognition(image);
 if(!temporary.isValid()||!prepared.save(path,"BMP"))return {{},QStringLiteral("无法准备识别图片，请检查磁盘空间。")};
 QProcess process;
#ifdef Q_OS_WIN
 process.start("powershell.exe",{"-NoLogo","-NoProfile","-NonInteractive","-ExecutionPolicy","Bypass","-File",QCoreApplication::applicationDirPath()+"/windows.ps1","-Mode","ocr","-Path",path});
#else
 return {{},QStringLiteral("当前系统未提供本机识别引擎。")};
#endif
 if(!process.waitForStarted())return {{},QStringLiteral("无法启动系统识别组件。")};
 if(!process.waitForFinished(90000)){process.kill();process.waitForFinished();return {{},QStringLiteral("识别超时，请缩小图片后重试。")};}
 const auto output=process.readAllStandardOutput();
 const auto error=QString::fromUtf8(process.readAllStandardError()).trimmed();
 auto document=QJsonDocument::fromJson(output);
 if(process.exitStatus()!=QProcess::NormalExit||process.exitCode()!=0||!document.isObject())
  return {{},error.isEmpty()?QStringLiteral("系统识别失败，请检查语言包。"):error};
 if(!document.object().value("text").isString()||!document.object().value("blocks").isArray())
  return {{},QStringLiteral("系统识别组件返回的数据不完整，请重试。")};
 auto data=document.object();data["rawText"]=data["text"];data["text"]=layoutText(data["blocks"].toArray());data["elapsedMs"]=elapsed.elapsed();data["inputWidth"]=image.width();data["inputHeight"]=image.height();data["processedWidth"]=prepared.width();data["processedHeight"]=prepared.height();return {data,{}};
}
