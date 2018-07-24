#ifndef TEXTLINE_H
#define TEXTLINE_H

#include <QFontMetrics>
#include <QFontMetricsF>
#include <QObject>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextLayout>

#include <memory>

#include "font.h"
#include "range.h"
#include "style_span.h"

namespace xi {

struct SelRange {
    QColor color;
    RangeI range;
};

// Render info, line
class TextLine {
    friend class TextLineBuilder;

public:
    explicit TextLine(const QString &text, const std::shared_ptr<Font> &font);

    int xToIndex(qreal x);
    qreal indexTox(int ix);

    qreal width() const {
        return metrics()->width(m_text);
    }
    std::shared_ptr<QTextLayout> layout() const {
        return m_layout;
    }
    std::shared_ptr<QFontMetricsF> metrics() const {
        return m_fontMetrics;
    }
    std::shared_ptr<QVector<SelRange>> selRanges() const {
        return m_selRanges;
    }

protected:
    std::shared_ptr<Font> m_font;
    QString m_text;

    std::shared_ptr<QVector<StyleSpan>> m_styles;
    std::shared_ptr<QFontMetricsF> m_fontMetrics;
    std::shared_ptr<QTextLayout> m_layout;
    std::shared_ptr<QVector<SelRange>> m_selRanges;
};

class TextLineBuilder {
public:
    TextLineBuilder(const QString &text, const std::shared_ptr<Font> &font) {
        m_text = text;
        m_font = font;
    }

    void setFgColor(const QColor &color) {
        m_fgColor = color;
    }

    // init QTextLayout
    std::shared_ptr<TextLine> build();

private:
    std::shared_ptr<Font> m_font;
    QString m_text;
    QColor m_fgColor;
};

class Painter {
public:
    static void drawLineBg(QPainter &painter, const std::shared_ptr<TextLine> &line, qreal x, const RangeF &y);

    static void drawLine(QPainter &painter, const std::shared_ptr<TextLine> &line, qreal x, qreal y) {
        line->layout()->draw(&painter, QPoint(x, y));
    }

    static void drawLineDecorations(QPainter &painter, const std::shared_ptr<TextLine> &line, qreal x, qreal y) {
    }

    static void drawCursor(QPainter &painter, qreal x, qreal y, qreal width, qreal height, const QColor &fg) {
        painter.fillRect(x, y, width, height, fg);
    }
};

} // namespace xi

#endif // TEXTLINE_H
