#include "content_view.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include "perference.h"

namespace xi {

ContentView::ContentView(const std::shared_ptr<File> &file, const std::shared_ptr<CoreConnection> &connection, QWidget *parent) : QWidget(parent) {

    //setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_KeyCompression, false);
    setAttribute(Qt::WA_KeyboardFocusChange);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);

    m_connection = connection;
    m_file = file;
    m_dataSource = std::make_shared<EditViewDataSource>();
    {
        QString family = "Inconsolata";
        int size = 14; // 1920x1080
        int weight = 0;
        bool italic = false;
        QFont font(family, size, weight, italic);
        font.setStyleHint(QFont::TypeWriter, QFont::StyleStrategy(QFont::PreferDefault | QFont::ForceIntegerMetrics));
        font.setFixedPitch(true);
        font.setKerning(false);
        m_dataSource->defaultFont = std::make_shared<Font>(font);
    }
    m_dataSource->fontMetrics = std::make_shared<QFontMetricsF>(m_dataSource->defaultFont->getFont());
    m_dataSource->gutterWidth = 0;
    m_firstLine = 0;
    m_visibleLines = 0;
    m_maxLineWidth = 0;

    initSelectCommand();
}

const qreal ContentView::m_x0 = 2.0f;

bool ContentView::event(QEvent *e) {
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Tab) {
            insertTab();
            return true;
        }
    }
    return QWidget::event(e);
}

void ContentView::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    auto dirtyRect = event->rect(); //simple
    paint(painter, dirtyRect);
}

void ContentView::resizeEvent(QResizeEvent *event) {
    auto size = event->size();
    //QRect viewRect(0, 0, size.width(), size.height());
    //auto fl = getFirstLastVisibleLines(viewRect);
    //auto lines = fl.lines();
    //m_firstLine = fl.first;

    //if (lines != m_totalVisibleLines) {
    //	m_connection->sendScroll(m_file->viewId(), m_firstLine, m_firstLine + lines);
    //	m_totalVisibleLines = lines;
    //}

    auto linespace = getLineSpace();
    auto visibleLines = std::ceil(size.height() / qreal(linespace));
    if (m_visibleLines != visibleLines) {
        m_visibleLines = visibleLines;
        m_connection->sendScroll(m_file->viewId(), m_firstLine, m_firstLine + m_visibleLines);
    }

    QWidget::resizeEvent(event);
}

void ContentView::sendEdit(const QString &method) {
    if (!method.isEmpty()) {
        QJsonObject object;
        m_connection->sendEdit(m_file->viewId(), method, object);
    }
}

QPair<int, int> ContentView::getFirstLastVisibleLines(const QRect &bound) {
    auto linespace = m_dataSource->fontMetrics->height();
    auto topPad = linespace - m_dataSource->fontMetrics->ascent();
    auto xOff = m_dataSource->gutterWidth + m_x0 - m_scrollOrigin.x();
    auto yOff = topPad - m_scrollOrigin.y();

    auto firstVisible = qMax(0, (int)(std::ceil((bound.y() - topPad + m_scrollOrigin.y()) / linespace)));
    auto lastVisible = qMax(0, (int)(std::floor((bound.y() + bound.height() - topPad + m_scrollOrigin.y()) / linespace)));

    QPair<int, int> vlines(firstVisible, lastVisible);
    return vlines;
}

