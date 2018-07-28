#ifndef CONTENT_VIEW_H
#define CONTENT_VIEW_H

#include <QAbstractScrollArea>
#include <QFontMetrics>
#include <QFontMetricsF>
#include <QGridLayout>
#include <QMargins>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QPoint>
#include <QQueue>
#include <QScrollArea>
#include <QTimer>

#include <memory>

#include "core_connection.h"
#include "file.h"
#include "font.h"
#include "line_cache.h"

//! TODO: OPENGL VSYNC FPS
// #define ENABLE_GPU_RENDERING
// #define ENABLE_SHADER

namespace xi {

class Config;

// TODO (CV)
class DataSource {
public:
    DataSource();
    std::shared_ptr<Font> defaultFont;
    // TODO
    //std::shared_ptr<Font> cjkFont;
    //bool cjkFontEnable;
    std::shared_ptr<LineCache> lines;
    std::shared_ptr<Config> config;
    std::shared_ptr<QFontMetricsF> fontMetrics;
    qreal gutterWidth;
};

class LineColumn {
public:
    static LineColumn invalid() {
        return LineColumn(false);
    }

    LineColumn(bool valid = true) {
        setValid(valid);
    }
    LineColumn(int line, int column) {
        lc.first = line;
        lc.second = column;
        setValid(true);
    }
    inline bool isValid() const {
        return valid;
    }
    inline void setValid(bool valid) {
        this->valid = valid;
    }
    inline int line() const {
        return lc.first;
    }
    inline void line(int line) {
        lc.first = line;
    }
    inline int column() const {
        return lc.second;
    }
    inline void column(int column) {
        lc.second = column;
    }

private:
    bool valid;
    QPair<int, int> lc;
};

class ContentView;

class AsyncPaintTimer : public QObject {
    Q_OBJECT
public:
    AsyncPaintTimer(QWidget *parent);

public slots:
    void update();

private:
    ContentView *m_contentView = nullptr;
    std::unique_ptr<QTimer> m_timer;
};

#define SEND_EDIT_METHOD(TypeName) \
    void TypeName() { sendEdit(m_selectorToCommand[#TypeName]); }

// Main Content
#ifdef ENABLE_GPU_RENDERING
class ContentView : public QOpenGLWidget, protected QOpenGLFunctions {
#else
class ContentView : public QWidget {
#endif
    Q_OBJECT
public:
    friend class AsyncPaintTimer;

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
    int getLinespace();
    int getMaxCharWidth();
    QPoint getScrollOrigin();
    int getXOff();

    int getLine(int y);
    int getColumn(int line, int x);

    int checkLineVisible(int line);
    qreal getLineColumnWidth(int line, int column);
    int checkLineColumnPosition(int line, int column);
    LineColumn posToLineColumn(const QPoint &pos);
    ClosedRangeI getFirstLastVisibleLines(const QRect &bound);

    // TODO. FAST SCROLL LOOOONG FILE
    void asyncPaint(int ms = 100);

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
    void availablePluginsHandler(const QList<QJsonObject> &plugins);
    void updateCommandsHandler(const QStringList &commands);
    void configChangedHandler(const QJsonObject &changes);

    //II
    void themeChangedHandler();

private:
    std::shared_ptr<File> m_file;
    std::shared_ptr<CoreConnection> m_connection;
    std::shared_ptr<DataSource> m_dataSource; //owned
    QPoint m_scrollOrigin;
    int m_firstLine;
    int m_visibleLines;
    qreal m_maxLineWidth;
    QHash<QString, QString> m_selectorToCommand;
    QMarginsF m_padding;
    bool m_drag = false;
    QTimer m_mouseDoubleCheckTimer;
    std::unique_ptr<AsyncPaintTimer> m_asyncPaintTimer;
    QQueue<qint64> m_asyncPaintQueue;
};

} // namespace xi

#endif // CONTENT_VIEW_H