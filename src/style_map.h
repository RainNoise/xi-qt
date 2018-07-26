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
    //StyleMap(const std::shared_ptr<Font> &font) {
    //    m_font = font;
    //}
    StyleMap() {
        //m_font = font;
    }

    void defStyle(const QJsonObject& json);

    void applyStyle(const std::shared_ptr<TextLineBuilder> &builder, int id, const RangeI &range, const QColor &selColor) {
        // BUG: m_styles is NULL;
        if (id > m_styles.count()) {
            qWarning() << "stylemap can't resolve" << id;
            return;
        }
        if (id == 0 || id == 1) {
            builder->addSelSpan(range, selColor);
        } else {
            auto style = m_styles[id];
            if (!style) return;

            if (style->m_fgColor.isValid()) {
                builder->addFgSpan(range, style->m_fgColor);
            }
            //if (style->m_font) {
            //    // ?
            //}
            builder->addFontSpan(range, style->m_info);

            if (style->m_fakeItalic) {
                builder->addFakeItalicSpan(range);
            }
            if (style->m_underline) {
                builder->addUnderlineSpan(range, UnderlineStyle::single);
            }
        }        
    }

    void applyStyles(const std::shared_ptr<TextLineBuilder> &builder,
                     std::shared_ptr<QVector<StyleSpan>> styles,
                     const QColor &selColor, const QColor &highlightColor) {
        foreach(StyleSpan ss, *styles) {
            QColor color;
            auto id = ss.style();
            switch (id) {
            case 0:
                color = selColor;
                break;
            case 1:
                color = highlightColor;
                break;
            default:
                color = QColor(QColor::Invalid);
                break;
            }
            applyStyle(builder, id, ss.range(), color);
        }
    }

    StyleMap &operator=(const StyleMap &styleMap) {
        if (this != &styleMap) {
            m_styles = styleMap.m_styles;
        }
        return *this;
    }

private:
    QVector<std::shared_ptr<Style>> m_styles;
    //std::shared_ptr<Font> m_font;
};

} // namespace xi

#endif // STYLEMAP_H