void ContentView::paint(QPainter &renderer, const QRect &dirtyRect) {

    auto theme = Perference::shared()->theme();

    // for debug
    //renderer.drawRect(rect());

    renderer.fillRect(rect(), theme.background());

    //m_scrollOrigin.setY(-40);	// down
    //m_scrollOrigin.setY(285);	// up
    //renderer.fillRect(dirtyRect, QColor(0x272822));  // bg color

    // request cache

    auto linespace = m_dataSource->fontMetrics->height();
    auto topPad = linespace - m_dataSource->fontMetrics->ascent();
    auto xOff = m_dataSource->gutterWidth + m_x0 - m_scrollOrigin.x();
    auto yOff = topPad - m_scrollOrigin.y();

    auto firstVisible = qMax(0, (int)(std::ceil((dirtyRect.y() - topPad + m_scrollOrigin.y()) / linespace)));
    auto lastVisible = qMax(0, (int)(std::floor((dirtyRect.y() + dirtyRect.height() - topPad + m_scrollOrigin.y()) / linespace)));

    auto lineCache = m_dataSource->lines;
    auto totalLines = lineCache->height();

    auto first = qMin(totalLines, firstVisible);
    auto last = qMin(totalLines, lastVisible);

    auto lines = lineCache->getLines(RangeI(first, last + 1)); // fix

    auto font = m_dataSource->defaultFont;
    auto styleMap = m_dataSource->styleMap;
    QVector<std::shared_ptr<TextLine>> textLines;

    m_firstLine = first;
    qreal maxLineWidth = 0;

    // first pass: create TextLine objects and also draw background rects
    for (auto lineIx = first; lineIx <= last; ++lineIx) {
        auto relLineIx = lineIx - first;
        auto line = lines[relLineIx];
        if (!line) {
            textLines.append(nullptr);
            continue;
        }
        auto builder = std::make_shared<TextLineBuilder>(line->getText(), font);
        builder->setFgColor(theme.foreground());
        styleMap->applyStyles(builder, line->getStyles(), theme.selection(), theme.highlight());
        auto textLine = builder->build();
        textLines.append(textLine);
        maxLineWidth = std::max(maxLineWidth, textLine->width());
        auto y0 = yOff + linespace * lineIx;
        RangeF yRange(y0, y0 + linespace);
        Painter::drawLineBg(renderer, textLine, xOff, yRange);
    }

    if (maxLineWidth != m_maxLineWidth) {
        m_maxLineWidth = maxLineWidth;
    }

    // second pass: draw text
    for (auto lineIx = first; lineIx <= last; ++lineIx) {
        auto textLine = textLines[lineIx - first];
        if (textLine) {
            auto y = yOff + m_dataSource->fontMetrics->ascent() - linespace + linespace * lineIx;
            Painter::drawLine(renderer, textLine, xOff, y);
        }
    }

    // third pass: draw text decorations
    //for (auto lineIx = first; lineIx < last; ++lineIx) {
    //	auto textLine = textLines[lineIx - first];
    //	if (textLine) {
    //		auto y = yOff + m_dataSource->fontMetrics->ascent() + linespace + lineIx;
    //		Renderer::drawLineDecorations(renderer, textLine, xOff, y);
    //	}
    //}

    // fourth pass: draw carets
    for (auto lineIx = first; lineIx <= last; ++lineIx) {
        auto relLineIx = lineIx - first;
        auto textLine = textLines[relLineIx];
        auto line = lines[relLineIx];
        if (textLine && line) {
            auto y0 = yOff + m_dataSource->fontMetrics->ascent() - linespace + linespace * lineIx;
            auto cursors = line->getCursor().get();
            foreach (int cursor, *cursors) {
                auto x0 = xOff + textLine->indexTox(cursor) - 0.5f;
                Painter::drawCursor(renderer, x0, y0, 2, linespace, theme.caret());
            }
        }
    }

    // gutter drawing
}

