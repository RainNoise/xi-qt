#include "edit_view.h"

#include <QPainter>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QAbstractScrollArea>
#include <QScrollArea>
#include <QGridLayout>
#include <QFontMetrics>
#include <QFontMetricsF>
#include <QPoint>

#include <cmath>

#include "edit_window.h"
#include "text_line.h"
#include "content_view.h"


namespace xi {

static const int kScrollbarHStep = 100;

EditView::EditView(const std::shared_ptr<File>& file, const std::shared_ptr<CoreConnection>& connection, QWidget *parent):
	QWidget(parent) {

    setFocusPolicy(Qt::FocusPolicy::NoFocus);
	setAttribute(Qt::WA_KeyCompression, false);	
    setAttribute(Qt::WA_StaticContents);
	setAttribute(Qt::WA_Resized);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_content = new ContentView(file, connection, this);
	m_scrollBarV = new QScrollBar(this);
	m_scrollBarH = new QScrollBar(Qt::Horizontal, this);
	QGridLayout* layout = new QGridLayout();
	layout->addWidget(m_content, 1, 1);
	layout->addWidget(m_scrollBarV, 1, 2);
	layout->addWidget(m_scrollBarH, 2, 1);
	layout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(layout);

    connect(m_scrollBarV, &QScrollBar::valueChanged, this, &EditView::scrollBarVChanged);
	connect(m_scrollBarH, &QScrollBar::valueChanged, this, &EditView::scrollBarHChanged);
}

void EditView::updateHandler(const QJsonObject &json) {
	if (m_content) {
		m_content->updateHandler(json);
	}
	relayoutScrollBar();
}

void EditView::scrollHandler(int line, int column) {
	auto linespace = m_content->getLineSpace();
	auto lines = m_content->getLines();
	auto value = m_scrollBarV->value();
	auto nextLine = line + 1;

	relayoutScrollBar();

	if (nextLine*linespace > value + m_content->height()) {
		m_scrollBarV->setValue(nextLine*linespace - m_content->height());
	}
	if (line*linespace < value) {
		m_scrollBarV->setValue(line*linespace);
	}

	//auto linecheck = m_content->checkLineVisible(line);
	//if (linecheck != 0) {
	//	auto linespace = m_content->getLineSpace();
	//	auto delta = line * linespace; 
	//	m_scrollBarV->setValue(delta); // -m_content->height()
	//}
	auto check = m_content->checkLineColumnPosition(line, column);
	if (check != 0) {
		auto delta = check * m_content->width() / 2.f;
		auto value = m_scrollBarH->value();
		m_scrollBarH->setValue(value + delta); // 
	}
}

void EditView::themeChangedHandler(const QString &name, const Theme &theme) {

}

void EditView::resizeEvent(QResizeEvent *event) {
	relayoutScrollBar();
	QWidget::resizeEvent(event);
}

void EditView::keyPressEvent(QKeyEvent *e) {
	QWidget::keyPressEvent(e);
}

void EditView::relayoutScrollBar() {
	auto contentHeight = m_content->height();
	auto contentWidth = m_content->width();
	auto linespace = m_content->getLineSpace();

	auto linesHeight = m_content->getLinesHeight();
	m_scrollBarV->setRange(0, linesHeight-linespace); // keep one line
	m_scrollBarV->setSingleStep(m_content->getLineSpace());
	m_scrollBarV->setPageStep(contentHeight);
	m_scrollBarV->setVisible(linesHeight > contentHeight);

	auto maxLineWidth = m_content->getMaxLineWidth();
	auto maxCharWidth = m_content->getCharWidth();
	auto xOff = m_content->getXOff();
	m_scrollBarH->setRange(0, maxLineWidth);
	m_scrollBarH->setSingleStep(maxCharWidth);
	m_scrollBarH->setPageStep(contentWidth);
	auto visible = (maxLineWidth + xOff + maxCharWidth / 2.f > contentWidth);
	if (!visible && m_content->getScrollOrigin().x() != 0) visible = true;
	m_scrollBarH->setVisible(visible);
}

void EditView::wheelEvent(QWheelEvent *event) {
	if (m_scrollBarV->isVisible()) {
		auto linespace = m_content->getLineSpace();
		auto delta = -(event->delta() / (120) * (linespace * 3));
		auto v = m_scrollBarV->value();
		m_scrollBarV->setValue(v + delta);
	}
}

std::shared_ptr<File> EditView::getFile() const {
	return m_content->getFile();
}

void EditView::focusOnEdit() {
	m_content->setFocus();
	m_content->update();
}

void EditView::scrollBarVChanged(int y) {
	m_content->scrollY(y);
	relayoutScrollBar();
}

void EditView::scrollBarHChanged(int x) {
	m_content->scrollX(x);
	relayoutScrollBar();
}

} // xi