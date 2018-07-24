#ifndef CONTENT_VIEW_H
#define CONTENT_VIEW_H

#include <QAbstractScrollArea>
#include <QFontMetrics>
#include <QFontMetricsF>
#include <QGridLayout>
#include <QPoint>
#include <QScrollArea>

#include <memory>

#include "core_connection.h"
#include "file.h"
#include "font.h"
#include "line_cache.h"

namespace xi {

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
    void TypeName() { sendEdit(m_selectorToCommand[#TypeName]); }

// Main Content
class ContentView : public QWidget {
    Q_OBJECT
public:
    ContentView(const std::shared_ptr<File> &file, const std::shared_ptr<CoreConnection> &connection, QWidget *parent);

protected:
    virtual bool event(QEvent *e) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *e) override;
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseMoveEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *e) override;

    void paint(QPainter &renderer, const QRect &dirtyRect);
    void initSelectCommand();

public:
    std::shared_ptr<File> getFile() const;
	int getLines();
	qreal getTopPad();
    qreal getMaxLineWidth();
	int getLinesHeight();
	int getContentHeight();
	int getLineSpace();
	int getMaxCharWidth();
    QPoint getScrollOrigin();
	int getXOff();

	int getLine(int y);
	int getColumn(int line, int x);

    int checkLineVisible(int line);
    qreal getLineColumnWidth(int line, int column);
    int checkLineColumnPosition(int line, int column);
    QPair<int, int> posToLineColumn(const QPoint &pos);
	QPair<int, int> getFirstLastVisibleLines(const QRect &bound);

    void scrollY(int y);
    void scrollX(int x);

    void sendEdit(const QString &method);

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

    void insertChar(const QString &text);
    void copy();
    void cut();
    void paste();

signals:

public slots:

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
	QPointF m_margin;
};
} // namespace xi

#endif // CONTENT_VIEW_H