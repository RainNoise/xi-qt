#ifndef STYLE_SPAN_H
#define STYLE_SPAN_H

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QVector>

#include <memory>

#include "range.h"

namespace xi {

class StyleSpan {
public:
    friend class TextLine;

    using StyleIdentifier = int;

    StyleSpan();
    StyleSpan(StyleIdentifier style, RangeI range);

    static std::shared_ptr<QVector<StyleSpan>> styles(const QJsonArray &object, const QString &text);

    StyleIdentifier style() const {
        return m_style;
    }
    RangeI range() const {
        return m_range;
    }

private:
    StyleIdentifier m_style;
    RangeI m_range;
};

} // namespace xi

#endif // STYLE_SPAN_H
