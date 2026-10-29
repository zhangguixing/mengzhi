import AppKit
import Foundation

// Rebuild all 萌知 platform icons without third-party dependencies.
// Run: swift scripts/build_branding.swift assets/branding
let output = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "assets/branding"
let outURL = URL(fileURLWithPath: output, isDirectory: true)
try FileManager.default.createDirectory(at: outURL, withIntermediateDirectories: true)

func color(_ hex: String, _ alpha: CGFloat = 1) -> CGColor {
    let n = UInt32(hex.replacingOccurrences(of: "#", with: ""), radix: 16)!
    return CGColor(srgbRed: CGFloat((n >> 16) & 255) / 255, green: CGFloat((n >> 8) & 255) / 255, blue: CGFloat(n & 255) / 255, alpha: alpha)
}
func path(_ build: (CGMutablePath) -> Void) -> CGPath { let p = CGMutablePath(); build(p); return p }
let leftLeaf = path { p in
    p.move(to: CGPoint(x: 507, y: 560))
    p.addCurve(to: CGPoint(x: 347, y: 398), control1: CGPoint(x: 398, y: 560), control2: CGPoint(x: 347, y: 487))
    p.addCurve(to: CGPoint(x: 507, y: 560), control1: CGPoint(x: 447, y: 398), control2: CGPoint(x: 514, y: 449))
    p.closeSubpath()
}
let rightLeaf = path { p in
    p.move(to: CGPoint(x: 507, y: 488))
    p.addCurve(to: CGPoint(x: 687, y: 320), control1: CGPoint(x: 505, y: 378), control2: CGPoint(x: 587, y: 320))
    p.addCurve(to: CGPoint(x: 507, y: 488), control1: CGPoint(x: 687, y: 426), control2: CGPoint(x: 613, y: 488))
    p.closeSubpath()
}
let stem = path { p in
    p.move(to: CGPoint(x: 507, y: 681))
    p.addLine(to: CGPoint(x: 507, y: 516))
    p.addCurve(to: CGPoint(x: 547, y: 426), control1: CGPoint(x: 507, y: 478), control2: CGPoint(x: 523, y: 449))
}
let frame = path { p in
    p.move(to: CGPoint(x: 393, y: 255)); p.addLine(to: CGPoint(x: 289, y: 255)); p.addQuadCurve(to: CGPoint(x: 255, y: 289), control: CGPoint(x: 255, y: 255)); p.addLine(to: CGPoint(x: 255, y: 393))
    p.move(to: CGPoint(x: 631, y: 255)); p.addLine(to: CGPoint(x: 735, y: 255)); p.addQuadCurve(to: CGPoint(x: 769, y: 289), control: CGPoint(x: 769, y: 255)); p.addLine(to: CGPoint(x: 769, y: 393))
    p.move(to: CGPoint(x: 769, y: 631)); p.addLine(to: CGPoint(x: 769, y: 735)); p.addQuadCurve(to: CGPoint(x: 735, y: 769), control: CGPoint(x: 769, y: 769)); p.addLine(to: CGPoint(x: 631, y: 769))
    p.move(to: CGPoint(x: 393, y: 769)); p.addLine(to: CGPoint(x: 289, y: 769)); p.addQuadCurve(to: CGPoint(x: 255, y: 735), control: CGPoint(x: 255, y: 769)); p.addLine(to: CGPoint(x: 255, y: 631))
}
let space = CGColorSpace(name: CGColorSpace.sRGB)!
func gradient(_ colors: [CGColor], _ positions: [CGFloat]) -> CGGradient {
    return CGGradient(colorsSpace: space, colors: colors as CFArray, locations: positions)!
}
func drawMark(_ ctx: CGContext, monochrome: Bool = false) {
    ctx.saveGState()
    ctx.setStrokeColor(color("F4FBFF")); ctx.setLineWidth(43); ctx.setLineCap(.round); ctx.setLineJoin(.round)
    ctx.addPath(frame); ctx.strokePath()
    ctx.setStrokeColor(color(monochrome ? "F4FBFF" : "9DF3DE")); ctx.setLineWidth(38)
    ctx.addPath(stem); ctx.strokePath()
    ctx.addPath(leftLeaf); ctx.clip()
    ctx.drawLinearGradient(gradient([color(monochrome ? "F4FBFF" : "62DDC3"), color(monochrome ? "F4FBFF" : "B1F6DB")], [0,1]), start: CGPoint(x: 351,y: 401), end: CGPoint(x: 515,y: 555), options: [])
    ctx.restoreGState()
    ctx.saveGState(); ctx.addPath(rightLeaf); ctx.clip()
    ctx.drawLinearGradient(gradient([color(monochrome ? "F4FBFF" : "C7FFE6"), color(monochrome ? "F4FBFF" : "63DFC7")], [0,1]), start: CGPoint(x: 681,y: 322), end: CGPoint(x: 509,y: 488), options: [])
    ctx.restoreGState()
}
func render(_ size: Int, markOnly: Bool = false) -> Data {
    // Supersampling keeps the capture corners crisp in tray/taskbar sizes.
    let pixels = max(size, 1024)
    let ctx = CGContext(data: nil, width: pixels, height: pixels, bitsPerComponent: 8, bytesPerRow: 0, space: space, bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue)!
    ctx.translateBy(x: 0,y: CGFloat(pixels)); ctx.scaleBy(x: CGFloat(pixels)/1024,y: -CGFloat(pixels)/1024)
    if !markOnly {
        let bounds = CGRect(x: 64,y: 64,width: 896,height: 896)
        let shape = CGPath(roundedRect: bounds, cornerWidth: 202,cornerHeight: 202,transform: nil)
        ctx.saveGState(); ctx.addPath(shape); ctx.clip()
        ctx.drawLinearGradient(gradient([color("163C55"),color("0E2A42"),color("091A2C")],[0,0.48,1]), start: CGPoint(x: 140,y: 90),end: CGPoint(x: 870,y: 940),options: [])
        ctx.drawRadialGradient(gradient([color("3EE7C8",0.21),color("3EE7C8",0)],[0,1]),startCenter: CGPoint(x: 300,y: 170),startRadius: 0,endCenter: CGPoint(x: 300,y: 170),endRadius: 660,options: [])
        ctx.restoreGState()
        ctx.addPath(CGPath(roundedRect: bounds.insetBy(dx: 2,dy: 2),cornerWidth: 200,cornerHeight: 200,transform: nil))
        ctx.setStrokeColor(color("CEF7FC",0.17));ctx.setLineWidth(3);ctx.strokePath()
    }
    drawMark(ctx,monochrome: markOnly)
    let full = ctx.makeImage()!
    let final: CGImage
    if size == pixels { final = full }
    else {
        let small = CGContext(data: nil,width: size,height: size,bitsPerComponent: 8,bytesPerRow: 0,space: space,bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue)!
        small.interpolationQuality = .high;small.draw(full,in: CGRect(x: 0,y: 0,width: size,height: size));final = small.makeImage()!
    }
    return NSBitmapImageRep(cgImage: final).representation(using: .png, properties: [:])!
}
let svg = """
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1024 1024" fill="none">
  <title>萌知 · Capture a new idea</title>
  <desc>A fresh sprout inside four screenshot corners, on a midnight teal app tile.</desc>
  <defs>
    <linearGradient id="tile" x1="140" y1="90" x2="870" y2="940" gradientUnits="userSpaceOnUse"><stop stop-color="#163C55"/><stop offset=".48" stop-color="#0E2A42"/><stop offset="1" stop-color="#091A2C"/></linearGradient>
    <radialGradient id="light" cx="300" cy="170" r="660" gradientUnits="userSpaceOnUse"><stop stop-color="#3EE7C8" stop-opacity=".21"/><stop offset="1" stop-color="#3EE7C8" stop-opacity="0"/></radialGradient>
    <linearGradient id="left" x1="351" y1="401" x2="515" y2="555" gradientUnits="userSpaceOnUse"><stop stop-color="#62DDC3"/><stop offset="1" stop-color="#B1F6DB"/></linearGradient>
    <linearGradient id="right" x1="681" y1="322" x2="509" y2="488" gradientUnits="userSpaceOnUse"><stop stop-color="#C7FFE6"/><stop offset="1" stop-color="#63DFC7"/></linearGradient>
  </defs>
  <rect x="64" y="64" width="896" height="896" rx="202" fill="url(#tile)"/>
  <rect x="64" y="64" width="896" height="896" rx="202" fill="url(#light)"/>
  <rect x="66" y="66" width="892" height="892" rx="200" stroke="#CEF7FC" stroke-opacity=".17" stroke-width="3"/>
  <path d="M393 255H289Q255 255 255 289V393 M631 255H735Q769 255 769 289V393 M769 631V735Q769 769 735 769H631 M393 769H289Q255 769 255 735V631" stroke="#F4FBFF" stroke-width="43" stroke-linecap="round" stroke-linejoin="round"/>
  <path d="M507 681V516C507 478 523 449 547 426" stroke="#9DF3DE" stroke-width="38" stroke-linecap="round"/>
  <path d="M507 560C398 560 347 487 347 398C447 398 514 449 507 560Z" fill="url(#left)"/>
  <path d="M507 488C505 378 587 320 687 320C687 426 613 488 507 488Z" fill="url(#right)"/>
</svg>
"""
try svg.write(to: outURL.appendingPathComponent("mengzhi.svg"),atomically: true,encoding: .utf8)
try render(1024).write(to: outURL.appendingPathComponent("mengzhi-1024.png"))
try render(256).write(to: outURL.appendingPathComponent("mengzhi-preview.png"))
try render(1024,markOnly: true).write(to: outURL.appendingPathComponent("mengzhi-mark.png"))
let iconset = outURL.appendingPathComponent("Mengzhi.iconset",isDirectory: true)
try FileManager.default.createDirectory(at: iconset,withIntermediateDirectories: true)
for size in [16,32,128,256,512] {
    try render(size).write(to: iconset.appendingPathComponent("icon_\(size)x\(size).png"))
    try render(size*2).write(to: iconset.appendingPathComponent("icon_\(size)x\(size)@2x.png"))
}
// PNG-backed ICO entries are supported by modern Windows, including 256 px.
var ico = Data([0,0,1,0,6,0])
let sizes = [16,24,32,48,64,256]
let images = sizes.map { render($0) }
var offset = 6 + sizes.count*16
func u16(_ n: Int) -> Data { Data([UInt8(n&255),UInt8((n>>8)&255)]) }
func u32(_ n: Int) -> Data { Data([UInt8(n&255),UInt8((n>>8)&255),UInt8((n>>16)&255),UInt8((n>>24)&255)]) }
for (i,size) in sizes.enumerated() {
    ico.append(contentsOf: [UInt8(size == 256 ? 0 : size),UInt8(size == 256 ? 0 : size),0,0])
    ico.append(u16(1));ico.append(u16(32));ico.append(u32(images[i].count));ico.append(u32(offset));offset += images[i].count
}
images.forEach { ico.append($0) }
try ico.write(to: outURL.appendingPathComponent("mengzhi.ico"))
let proc = Process();proc.executableURL=URL(fileURLWithPath:"/usr/bin/iconutil");proc.arguments=["-c","icns",iconset.path,"-o",outURL.appendingPathComponent("mengzhi.icns").path];try proc.run();proc.waitUntilExit()
if proc.terminationStatus != 0 { fatalError("iconutil could not create ICNS") }
print("Created editable SVG, PNG previews, ICNS, ICO and iconset at \(outURL.path)")
