#ifndef STYLE_H
#define STYLE_H

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <QVector>

#include <memory>

#include "font.h"

namespace xi {

class Style {
    friend class StyleMap;

public:
    Style() {
    }

private:
    Font m_font;
    QColor m_fgColor;
    QColor m_bgColor;
    bool m_underline;
    bool m_italic;
    int m_weight;
    bool m_fakeItalic = false;

};

} // namespace xi

#endif // STYLE_H
