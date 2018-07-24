#ifndef THEME_H
#define THEME_H

#include <QBrush>
#include <QColor>
#include <QHash>
#include <QJsonObject>
#include <QString>

namespace xi {

struct ThemeElement {
    enum Type {
        Color,
        Option,
        Null,
    };

    QColor color;
    QString option;
    Type type;

    ThemeElement() {
        type = Null;
    }
    ~ThemeElement() {
    }
    ThemeElement(const QColor &color) {
        type = Color;
        this->color = color;
    }
    ThemeElement(Qt::GlobalColor gcolor) {
        ThemeElement(QColor(gcolor));
    }
    ThemeElement(const ThemeElement &ele) {
        *this = ele;
    }
    ThemeElement &operator=(const ThemeElement &ele) {
        type = ele.type;
        if (type == Color) {
            color = ele.color;
        } else if (type == Option) {
            option = ele.option;
        }
        return *this;
    }
    operator QColor() {
        return color;
    }
    operator QString() {
        return option;
    }
    //operator QBrush() {
    //    return color;
    //}
};

#define THEME_ELEMENT_METHOD(TypeName)                                              \
    void TypeName(const ThemeElement &element) { m_elements[#TypeName] = element; } \
    ThemeElement TypeName() { return m_elements[#TypeName]; }

class Theme {
public:
    static Theme defaultTheme();

    Theme();
    Theme(const Theme &info);
    explicit Theme(const QString &name, const QJsonObject &json);
    Theme &operator=(const Theme &theme);

public:
    THEME_ELEMENT_METHOD(accent);
    THEME_ELEMENT_METHOD(active_guide);
    THEME_ELEMENT_METHOD(background);
    THEME_ELEMENT_METHOD(bracket_contents_foreground);
    THEME_ELEMENT_METHOD(bracket_contents_options);
    THEME_ELEMENT_METHOD(brackets_background);
    THEME_ELEMENT_METHOD(brackets_foreground);
    THEME_ELEMENT_METHOD(brackets_options);
    THEME_ELEMENT_METHOD(caret);
    THEME_ELEMENT_METHOD(find_highlight);
    THEME_ELEMENT_METHOD(find_highlight_foreground);
    THEME_ELEMENT_METHOD(foreground);
    THEME_ELEMENT_METHOD(guide);
    THEME_ELEMENT_METHOD(gutter);
    THEME_ELEMENT_METHOD(gutter_foreground);
    THEME_ELEMENT_METHOD(highlight);
    THEME_ELEMENT_METHOD(highlight_foreground);
    THEME_ELEMENT_METHOD(inactive_selection);
    THEME_ELEMENT_METHOD(inactive_selection_foreground);
    THEME_ELEMENT_METHOD(line_highlight);
    THEME_ELEMENT_METHOD(minimap_border);
    THEME_ELEMENT_METHOD(misspelling);
    THEME_ELEMENT_METHOD(phantom_css);
    THEME_ELEMENT_METHOD(popup_css);
    THEME_ELEMENT_METHOD(selection);
    THEME_ELEMENT_METHOD(selection_background);
    THEME_ELEMENT_METHOD(selection_border);
    THEME_ELEMENT_METHOD(selection_foreground);
    THEME_ELEMENT_METHOD(shadow);
    THEME_ELEMENT_METHOD(stack_guide);
    THEME_ELEMENT_METHOD(tags_foreground);
    THEME_ELEMENT_METHOD(tags_options);

private:
    void merge(const Theme &info);
    QString m_name;
    QHash<QString, ThemeElement> m_elements;
};

} // namespace xi

#endif // THEME_H
