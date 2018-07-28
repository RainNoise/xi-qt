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

    std::shared_ptr<Theme> theme() const {
        return m_theme;
    }

    std::shared_ptr<StyleMap> styleMap() const {
        return m_styleMap;
    }

private:
    Perference();

    std::shared_ptr<Theme> m_theme;
    std::shared_ptr<StyleMap> m_styleMap;
};

} // namespace xi

#endif // PERFERENCE_H