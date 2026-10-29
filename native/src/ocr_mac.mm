#include "ocr.h"
#include <QJsonArray>
#include <QElapsedTimer>
#import <Vision/Vision.h>
#import <CoreGraphics/CoreGraphics.h>

Ocr::Result Ocr::recognize(const QImage& source){
 @autoreleasepool {
  QElapsedTimer elapsed;elapsed.start();
  if(source.isNull())return {{},QStringLiteral("图片为空，请重新导入。")};
  // Vision reads the original pixels directly; no resizing, PNG encode or disk I/O.
  QImage pixels=source.convertToFormat(QImage::Format_RGBA8888);
  CFDataRef bytes=CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,pixels.constBits(),pixels.sizeInBytes(),kCFAllocatorNull);
  CGDataProviderRef provider=CGDataProviderCreateWithCFData(bytes);
  CGColorSpaceRef space=CGColorSpaceCreateDeviceRGB();
  CGImageRef image=CGImageCreate(pixels.width(),pixels.height(),8,32,pixels.bytesPerLine(),space,kCGImageAlphaLast,provider,nullptr,false,kCGRenderingIntentDefault);
  CGColorSpaceRelease(space);CGDataProviderRelease(provider);CFRelease(bytes);
  if(!image)return {{},QStringLiteral("无法读取图片像素。")};
  VNRecognizeTextRequest* request=[VNRecognizeTextRequest new];
  request.recognitionLevel=VNRequestTextRecognitionLevelAccurate;
  request.recognitionLanguages=@[@"zh-Hans",@"en-US"];
  request.usesLanguageCorrection=YES;
  VNImageRequestHandler* handler=[[VNImageRequestHandler alloc] initWithCGImage:image options:@{}];
  NSError* error=nil;BOOL success=[handler performRequests:@[request] error:&error];CGImageRelease(image);
  if(!success)return {{},QString::fromUtf8(error.localizedDescription.UTF8String)};
  QJsonArray blocks;QStringList text;
  for(VNRecognizedTextObservation* observation in request.results){
   VNRecognizedText* candidate=[observation topCandidates:1].firstObject;if(!candidate)continue;
   CGRect b=observation.boundingBox;QString value=QString::fromUtf8(candidate.string.UTF8String);
   blocks.append(QJsonObject{{"text",value},{"x",b.origin.x},{"y",1-CGRectGetMaxY(b)},{"width",b.size.width},{"height",b.size.height},{"confidence",candidate.confidence}});text<<value;
  }
  return {QJsonObject{{"blocks",blocks},{"text",text.join('\n')},{"engine","Apple Vision"},{"elapsedMs",elapsed.elapsed()}},{}};
 }
}
