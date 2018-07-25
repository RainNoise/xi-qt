#ifndef STYLEMAP_H
#define STYLEMAP_H

#include <QColor>
#include <QDebug>
#include <QJsonObject>
#include <QString>
#include <QVector>

#include <memory>

#include "range.h"
#include "style.h"
#include "style_span.h"
#include "text_line.h"

namespace xi {

QColor colorFromArgb(quint32 argb);

class StyleMap {
public:
    StyleMap() {

    }

    void defStyle(const QJsonObject& json);

    void applyStyle(const std::shared_ptr<TextLineBuilder> &builder, int id, const RangeI &range, const QColor &selColor) {
        if (id > m_styles.count()) {
            qWarning() << "stylemap can't resolve" << id;
            return;
        }
    }

    void applyStyles(const std::shared_ptr<TextLineBuilder> &builder,
                     std::shared_ptr<QVector<StyleSpan>> styles,
                     const QColor &selColor, const QColor &highlightColor) {
    }

private:
    QVector<std::shared_ptr<Style>> m_styles;
};

} // namespace xi

#endif // STYLEMAP_H