void ContentView::initSelectCommand() {
    m_selectorToCommand["deleteBackward"] = "delete_backward";
    m_selectorToCommand["deleteForward"] = "delete_forward";
    m_selectorToCommand["deleteToBeginningOfLine"] = "delete_to_beginning_of_line";
    m_selectorToCommand["deleteToEndOfParagraph"] = "delete_to_end_of_paragraph";
    m_selectorToCommand["deleteWordBackward"] = "delete_word_backward";
    m_selectorToCommand["deleteWordForward"] = "delete_word_forward";
    m_selectorToCommand["insertNewline"] = "insert_newline";
    m_selectorToCommand["insertTab"] = "insert_tab";
    m_selectorToCommand["moveBackward"] = "move_backward";
    m_selectorToCommand["moveDown"] = "move_down";
    m_selectorToCommand["moveDownAndModifySelection"] = "move_down_and_modify_selection";
    m_selectorToCommand["moveForward"] = "move_forward";
    m_selectorToCommand["moveLeft"] = "move_left";
    m_selectorToCommand["moveLeftAndModifySelection"] = "move_left_and_modify_selection";
    m_selectorToCommand["moveRight"] = "move_right";
    m_selectorToCommand["moveRightAndModifySelection"] = "move_right_and_modify_selection";
    m_selectorToCommand["moveToBeginningOfDocument"] = "move_to_beginning_of_document";
    m_selectorToCommand["moveToBeginningOfDocumentAndModifySelection"] = "move_to_beginning_of_document_and_modify_selection";
    m_selectorToCommand["moveToBeginningOfLine"] = "move_to_left_end_of_line";
    m_selectorToCommand["moveToBeginningOfLineAndModifySelection"] = "move_to_left_end_of_line_and_modify_selection";
    m_selectorToCommand["moveToBeginningOfParagraph"] = "move_to_beginning_of_paragraph";
    m_selectorToCommand["moveToEndOfDocument"] = "move_to_end_of_document";
    m_selectorToCommand["moveToEndOfDocumentAndModifySelection"] = "move_to_end_of_document_and_modify_selection";
    m_selectorToCommand["moveToEndOfLine"] = "move_to_right_end_of_line";
    m_selectorToCommand["moveToEndOfLineAndModifySelection"] = "move_to_right_end_of_line_and_modify_selection";
    m_selectorToCommand["moveToEndOfParagraph"] = "move_to_end_of_paragraph";
    m_selectorToCommand["moveToLeftEndOfLine"] = "move_to_left_end_of_line";
    m_selectorToCommand["moveToLeftEndOfLineAndModifySelection"] = "move_to_left_end_of_line_and_modify_selection";
    m_selectorToCommand["moveToRightEndOfLine"] = "move_to_right_end_of_line";
    m_selectorToCommand["moveToRightEndOfLineAndModifySelection"] = "move_to_right_end_of_line_and_modify_selection";
    m_selectorToCommand["moveUp"] = "move_up";
    m_selectorToCommand["moveUpAndModifySelection"] = "move_up_and_modify_selection";
    m_selectorToCommand["moveWordLeft"] = "move_word_left";
    m_selectorToCommand["moveWordLeftAndModifySelection"] = "move_word_left_and_modify_selection";
    m_selectorToCommand["moveWordRight"] = "move_word_right";
    m_selectorToCommand["moveWordRightAndModifySelection"] = "move_word_right_and_modify_selection";
    m_selectorToCommand["pageDownAndModifySelection"] = "page_down_and_modify_selection";
    m_selectorToCommand["pageUpAndModifySelection"] = "page_up_and_modify_selection";
    m_selectorToCommand["scrollPageDown"] = "scroll_page_down";
    m_selectorToCommand["scrollPageUp"] = "scroll_page_up";
    m_selectorToCommand["scrollToBeginningOfDocument"] = "move_to_beginning_of_document";
    m_selectorToCommand["scrollToEndOfDocument"] = "move_to_end_of_document";
    m_selectorToCommand["transpose"] = "transpose";
    m_selectorToCommand["yank"] = "yank";
    m_selectorToCommand["redo"] = "redo";
    m_selectorToCommand["undo"] = "undo";
    m_selectorToCommand["selectAll"] = "select_all";
    m_selectorToCommand["cancelOperation"] = "cancel_operation";
}

std::shared_ptr<File> ContentView::getFile() const {
    return m_file;
}

int ContentView::getLines() {
    return m_dataSource->lines->height();
}

qreal ContentView::getTopPad() {
    auto linespace = m_dataSource->fontMetrics->height();
    auto topPad = linespace - m_dataSource->fontMetrics->ascent();
    return topPad;
}

qreal ContentView::getMaxLineWidth() {
    return m_maxLineWidth;
}

int ContentView::getLinesHeight() {
    return getLines() * getLineSpace();
}

int ContentView::getContentHeight() {
    return getLinesHeight() + getTopPad();
}

int ContentView::getLineSpace() {
    return m_dataSource->fontMetrics->height();
}

int ContentView::getMaxCharWidth() {
    return m_dataSource->fontMetrics->maxWidth();
}

QPoint ContentView::getScrollOrigin() {
    return m_scrollOrigin;
}

int ContentView::getXOff() {
    return std::ceil(m_dataSource->gutterWidth + m_x0);
}

int ContentView::getLine(int y) {
    return qMax(0, (int)(m_scrollOrigin.y() + y - getTopPad()) / getLineSpace());
}

int ContentView::getColumn(int line, int x) {
    auto cacheLine = m_dataSource->lines->getLine(line);
    if (!cacheLine) return -1;
    auto textline = std::make_shared<TextLine>(cacheLine->getText(), m_dataSource->defaultFont);
    return textline->xToIndex(x);
}

int ContentView::checkLineVisible(int line) {
    auto viewRect = rect();
    auto fl = getFirstLastVisibleLines(viewRect);

    if (line < fl.first)
        return -1;
    else if (line > fl.second)
        return 1;
    else
        return 0;
}

