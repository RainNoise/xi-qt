#ifndef PERFERENCE_H
#define PERFERENCE_H

#include <memory>

#include "theme.h"

namespace xi {

/// xi::Perference::shared()->theme();
/// xi::Perference::shared()->keymap();
/// xi::Perference::shared()->setting();
class Perference {
public:
    static Perference *shared();

    Theme theme() const;
    void theme(const Theme &theme);

    //Keymap keymap() const;
    //void keymap(const Keymap &keymap);

    //Settings settings() const;
    //void settings(const Settings &settings);

private:
    Perference();
    Theme m_theme;
};

} // namespace xi

#endif // PERFERENCE_H