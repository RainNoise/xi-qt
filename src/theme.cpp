#include "theme.h"

#include <QPalette>

#include <iterator>


namespace xi {

const char* names[] = {
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

/*
return Theme(foreground: NSColor.black,
background: NSColor.white,
caret: NSColor.black,
lineHighlight: nil,
findHighlight: NSColor(deviceWhite: 0.8, alpha: 0.4),
findHighlightForeground: nil,
gutter: NSColor(deviceWhite: 0.9, alpha: 1.0),
gutterForeground: NSColor(deviceWhite: 0.5, alpha: 1.0),
selection: NSColor.selectedTextBackgroundColor,
selectionForeground: NSColor.selectedTextColor,
selectionBorder: nil,
inactiveSelection: NSColor(red: 0.8, green: 0.8, blue: 0.8, alpha: 1.0),
inactiveSelectionForeground: NSColor.selectedTextColor,
shadow: nil
)
*/
ThemeInfo defaultThemeInfo() {
	ThemeInfo info;
    info.foreground(Qt::black);
	info.background(Qt::white);
    info.caret(Qt::black);
    info.find_highlight();
	return info;
}

Theme Theme::defaultTheme() {
	return Theme(defaultThemeInfo());
}

QColor to_color(const QJsonObject& json) {
	auto a = json["a"].toInt();
	auto b = json["b"].toInt();
	auto g = json["g"].toInt();
	auto r = json["r"].toInt();
	return QColor(r, g, b, a);
}

Theme::Theme(const QJsonObject& json) {
	ThemeInfo info;
	for (auto i = 0; i < std::size(names); ++i) {
		QString name = names[i];
		auto& element = info.m_elements[name];
		if (!json.contains(name)) {
			element.type = ThemeElement::Null;
			continue;
		}
		auto value = json[name];
		if (value.isNull()) {
			element.type = ThemeElement::Null;
		} else if (value.isArray()) {
			element.type = ThemeElement::Color;
			element.color = to_color(json[names[i]].toObject());
		} else if (value.isString()) {
			element.type = ThemeElement::String;
			element.str = value.toString();
		}
	}
	m_info = info;
	// m_info.merge(defaultThemeInfo());
}

Theme::Theme(const ThemeInfo& info) {
	m_info = info;
}

Theme::Theme() {
	m_info = defaultTheme().m_info;
}

Theme& Theme::operator=(const Theme& theme) {
	m_info = theme.m_info;
	return *this;
}

ThemeInfo& ThemeInfo::operator=(const ThemeInfo& info) {
	m_elements = info.m_elements;
	return *this;
}

void ThemeInfo::merge(const ThemeInfo& info) {
	for (auto i = 0; i < std::size(names); ++i) {
		auto& element = m_elements[names[i]];
		if (element.type == ThemeElement::Null) {
			element = info.m_elements[names[i]];
		}
	}
}

} // xi