qreal ContentView::getLineColumnWidth(int line, int column) {
    auto lineVisible = checkLineVisible(line);
    if (lineVisible == 0) {
        auto viewRect = rect();
        auto fl = getFirstLastVisibleLines(viewRect);

        auto lineCache = m_dataSource->lines;
        auto totalLines = lineCache->height();

        auto first = qMin(totalLines, fl.first);
        auto last = qMin(totalLines, fl.second);
        auto lines = lineCache->getLines(RangeI(first, last + 1)); // fix

        auto font = m_dataSource->defaultFont;
        auto xline = lines[line - first];

        if (xline) {
            auto textline = std::make_shared<TextLine>(xline->getText(), font);
            auto width = textline->indexTox(column);
            return width;
        } else {
            return 0;
        }
    }
    return 0;
}

int ContentView::checkLineColumnPosition(int line, int column) {
    auto delta = getLineColumnWidth(line, column) - m_scrollOrigin.x();
    if (delta < 0)
        return -1;
    else if (delta > width())
        return 1;
    else
        return 0;
}

QPair<int, int> ContentView::posToLineColumn(const QPoint &pos) {
    auto viewRect = rect();
    QPair<int, int> result(0, 0);

    auto linespace = m_dataSource->fontMetrics->height();
    auto topPad = linespace - m_dataSource->fontMetrics->ascent();
    auto xOff = m_dataSource->gutterWidth + m_x0 - m_scrollOrigin.x();
    auto yOff = topPad - m_scrollOrigin.y();

    auto firstVisible = std::max(0, (int)(std::ceil((viewRect.y() - topPad + m_scrollOrigin.y()) / linespace)));
    auto lastVisible = std::max(0, (int)(std::floor((viewRect.y() + viewRect.height() - topPad + m_scrollOrigin.y()) / linespace)));

    auto lineCache = m_dataSource->lines;
    auto totalLines = lineCache->height();

    auto first = std::min(totalLines, firstVisible);
    auto last = std::min(totalLines, lastVisible);

    auto lines = lineCache->getLines(RangeI(first, last + 1)); // fix

    auto font = m_dataSource->defaultFont;
    auto lineNum = qMax(0, int((pos.y() - topPad) / linespace + first));

    if (lineNum > last) return result;
    auto line = lines[lineNum - first];

    auto textline = std::make_shared<TextLine>(line->getText(), font);
    auto column = textline->xToIndex(pos.x() - m_x0 - m_dataSource->gutterWidth);

    result.first = lineNum;
    result.second = column;

    return result;
}

void ContentView::scrollY(int y) {
    auto value = y - getTopPad();
    auto linespace = getLineSpace();
    auto lines = getLines();
    if (lines == 0) return;

    auto first = std::max(0, (int)(std::floor(value / qreal(linespace) + 0.9))); // last line [visible]
    first = std::min(lines - 1, first);
    if (m_firstLine != first) {
        m_firstLine = first;
        m_connection->sendScroll(m_file->viewId(), m_firstLine, m_firstLine + m_visibleLines);
    }
    m_scrollOrigin.setY(m_firstLine * linespace);
    update();

    //auto check = (m_scrollOrigin.y() > y) ? -1 : 1;
    // pagedown, pageup会出错
    //m_scrollOrigin.setY(m_scrollOrigin.y() + check*linespace);
    //m_scrollOrigin.setY(y);
    //auto viewRect = rect();
    //auto fl = getFirstLastVisibleLines(viewRect);
    //auto lines = fl.lines();

    //if (m_firstLine != fl.first)
    //{
    //	m_firstLine = fl.first;
    //	m_connection->sendScroll(m_file->viewId(), fl.first, fl.last);
    //	m_totalVisibleLines = lines;
    //}
    //update();
}

void ContentView::scrollX(int x) {
    m_scrollOrigin.setX(x);
    update();
}

void ContentView::updateHandler(const QJsonObject &json) {
    m_dataSource->lines->applyUpdate(json);
    update();
}

void ContentView::scrollHandler(int line, int column) {
}

