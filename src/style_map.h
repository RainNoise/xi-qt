#ifndef STYLEMAP_H
#define STYLEMAP_H

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QVector>

#include <memory>

#include "range.h"
#include "style_span.h"
#include "text_line.h"

namespace xi {

class StyleMap {
public:
    StyleMap() {
    }

    // init builder var
    void applyStyles(const std::shared_ptr<TextLineBuilder> &builder,
                     std::shared_ptr<QVector<StyleSpan>> styles,
                     const QColor &selColor, const QColor &highlightColor) {
    }
};

} // namespace xi

#endif // STYLEMAP_H
