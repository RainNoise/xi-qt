#ifndef STYLE_H
#define STYLE_H

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QVector>

#include <memory>

#include "font.h"

namespace xi {

int utf8OffsetToUtf16(const QString &text, int ix);

class Style {
    friend class StyleMapState;

public:
    //Style(const std::shared_ptr<Font> &font, const QColor &fgColor, 
    //    const QColor &bgColor, bool underline, bool italic, int weight) : 
    //    m_font(font), m_fgColor(fgColor), m_bgColor(bgColor), m_underline(underline),
    //    m_italic(italic), m_weight(weight) {
    //}
    Style(const QColor &fgColor,
          const QColor &bgColor, bool underline, bool italic, int weight) : 
        m_fgColor(fgColor), m_bgColor(bgColor), m_underline(underline),                                                                            
        m_italic(italic), m_weight(weight) {
        m_info.italic = italic;
        m_info.underline = underline;
        m_info.weight = weight;
    }

private:
    // std::shared_ptr<Font> m_font;
    QColor m_fgColor;
    QColor m_bgColor;
    FontInfo m_info;
    bool m_underline;
    bool m_italic;
    int m_weight;
    bool m_fakeItalic = false;

};

} // namespace xi

#endif // STYLE_H
