#include "perference.h"
#include "text_line.h"

namespace xi {

TextLine::TextLine(const QString &text, const std::shared_ptr<Font> &font) {
    m_text = text;
    m_font = font;
    m_fontMetrics = std::make_unique<QFontMetricsF>(font->getFont());
    m_layout = std::make_shared<QTextLayout>(text, font->getFont());
    m_selRanges = std::make_shared<QVector<SelRange>>();
}

int TextLine::xToIndex(qreal x) {
    //QString text = m_text;
    //auto width = 0.f;
    //auto idx = -1;
    //for (auto i = 0; i < text.size(); ++i) {
    //    width = m_fontMetrics->width(text.mid(0, i));
    //    if (width >= x - m_fontMetrics->averageCharWidth() / 2) {
    //        idx = i;
    //        break;
    //    }
    //}
    //if (idx == 0) return 0;
    //if (idx == -1) return m_text.toUtf8().length();

    //auto utf8Ix = text.mid(0, idx).toUtf8().length();
    //return utf8Ix;

    // TODO: IMPROVE
    if (m_layout->lineCount() > 0) {
        auto innerLine = m_layout->lineAt(0);
        auto idx = innerLine.xToCursor(x);
        auto utf8Ix = m_text.mid(0, idx).toUtf8().length();
        return utf8Ix;
    }
    return 0;
}

qreal TextLine::indexTox(int ix) {
    //if (ix < 0 || ix > m_text.size()) { return 0; }
    //return m_fontMetrics->width(m_text.toUtf8().left(ix));

    // TODO: IMPROVE
    if (m_layout->lineCount() > 0) {
        auto innerLine = m_layout->lineAt(0);
        auto utf8ix = m_text.toUtf8().mid(0, ix).length();
        auto x = innerLine.cursorToX(utf8ix);
        return x;
    }
    return 0;
}

std::shared_ptr<xi::TextLine> TextLineBuilder::build() {
    //auto theme = Perference::shared()->theme();
    auto textline = std::make_shared<TextLine>(m_text, m_font);
    int leading = textline->metrics()->leading();
    auto lineWidth = textline->metrics()->width(m_text); // slow

    foreach (std::shared_ptr<FontSpan> span, m_fontSpans) {
        QTextLayout::FormatRange fmt;
        QTextCharFormat cfmt;
        cfmt.setFont(m_font->getFont());
        cfmt.setFontItalic(span->payload.italic);
        cfmt.setFontWeight(span->payload.weight);
        fmt.start = span->range.start();
        fmt.length = span->range.length();
        fmt.format = cfmt;
        m_overrides.push_back(fmt);
    }
    
    foreach(std::shared_ptr<ColorSpan> span, m_fgSpans) {
        QTextLayout::FormatRange fmt;
        QTextCharFormat cfmt;
        //cfmt.setFont(m_font->getFont());
        if (span->payload.isValid()) {
            cfmt.setForeground(span->payload);
        }
        fmt.start = span->range.start();
        fmt.length = span->range.length();
        fmt.format = cfmt;
        m_overrides.push_back(fmt);
    }

    foreach (std::shared_ptr<ColorSpan> span, m_selSpans) {
        QTextLayout::FormatRange fmt;
        QTextCharFormat cfmt;
        //cfmt.setFont(m_font->getFont());
        cfmt.setBackground(span->payload);
        fmt.start = span->range.start();
        fmt.length = span->range.length();
        fmt.format = cfmt;
        m_overrides.push_back(fmt);
    }

    //foreach (std::shared_ptr<UnderlineSpan> span, m_underlineSpans) {
    //    QTextLayout::FormatRange fmt;
    //    QTextCharFormat cfmt;
    //    cfmt.setFont(m_font->getFont());
    //    cfmt.setFontUnderline(true);
    //    cfmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    //    fmt.start = span->range.start();
    //    fmt.length = span->range.length();
    //    fmt.format = cfmt;
    //    m_overrides.push_back(fmt);
    //}

    qreal height = 0;
    textline->layout()->setFormats(m_overrides);
    textline->layout()->setCacheEnabled(true);
    textline->layout()->beginLayout();
    while (1) {
        QTextLine qline = textline->layout()->createLine();
        if (!qline.isValid())
            break;
        qline.setLineWidth(lineWidth*2);
        height += leading;
        qline.setPosition(QPointF(0, height));
        height += qline.height();
    }
    textline->layout()->endLayout();

    return textline;
}

void Painter::drawLineBg(QPainter &painter, const std::shared_ptr<TextLine> &line, qreal x, const RangeF &y) {
    auto selRanges = line->selRanges();
    foreach (const SelRange selRange, *selRanges) {
        QRect rect;
        selRange.range;
        painter.fillRect(rect, selRange.color);
    }
}

} // namespace xi
