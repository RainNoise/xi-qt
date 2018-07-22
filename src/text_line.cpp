#include "text_line.h"


namespace xi {

TextLine::TextLine(const QString& text, const std::shared_ptr<Font>& font) {
    m_text = text;
    m_font = font;
    //m_cursor = line.m_cursor;
    //m_styles = line.m_styles;
	m_fontMetrics = std::make_unique<QFontMetricsF>(font->getFont());
	m_layout = std::make_shared<QTextLayout>(text, font->getFont());
	m_selRanges = std::make_shared<QVector<SelRange>>();
}

int TextLine::xToIndex(qreal x) {
	QString text = m_text;
	auto width = 0.f;
	auto idx = 0;
	for (auto i = 0; i < text.size(); ++i) {
		width += m_fontMetrics->width(text.mid(0, i));
		if (width >= x) {
			idx = i;
			break;
		}
	}
	if (idx == 0) return 0;

	auto utf8Idx = text.mid(0, idx).toUtf8().length();
	return utf8Idx;
}

qreal TextLine::indexTox(int ix) {
	if (ix < 0 || ix > m_text.size()) { return 0; }
	auto u8str = m_text.toUtf8();
	return m_fontMetrics->width(u8str.left(ix));
}

} // xi
