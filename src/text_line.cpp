#include "text_line.h"

namespace xi {

TextLine::TextLine(const QString &text, const std::shared_ptr<Font> &font) {
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
	auto idx = -1;
	for (auto i = 0; i < text.size(); ++i) {
		width = m_fontMetrics->width(text.mid(0, i));
		if (width >= x - m_fontMetrics->averageCharWidth()/2) {
			idx = i;
			break;
		}
	}
	if (idx == 0) return 0;
	if (idx == -1) return m_text.toUtf8().length();

	auto utf8Idx = text.mid(0, idx).toUtf8().length();
	return utf8Idx;

	//auto line = m_layout->lineAt(0);
	//auto utf8Idx = m_text.mid(0, line.xToCursor(x)).toUtf8().length();
	//return utf8Idx;
}

qreal TextLine::indexTox(int ix) {
    if (ix < 0 || ix > m_text.size()) { return 0; }
    auto u8str = m_text.toUtf8();
    return m_fontMetrics->width(u8str.left(ix));
}

std::shared_ptr<xi::TextLine> TextLineBuilder::build()
{
	auto textline = std::make_shared<TextLine>(m_text, m_font);
	int leading = textline->metrics()->leading();
	auto lineWidth = textline->metrics()->width(m_text); // slow
	//auto lineWidth = 10000;
	qreal height = 0;
	QVector<QTextLayout::FormatRange> overrides;
	QTextLayout::FormatRange  fmt;
	QTextCharFormat cfmt;
	cfmt.setFont(m_font->getFont());
	cfmt.setForeground(QColor::fromRgb(248, 248, 186)); // 255, 215, 0 // 255, 128, 0 // 189, 183, 107 // 248, 248, 186
	fmt.start = 0;
	fmt.length = m_text.length();
	fmt.format = cfmt;
	overrides.push_back(fmt);
	textline->layout()->setFormats(overrides);
	textline->layout()->setCacheEnabled(true);
	textline->layout()->beginLayout();
	while (1) {
		QTextLine qline = textline->layout()->createLine();
		if (!qline.isValid())
			break;

		qline.setLineWidth(lineWidth);
		height += leading;
		qline.setPosition(QPointF(0, height));
		height += qline.height();
	}
	textline->layout()->endLayout();

	return textline;
}

void Painter::drawLineBg(QPainter &painter, const std::shared_ptr<TextLine> &line, qreal x, const RangeF &y)
{
	auto selRanges = line->selRanges();
	foreach(const SelRange selRange, *selRanges) {
		QRect rect;
		selRange.range;
		painter.fillRect(rect, selRange.color);
	}
}

} // namespace xi
