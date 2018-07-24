#include "style_span.h"

namespace xi {

StyleSpan::StyleSpan(StyleIdentifier style, RangeI range) : m_style(style), m_range(range) {
}

StyleSpan::StyleSpan() : m_style(-1) {
}

} // namespace xi