void ContentView::keyPressEvent(QKeyEvent *ev) {
    auto modifiers = ev->modifiers();
    auto ctrl = (modifiers & Qt::ControlModifier);
    auto alt = (modifiers & Qt::AltModifier);
    auto shift = (modifiers & Qt::ShiftModifier);
    auto meta = (modifiers & Qt::MetaModifier);
    auto none = (modifiers == Qt::NoModifier);

    switch (ev->key()) {
    case Qt::Key_Delete:
        deleteForward();
        return;
    case Qt::Key_Backspace:
        deleteBackward();
        return;
    case Qt::Key_Return:
        insertNewline();
        return;
    case Qt::Key_Home:
        if (none)
            moveToBeginningOfLine();
        else if (ctrl && shift)
            moveToBeginningOfDocumentAndModifySelection();
        else if (shift)
            moveToBeginningOfLineAndModifySelection();
        else if (ctrl)
            moveToBeginningOfDocument();
        return;
    case Qt::Key_End:
        if (none)
            moveToEndOfLine();
        else if (ctrl && shift)
            moveToEndOfDocumentAndModifySelection();
        else if (shift)
            moveToEndOfLineAndModifySelection();
        else if (ctrl)
            moveToEndOfDocument();
        return;
    case Qt::Key_PageDown:
        if (none)
            scrollPageDown();
        else if (shift)
            pageDownAndModifySelection();
        return;
    case Qt::Key_PageUp:
        if (none)
            scrollPageUp();
        else if (shift)
            pageUpAndModifySelection();
        return;
    case Qt::Key_Up:
        if (none)
            moveUp();
        else if (shift)
            moveUpAndModifySelection();
        return;
    case Qt::Key_Down:
        if (none)
            moveDown();
        else if (shift)
            moveDownAndModifySelection();
        return;
    case Qt::Key_Left:
        if (none)
            moveLeft();
        else if (ctrl && shift)
            moveWordLeftAndModifySelection();
        else if (ctrl)
            moveWordLeft();
        else if (shift)
            moveLeftAndModifySelection();
        return;
    case Qt::Key_Right:
        if (none)
            moveRight();
        else if (ctrl && shift)
            moveWordRightAndModifySelection();
        else if (ctrl)
            moveWordRight();
        else if (shift)
            moveRightAndModifySelection();
        return;
    case Qt::Key_Z:
        if (ctrl) {
            undo();
            return;
        }
        break;
    case Qt::Key_Y:
        if (ctrl) {
            redo();
            return;
        }
        break;
    case Qt::Key_C:
        if (ctrl) {
            copy();
            return;
        }
        break;
    case Qt::Key_X:
        if (ctrl) {
            cut();
            return;
        }
        break;
    case Qt::Key_V:
        if (ctrl) {
            paste();
            return;
        }
        break;
    case Qt::Key_A:
        if (ctrl) {
            selectAll();
            return;
        }
        break;
    default:
        break;
    };

    QString text = ev->text();
    if (!text.isEmpty()) {
        insertChar(text);
    } else {
        QWidget::keyPressEvent(ev);
    }
}

void ContentView::mousePressEvent(QMouseEvent *e) {
    setFocus();
    auto lc = posToLineColumn(e->pos());
    auto line = lc.first, column = lc.second;
    m_connection->sendGesture(m_file->viewId(), line, column, "point_select");

    QWidget::mousePressEvent(e);
}

void ContentView::mouseMoveEvent(QMouseEvent *e) {
    setFocus();
    //auto lc = posToLineColumn(e->pos());
    //auto line = lc.first, column = lc.second;
    //m_connection->sendGesture(m_file->viewId(), line, column, "point_select");
    QWidget::mouseMoveEvent(e);
}

void ContentView::mouseReleaseEvent(QMouseEvent *e) {
    setFocus();
    //auto lc = posToLineColumn(e->pos());
    //auto line = lc.first, column = lc.second;
    QWidget::mouseReleaseEvent(e);
}

void ContentView::mouseDoubleClickEvent(QMouseEvent *e) {
    setFocus();
    auto lc = posToLineColumn(e->pos());
    auto line = lc.first, column = lc.second;
    m_connection->sendGesture(m_file->viewId(), line, column, "word_select");
    QWidget::mouseDoubleClickEvent(e);
}

void ContentView::insertChar(const QString &text) {
    QJsonObject object;
    object["chars"] = text;
    m_connection->sendEdit(m_file->viewId(), "insert", object);
}

void ContentView::copy() {
    ResponseHandler handler([&](const QJsonObject &json) {
        qDebug() << "copy ResponseHandler";
        qDebug() << json;
        QClipboard *clipboard = QApplication::clipboard();
        auto text = json["result"].toString();
        clipboard->setText(text);
    });
    m_connection->sendCopy(m_file->viewId(), handler);
}

void ContentView::cut() {
    ResponseHandler handler([&](const QJsonObject &json) {
        qDebug() << "cut ResponseHandler";
        qDebug() << json;
        QClipboard *clipboard = QApplication::clipboard();
        auto text = json["result"].toString();
        clipboard->setText(text);
    });
    m_connection->sendCut(m_file->viewId(), handler);
}

void ContentView::paste() {
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (mimeData->hasText()) {
        auto text = mimeData->text();
        m_connection->sendInsert(m_file->viewId(), text);
    } else {
        qDebug() << "unknown clipboard data";
    }
}

} // namespace xi
