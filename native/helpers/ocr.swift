import Foundation
import Vision
import ImageIO

struct Block: Codable { let text: String; let x: Double; let y: Double; let width: Double; let height: Double; let confidence: Double }
struct Result: Codable { let blocks: [Block]; let text: String; let engine: String }
func fail(_ text: String) -> Never { FileHandle.standardError.write(Data(text.utf8)); exit(1) }
guard CommandLine.arguments.count == 2 else { fail("Expected an image path") }
let url = URL(fileURLWithPath: CommandLine.arguments[1])
let request = VNRecognizeTextRequest()
request.recognitionLevel = .accurate
request.recognitionLanguages = ["zh-Hans", "en-US"]
request.usesLanguageCorrection = true
do {
    try VNImageRequestHandler(url: url, options: [:]).perform([request])
    let blocks = (request.results ?? []).compactMap { observation -> Block? in
        guard let candidate = observation.topCandidates(1).first else { return nil }
        let b = observation.boundingBox
        return Block(text: candidate.string, x: b.minX, y: 1 - b.maxY, width: b.width, height: b.height, confidence: Double(candidate.confidence))
    }.sorted { a, b in abs(a.y-b.y) < min(a.height,b.height)*0.5 ? a.x < b.x : a.y < b.y }
    let result = Result(blocks: blocks, text: blocks.map(\.text).joined(separator: "\n"), engine: "Apple Vision")
    FileHandle.standardOutput.write(try JSONEncoder().encode(result))
} catch { fail("系统文字识别失败：\(error.localizedDescription)") }
