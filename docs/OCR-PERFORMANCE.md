# OCR 性能优化

macOS 从「GUI 线程保存 PNG + 每次启动 Swift helper」改为「专用后台线程 + Vision 直接读取原图内存像素」。保持 accurate、zh-Hans/en-US、语言纠错，不缩小原图。启动后在同一串行工作线程上用内置合成文字预热引擎，避免同时触发多个初始化。

Windows 仍使用系统 OCR；图片准备和进程等待改到后台，临时格式使用无需 PNG 压缩的 BMP。Windows 实机耗时尚未测量。

每张 Capture 在本次会话内缓存成功结果，包括空结果。再次点击复用当前校对内容；右键识别按钮可选择「重新识别（覆盖当前结果）」强制重跑。失败不会标记缓存成功。识别完成后显示包含排队、模型调用和结果处理的耗时。

## 本机测量（1040×760 合成设备表，24 块文字）

- 旧 Swift helper：稳定运行约 366–403 ms；测量中也出现 29.4–30.1 s 的初始化等待。
- 新内存路径：一次复测为 375/174/174 ms；早期测试也出现首次约 30.4 s、后续 169/168 ms。
- 两条路径的 24 个文字块内容逐项比较一致。此结果只代表该样图；尚不能推断所有真实图片的准确率或耗时。
- CPU-only 对照仍出现首次约 29.8 s，故没有强制切换计算设备。

结论：去掉重复进程与 PNG 开销、在应用进程内保留已初始化的模型状态，并避免 GUI 阻塞。后台预热减少首次点击等待，但不能承诺消除所有 macOS 冷启动延迟。

可复现命令：

```sh
QT_QPA_PLATFORM=offscreen build/native/Mengzhi.app/Contents/MacOS/Mengzhi --ocr-benchmark artifacts/native/sample.png
```

命令输出三次测量的 JSON、文字块和 wallMs，可与旧 shizi-ocr 输出比较。现有表格推断及 Word/Excel/PDF 导出测试通过。
