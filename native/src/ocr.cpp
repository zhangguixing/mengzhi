#include "ocr.h"
#include <QTemporaryDir>
#include <QProcess>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QElapsedTimer>
Ocr::Result Ocr::recognize(const QImage& image){
 QElapsedTimer elapsed;elapsed.start();QTemporaryDir temporary;
 auto path=temporary.filePath("ocr.bmp");
 if(!temporary.isValid()||!image.save(path,"BMP"))return {{},QStringLiteral("无法准备识别图片，请检查磁盘空间。")};
 QProcess process;
#ifdef Q_OS_WIN
 process.start("powershell.exe",{"-NoLogo","-NoProfile","-NonInteractive","-ExecutionPolicy","Bypass","-File",QCoreApplication::applicationDirPath()+"/windows.ps1","-Mode","ocr","-Path",path});
#else
 return {{},QStringLiteral("当前系统未提供本机识别引擎。")};
#endif
 if(!process.waitForStarted())return {{},QStringLiteral("无法启动系统识别组件。")};
 if(!process.waitForFinished(90000)){process.kill();process.waitForFinished();return {{},QStringLiteral("识别超时，请缩小图片后重试。")};}
 auto document=QJsonDocument::fromJson(process.readAllStandardOutput());
 if(process.exitCode()!=0||!document.isObject())return {{},QString::fromUtf8(process.readAllStandardError()).trimmed().isEmpty()?QStringLiteral("系统识别失败，请检查语言包。"):QString::fromUtf8(process.readAllStandardError())};
 auto data=document.object();data["elapsedMs"]=elapsed.elapsed();return {data,{}};
}
