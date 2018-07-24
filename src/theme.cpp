#include "theme.h"

#include <QPalette>

#include <iterator>

namespace xi {

const char *names[] = {
    "accent",
    "active_guide",
    "background",
    "bracket_contents_foreground",
    "bracket_contents_options",
    "brackets_background",
    "brackets_foreground",
    "brackets_options",
    "caret",
    "find_highlight",
    "find_highlight_foreground",
    "foreground",
    "guide",
    "gutter",
    "gutter_foreground",
    "highlight",
    "highlight_foreground",
    "inactive_selection",
    "inactive_selection_foreground",
    "line_highlight",
    "minimap_border",
    "misspelling",
    "phantom_css",
    "popup_css",
    "selection",
    "selection_background",
    "selection_border",
    "selection_foreground",
    "shadow",
    "stack_guide",
    "tags_foreground",
    "tags_options",
};

Theme Theme::defaultTheme() {
    Theme theme;
    theme.foreground(QColor::fromRgb(255, 215, 0));
    theme.background(Qt::black);
    theme.caret(QColor::fromRgb(220, 220, 220));
    return theme;
}

QColor to_color(const QJsonObject &json) {
    auto a = json["a"].toInt();
    auto b = json["b"].toInt();
    auto g = json["g"].toInt();
    auto r = json["r"].toInt();
    return QColor(r, g, b, a);
}

Theme::Theme(const QString &name, const QJsonObject &json) {
    m_name = name;
    for (auto i = 0; i < std::size(names); ++i) {
        QString name = names[i];
        auto &element = m_elements[name];
        if (!json.contains(name)) {
            element.type = ThemeElement::Null;
            continue;
        }
        auto value = json[name];
        auto type = value.type();
        if (value.isNull()) {
            element.type = ThemeElement::Null;
        }else if (value.isString()) {
            element.type = ThemeElement::Option;
            element.option = value.toString();
        } else {
            element.type = ThemeElement::Color;
            element.color = to_color(json[names[i]].toObject());
        } 
    }
    //merge(defaultTheme());
}

Theme::Theme(const Theme &info) {
    *this = info;
}

Theme::Theme() {
}

Theme &Theme::operator=(const Theme &theme) {
    if (this != &theme) {
        m_name = theme.m_name;
        m_elements = theme.m_elements;
    }
    return *this;
}

void Theme::merge(const Theme &info) {
    for (auto i = 0; i < std::size(names); ++i) {
        auto &element = m_elements[names[i]];
        if (element.type == ThemeElement::Null) {
            element = info.m_elements[names[i]];
        }
    }
}

} // namespace xi
