#ifndef PERFERENCE_H
#define PERFERENCE_H

#include <memory>

#include "style_map.h"
#include "theme.h"

namespace xi {

class StyleMap;

/// Share
/// xi::Perference::shared()->theme();
/// xi::Perference::shared()->keymap();
/// xi::Perference::shared()->setting();

//struct Settings {
//    StyleMap styleMap;
//    Font font;
//    Font cjkFont;
//};

class Perference {
public:
    static Perference *shared();

    Theme theme() const;
    void theme(const Theme &theme);

    //void setTheme(const QString &name);
    //void setTheme(const QString &name, const Theme &theme);
    //void appendTheme(const Theme &theme);

    //Keymap keymap() const;
    //void keymap(const Keymap &keymap);

    //Settings settings() const;
    //void settings(const Settings &settings);

    StyleMap styleMap() const {
        return m_styleMap;
    }
    void styleMap(const StyleMap &styleMap) {
        m_styleMap = styleMap;
    }

    // TODO THREAD SAFE, locked
    //std::shared_ptr<StyleMap> styleMap() {
    //    return m_styleMap;
    //}

private:
    Perference();

    Theme m_theme;
    StyleMap m_styleMap;
    //std::shared_ptr<StyleMap> m_styleMap;

    //QString m_themeName;
    //ThemeList m_themes;
};

} // namespace xi

#endif // PERFERENCE_H