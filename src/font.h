#ifndef XIFONT_H
#define XIFONT_H

#include <QFont>

namespace xi {

/// add baseline
class Font {
public:
    Font() { m_baseLine = 0; }
    Font(const QFont &font) {
        m_font = font;
        m_baseLine = 0;
    }
    inline QFont getFont() const {
        return m_font;
    }
    inline int getBaseLine() const {
        return m_baseLine;
    }

private:
    QFont m_font;
    int m_baseLine;
};

} // namespace xi

#endif // XIFONT_H