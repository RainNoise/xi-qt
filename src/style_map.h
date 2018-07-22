#ifndef STYLEMAP_H
#define STYLEMAP_H

#include <QVector>
#include <QJsonObject>
#include <QString>
#include <QColor>

#include <memory>

#include "range.h"
#include "text_line.h"


namespace xi {

class StyleSpan
{
public:
    friend class TextLine;

	using StyleIdentifier = int;

	StyleSpan();
	StyleSpan(StyleIdentifier style, RangeI range);

	static std::shared_ptr<QVector<StyleSpan>> styles(const QJsonArray& object, const QString& text);

private:
	StyleIdentifier m_style;
	RangeI m_range;
};

class StyleMap
{
public:
	StyleMap() {

	}

	// init builder var
	void applyStyles(const std::shared_ptr<TextLineBuilder>& builder, 
		std::shared_ptr<QVector<StyleSpan>> styles,
		const QColor& selColor, const QColor& highlightColor) {

	}
};

} // xi

#endif // STYLEMAP_H
