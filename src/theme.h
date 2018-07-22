#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QJsonObject>
#include <QHash>
#include <QString>


namespace xi {

struct ThemeElement {
    QColor color;
    QString str;
	enum Type {
		Color,
		String,
		Null,
	};
	Type type;
	ThemeElement() {
		type = Null;
	}
    ~ThemeElement() {

    }
    ThemeElement(const QColor& color) {
        type = Color;
        this->color = color;
    }
    ThemeElement(Qt::GlobalColor gcolor) {
        ThemeElement(QColor(gcolor));
    }
    ThemeElement(const ThemeElement& ele) {
        *this = ele;
    }
	ThemeElement& operator=(const ThemeElement& ele) {
		type = ele.type;
		if (type == Color) {
			color = ele.color;
		} else if (type == String) {
			str = ele.str;
		}
		return *this;
	}
};

#define THEME_ELEMENT_METHOD(TypeName) \
  void TypeName(const ThemeElement& element) { m_elements[#TypeName] = element; } \
  ThemeElement TypeName() { return m_elements[#TypeName]; }

class ThemeInfo {
	friend class Theme;
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

	ThemeInfo& operator=(const ThemeInfo& info);

protected:
	void merge(const ThemeInfo& info);

	QHash<QString, ThemeElement> m_elements;
};

class Theme {
public:
	static Theme defaultTheme();
	
	Theme();
	explicit Theme(const ThemeInfo& info);
	explicit Theme(const QJsonObject& json);

	ThemeInfo info() const {
		return m_info;
	}

	Theme& operator=(const Theme& theme);

private:
	ThemeInfo m_info;
};

} // xi

#endif // THEME_H

