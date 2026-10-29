#pragma once
#include <QList>
#include <QSettings>
#include <QString>

// finesse · register=product · design-language=apple · apple-mode=app
// Named palettes move the complete surface stack together. Filled controls stay
// graphite with white labels; accent colours belong to focus and capture guides.
namespace Theme {
struct Palette {
    QString id, name, description;
    bool dark;
    QString frame, surface, canvas, ink, muted, line, hover, accent;
    QString button, buttonHover, buttonPressed;
    QString toolTop, toolBottom, workbench, workbenchBorder;
    int radius;
};

inline QList<Palette> all()
{
    static const QList<Palette> palettes = {
        {
            "graphite", QStringLiteral("石墨玻璃"),
            QStringLiteral("通透浅灰 · 轻盈日常"), false,
            "#edf0f5", "#fcfdff", "#f1f4fa", "#23272f",
            "#606b7b", "#dce2ea", "#e8edf4", "#2463b8",
            "#303237", "#41454d", "#24262b",
            "#dde4ef", "#cbd6e5", "#f6f8fc", "#d8e1ed", 22
        },
        {
            "graphite-dark", QStringLiteral("暗夜石墨"),
            QStringLiteral("中性深灰 · 安静专注"), true,
            "#14171c", "#23272f", "#181c23", "#f4f6fa",
            "#aab3c1", "#39414d", "#333c49", "#a8bad2",
            "#555b65", "#626975", "#494f59",
            "#3b424e", "#252b35", "#242b35", "#4a5361", 22
        },
        {
            "orbit", QStringLiteral("深空蓝"),
            QStringLiteral("深蓝舱体 · 精密冷光"), true,
            "#0a1323", "#121f33", "#0b1629", "#e8f1ff",
            "#a4b8d4", "#2b4465", "#1e3552", "#76b9ff",
            "#454d59", "#565f6e", "#353d49",
            "#263d5c", "#14263f", "#14243b", "#365574", 16
        },
        {
            "night", QStringLiteral("夜蓝"),
            QStringLiteral("夜间蓝灰 · 柔和层次"), true,
            "#17212b", "#242f3d", "#0e1621", "#edf3f8",
            "#a6b5c5", "#3b4b5d", "#304255", "#6ab3f3",
            "#3c4e60", "#4c6075", "#2d3e50",
            "#34475a", "#253546", "#1c2936", "#4b6178", 18
        },
        {
            "nebula", QStringLiteral("星云紫"),
            QStringLiteral("深紫层次 · 柔和星光"), true,
            "#1b132a", "#2c213d", "#21192f", "#f4eefd",
            "#c3b5d4", "#4d3b62", "#3d3052", "#c5a6ff",
            "#514b5b", "#625b6f", "#403a4b",
            "#544165", "#332641", "#33263f", "#675279", 20
        },
        {
            "glacier", QStringLiteral("冰川银"),
            QStringLiteral("银白金属 · 冰蓝细节"), false,
            "#e5ebf1", "#f8fbff", "#eaf1f7", "#23323f",
            "#526575", "#c6d5e2", "#d9e6f0", "#27658f",
            "#34414d", "#455563", "#26333f",
            "#dae5ef", "#bdcedd", "#eff5fa", "#c3d3e0", 16
        }
    };
    return palettes;
}

inline Palette get(const QString& id)
{
    const auto palettes = all();
    for (const auto& palette : palettes)
        if (palette.id == (id == "aurora" ? "night" : id))
            return palette;
    return palettes.first();
}

inline Palette current()
{
    QSettings settings;
    // Existing installations retain their light/dark choice until a named theme
    // is selected. Reading a palette does not overwrite application preferences.
    return get(settings.value("theme", settings.value("dark", false).toBool()
                             ? "graphite-dark" : "graphite").toString());
}

// Compatibility for callers that still use the previous two-mode API.
inline QString buttonFill(bool dark) { return dark ? "#555b65" : "#303237"; }
inline QString buttonHover(bool dark) { return dark ? "#626975" : "#41454d"; }
inline QString buttonPressed(bool dark) { return dark ? "#494f59" : "#24262b"; }
}
