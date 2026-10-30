#include "ocr.h"
#include <QRectF>
#include <QVector>
#include <algorithm>
#include <cmath>

namespace {
int columns(const QString& text){
 int width=0;
 for(const auto c:text){const auto u=c.unicode();width+=(u>=0x2e80&&u<=0xffef)?2:1;}
 return width;
}
struct Word {QString text;QRectF bounds;};
struct Row {QVector<Word> words;double center,height;};
}

QString Ocr::layoutText(const QJsonArray& blocks){
 QVector<Word> words;QVector<double> pitches;
 double left=1;
 for(const auto& value:blocks){
  const auto b=value.toObject();const auto text=b["text"].toString().trimmed();
  QRectF rect(b["x"].toDouble(),b["y"].toDouble(),b["width"].toDouble(),b["height"].toDouble());
  if(text.isEmpty()||!rect.isValid()||!std::isfinite(rect.x())||!std::isfinite(rect.y()))continue;
  words.append({text,rect});left=qMin(left,rect.x());
  pitches.append(rect.width()/qMax(1,columns(text)));
 }
 if(words.isEmpty())return {};
 std::sort(pitches.begin(),pitches.end());const double pitch=qMax(.0001,pitches[pitches.size()/2]);
 std::sort(words.begin(),words.end(),[](const Word&a,const Word&b){return a.bounds.center().y()<b.bounds.center().y();});
 QVector<Row> rows;
 for(const auto& word:words){
  Row* match=nullptr;
  for(auto& row:rows)if(qAbs(row.center-word.bounds.center().y())<qMax(row.height,word.bounds.height())*.65){match=&row;break;}
  if(match){match->words.append(word);match->height=qMax(match->height,word.bounds.height());}
  else rows.append({{word},word.bounds.center().y(),word.bounds.height()});
 }
 QStringList lines;const Row* previous=nullptr;
 for(auto& row:rows){
  if(previous&&row.center-previous->center>qMax(previous->height,row.height)*1.85)lines.append("");
  std::sort(row.words.begin(),row.words.end(),[](const Word&a,const Word&b){return a.bounds.x()<b.bounds.x();});
  QString line;int used=0;double lastRight=0;
  for(const auto& word:row.words){
   const int desired=qBound(0,qRound((word.bounds.x()-left)/pitch),400);
   int spaces=qMax(0,desired-used);
   if(!line.isEmpty()){
    const double gap=word.bounds.left()-lastRight;
    if(gap<pitch*1.5){
     const bool punctuation=line.back().isPunct()||word.text.front().isPunct();
     const bool cjk=line.back().unicode()>=0x2e80&&word.text.front().unicode()>=0x2e80;
     spaces=!punctuation&&gap>pitch*(cjk?.9:.35)?1:0;
    }
   }
   line+=QString(qMin(spaces,120),' ')+word.text;used+=qMin(spaces,120)+columns(word.text);
   lastRight=word.bounds.right();
  }
  lines.append(line);
  previous=&row;
 }
 return lines.join('\n');
}
