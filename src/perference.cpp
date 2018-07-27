#include "perference.h"

namespace xi {

Perference::Perference() {
}

Perference *Perference::shared() {
    static Perference perference;
    return &perference;
}

Theme Perference::theme() const {
    return m_theme;
    //return m_themes.get(m_themeName);
}

//void Perference::setTheme(const QString &name) {
//    m_themeName = name;
//}
//
//void Perference::setTheme(const QString &name, const Theme &theme) {
//    m_themes.append(name, theme);
//    setTheme(name);
//}

void Perference::setTheme(const Theme &theme) {
    m_theme = theme;
}

} // namespace xi