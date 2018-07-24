#include "perference.h"

namespace xi {

Perference::Perference() {
    m_theme = std::make_shared<Theme>();
}

Perference *Perference::shared() {
    static Perference perference;
    return &perference;
}

} // namespace xi