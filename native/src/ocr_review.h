#pragma once
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>

// Flag ambiguous tokens for review; never silently rewrite IDs or source text.
class OcrReviewHighlighter : public QSyntaxHighlighter {
public:
 explicit OcrReviewHighlighter(QTextDocument* document):QSyntaxHighlighter(document){}
protected:
 void highlightBlock(const QString& text) override {
  static const QRegularExpression token("[A-Za-z0-9_|]+");
  static const QRegularExpression numeric("^[0-9iIl|]+$");
  static const QRegularExpression ambiguous("[iIl|]");
  static const QRegularExpression digit("[0-9]");
  QTextCharFormat format;
  format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
  format.setUnderlineColor(QColor("#b7791f"));
  format.setToolTip(QStringLiteral("此处可能混淆 1 / i / I / l，请对照原图确认；原文未自动替换。"));
  auto matches=token.globalMatch(text);
  while(matches.hasNext()){
   const auto match=matches.next();const auto value=match.captured();
   if(numeric.match(value).hasMatch()&&ambiguous.match(value).hasMatch()
      &&(value.size()==1||digit.match(value).hasMatch()))setFormat(match.capturedStart(),match.capturedLength(),format);
  }
 }
};
