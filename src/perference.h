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

    std::shared_ptr<Theme> theme();
    void theme(const Theme &theme);

private:
    Perference();
    std::shared_ptr<Theme> m_theme;
};

} // namespace xi

#endif // PERFERENCE_H