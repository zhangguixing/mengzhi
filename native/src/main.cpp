#include "window.h"
#include "ui_study.h"
#include "ocr.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QElapsedTimer>
#include <cstdio>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QApplication>
#include <QTimer>
#include <QIcon>
#include <QSettings>
#include <QCommandLineParser>
int main(int argc,char**argv){QApplication app(argc,argv);app.setWindowIcon(QIcon(":/branding/mengzhi-1024.png"));QCoreApplication::setApplicationName("Mengzhi");QCoreApplication::setOrganizationName("Mengzhi");QCoreApplication::setApplicationVersion("0.1.0");app.setQuitOnLastWindowClosed(false);QCommandLineParser parser;parser.addHelpOption();parser.addVersionOption();parser.addOption({"preview", "Render deterministic UI and export samples, then exit.","directory"});parser.addOption({"ocr-benchmark","Measure native OCR on an image.","image"});parser.addOption({"ui-study","Render three native design studies.","directory"});parser.addOption({"ui-variant","Run a design study: pearl, graphite or folio.","variant"});parser.addPositionalArgument("image","Image to open.");parser.process(app);if(parser.isSet("ui-study")){QSettings::setDefaultFormat(QSettings::IniFormat);QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,parser.value("ui-study")+"/settings");QSettings().setValue("history",false);QSettings().setValue("reduceMotion",true);QTimer::singleShot(0,&app,[&]{app.exit(renderUiStudies(parser.value("ui-study"))?0:1);});return app.exec();}if(parser.isSet("preview")){QSettings::setDefaultFormat(QSettings::IniFormat);QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,parser.value("preview")+"/settings");QSettings().setValue("history",false);}if(parser.isSet("ocr-benchmark")){auto watcher=new QFutureWatcher<QJsonArray>(&app);QObject::connect(watcher,&QFutureWatcher<QJsonArray>::finished,&app,[&]{auto json=QJsonDocument(watcher->result()).toJson(QJsonDocument::Compact);std::fwrite(json.constData(),1,json.size(),stdout);app.quit();});QString path=parser.value("ocr-benchmark");watcher->setFuture(QtConcurrent::run([path]{QJsonArray runs;QImage image(path);for(int i=0;i<3;i++){QElapsedTimer t;t.start();auto result=Ocr::recognize(image);auto data=result.data;data["wallMs"]=t.elapsed();data["error"]=result.error;runs.append(data);}return runs;}));return app.exec();}Window window;if(parser.isSet("ui-variant"))applyUiStudy(&window,parser.value("ui-variant"));window.show();if(!parser.positionalArguments().isEmpty())window.openImage(parser.positionalArguments().first());if(parser.isSet("preview")){QTimer::singleShot(600,&window,[&]{window.renderPreview(parser.value("preview"));app.quit();});}return app.exec();}
