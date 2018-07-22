#ifndef CONTENT_VIEW_H
#define CONTENT_VIEW_H

#include <QAbstractScrollArea>
#include <QScrollArea>
#include <QGridLayout>
#include <QFontMetrics>
#include <QFontMetricsF>
#include <QPoint>

#include <memory>

#include "core_connection.h"
#include "file.h"
#include "font.h"
#include "line_cache.h"


namespace xi {

struct PaintInfo {
	qint64 first;
	qint64 last;
	qreal linespace;
	qreal maxLineWidth;
};

struct VisibleLines {
	VisibleLines(int first, int last) : first(first), last(last) {
	}
	int first;
	int last;
	int lines() {
		return last - first + 1;
	}
};

class EditViewDataSource {
public:
	EditViewDataSource() {
		lines = std::make_shared<LineCache>();
	}
	// settings
	std::shared_ptr<Font> defaultFont;
	// V2 todo:
	//std::shared_ptr<XiFont> cjkFont;
	//bool cjkFontEnable;
	std::shared_ptr<LineCache> lines;
	//std::shared_ptr<Theme> theme;
	std::shared_ptr<StyleMap> styleMap;
	std::shared_ptr<QFontMetricsF> fontMetrics;
	qreal gutterWidth;
};

#define SEND_EDIT_METHOD(TypeName) \
  void TypeName() { sendEdit(m_selectorToCommand[#TypeName]); } \

// Main Content
class ContentView : public QWidget {
	Q_OBJECT
public:
	ContentView(const std::shared_ptr<File>& file,
		const std::shared_ptr<CoreConnection>& connection,
		QWidget *parent);

	virtual bool event(QEvent *e) override;
	virtual void paintEvent(QPaintEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *e) override;
	virtual void mousePressEvent(QMouseEvent *e) override;
	virtual void mouseMoveEvent(QMouseEvent *e) override;
	virtual void mouseReleaseEvent(QMouseEvent *e) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *e) override;

private:
	void paint(QPainter& renderer, const QRect& dirtyRect);
	void initSelectCommand();

public:
	std::shared_ptr<File> getFile() const {
		return m_file;
	}

	int getLines();
	qreal getMaxLineWidth();
	qint64 getLinesHeight();
	int getLineSpace();
	int getCharWidth();
	QPoint getScrollOrigin();
	int getXOff();
	int checkLineVisible(int line);
	qreal getLineColumnWidth(int line, int column);
	int checkLineColumnPosition(int line, int column);
	QPair<qint64, qint64> posToLineColumn(const QPoint& pos);
	VisibleLines getFirstLastVisibleLines(const QRect& bound);
	void scrollY(int y);
	void scrollX(int x);

	void sendEdit(const QString& method);

	SEND_EDIT_METHOD(deleteBackward);
	SEND_EDIT_METHOD(deleteForward);
	SEND_EDIT_METHOD(insertNewline);
	SEND_EDIT_METHOD(insertTab);	

	SEND_EDIT_METHOD(moveLeft);
	SEND_EDIT_METHOD(moveWordLeft);
	SEND_EDIT_METHOD(moveWordLeftAndModifySelection);
	SEND_EDIT_METHOD(moveLeftAndModifySelection);

	SEND_EDIT_METHOD(moveRight);
	SEND_EDIT_METHOD(moveWordRight);
	SEND_EDIT_METHOD(moveWordRightAndModifySelection);
	SEND_EDIT_METHOD(moveRightAndModifySelection);

	SEND_EDIT_METHOD(moveUp);
	SEND_EDIT_METHOD(moveUpAndModifySelection);

	SEND_EDIT_METHOD(moveDown);
	SEND_EDIT_METHOD(moveDownAndModifySelection);

	SEND_EDIT_METHOD(moveToBeginningOfLine);
	SEND_EDIT_METHOD(moveToBeginningOfDocumentAndModifySelection);
	SEND_EDIT_METHOD(moveToBeginningOfLineAndModifySelection);
	SEND_EDIT_METHOD(moveToBeginningOfDocument);

	SEND_EDIT_METHOD(moveToEndOfLine);
	SEND_EDIT_METHOD(moveToEndOfDocumentAndModifySelection);
	SEND_EDIT_METHOD(moveToEndOfLineAndModifySelection);
	SEND_EDIT_METHOD(moveToEndOfDocument);

	SEND_EDIT_METHOD(scrollPageDown);
	SEND_EDIT_METHOD(pageDownAndModifySelection);

	SEND_EDIT_METHOD(scrollPageUp);
	SEND_EDIT_METHOD(pageUpAndModifySelection);

	SEND_EDIT_METHOD(selectAll);

	SEND_EDIT_METHOD(uppercase);
	SEND_EDIT_METHOD(lowercase);
	SEND_EDIT_METHOD(undo);
	SEND_EDIT_METHOD(redo);

	void insertChar(const QString& text);
	void copy();
	void cut();
	void paste();

protected:
	//virtual bool event(QEvent *e) override;
	//virtual void timerEvent(QTimerEvent *e) override;

	//virtual void keyReleaseEvent(QKeyEvent *e) override;

	//virtual void resizeEvent(QResizeEvent *e) override;
	//virtual void paintEvent(QPaintEvent *e) override;

	//virtual void mousePressEvent(QMouseEvent *e) override;
	//virtual void mouseMoveEvent(QMouseEvent *e) override;
	//virtual void mouseReleaseEvent(QMouseEvent *e) override;
	//virtual void mouseDoubleClickEvent(QMouseEvent *e) override;
	//virtual bool focusNextPrevChild(bool next) override;
	//virtual void contextMenuEvent(QContextMenuEvent *e) override;
	//virtual void dragEnterEvent(QDragEnterEvent *e) override;
	//virtual void dragLeaveEvent(QDragLeaveEvent *e) override;
	//virtual void dragMoveEvent(QDragMoveEvent *e) override;
	//virtual void dropEvent(QDropEvent *e) override;
	//virtual void focusInEvent(QFocusEvent *e) override;
	//virtual void focusOutEvent(QFocusEvent *e) override;
	//virtual void showEvent(QShowEvent *) override;
	//virtual void changeEvent(QEvent *e) override;
	//virtual void wheelEvent(QWheelEvent *e) override;
	//virtual void inputMethodEvent(QInputMethodEvent *) override;
	//virtual void scrollContentsBy(int dx, int dy) override;

	//void alertHandler(const QString &text);
	//void defineStyleHandler(const QJsonObject &params);
	//void availableThemesHandler(const QVector<QString> &themes);
	//void themeChangedHandler(const QString &name, const Theme &theme);

signals:

public slots:

	// Handler
public:
	void updateHandler(const QJsonObject &update);
	void scrollHandler(int line, int column);
	void pluginStartedHandler(const QString &pluginName);
	void pluginStoppedHandler(const QString &pluginName);
	void availablePluginsHandler(const QVector<QJsonObject> &plugins);
	void updateCommandsHandler(const QVector<QString> &commands);
	void configChangedHandler(const QJsonObject &changes);

private:
	std::shared_ptr<File> m_file;
	std::shared_ptr<CoreConnection> m_connection;
	std::shared_ptr<EditViewDataSource> m_dataSource;
	const static qreal m_x0;
	QPoint m_scrollOrigin;
	int m_firstLine;
	int m_visibleLines;
	qreal m_maxLineWidth;
	QHash<QString, QString> m_selectorToCommand;
};
} // xi

#endif // CONTENT_VIEW_H