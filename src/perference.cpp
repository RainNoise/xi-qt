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
}

void Perference::theme(const Theme &theme) {
    m_theme = theme;
}

} // namespace xi