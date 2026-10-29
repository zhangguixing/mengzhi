# 最终架构决策

2026-10-29，用户要求由 Agent 综合判断，优先轻量和使用体验。最终选择 Qt 6 Widgets + C++17，不使用 Qt WebEngine，不将 React/Tauri 作为最终桌面运行时。

Qt Core/Gui/Widgets/PrintSupport 动态链接。截图覆盖层、绘图、贴图、托盘使用 Qt 原生窗口；macOS Vision/Windows OCR helper 沿用。DOCX/XLSX 用 Open XML + ZIP 实现，PDF 使用 QPdfWriter，无需 Node/Python 或大型 OCR 模型随成品安装。

现有 React 界面仅作为 UI 交互原型，不属于原生安装包。旧 Tauri 代码为早期实验，不继续开发和分发。SYSTEM.md 与 DESIGN.md 中的用户流程、导出能力边界与视觉规范保留；本文覆盖其中旧技术选型。

许可：只选经过核验的 LGPL 兼容模块，动态链接；发布附 Qt 许可、第三方声明、对应源码与构建/替换说明。正式发布前检查实际分发的 Qt 版本与插件，不能仅依赖模块总览。Qt 对应源码需由本项目发布方可控地提供，不能只链接 Qt 官网。

安装包：macOS .app/.dmg；Windows 安装向导 .exe 与便携目录。签名/公证在测试通过后配置，未签名包明确标注。Qt 发布源码材料、发布版本校验与哈希由构建流程准备。
