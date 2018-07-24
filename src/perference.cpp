#include "perference.h"

namespace xi {

Perference::Perference() {
    m_theme = std::make_shared<Theme>();
}

Perference *Perference::shared() {
    static Perference perference;
    return &perference;
}

std::shared_ptr<xi::Theme> Perference::theme()
{
	return m_theme;
}

void Perference::theme(const Theme &theme) {
}

} // namespace xi