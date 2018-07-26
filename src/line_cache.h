#ifndef LINECACHE_H
#define LINECACHE_H

#include <QJsonArray>
#include <QList>
#include <QObject>
#include <QVector>
#include <qopengl.h>

#include <list>
#include <vector>

#include "style_map.h"
#include "unfair_lock.h"

namespace xi {

class Line;
class TextLine;

class Line {
public:
    friend class LineCache;
    friend class TextLine;

    Line(const QJsonObject &object);
    // Line(const Line *line, const QJsonObject &object);
    Line(std::shared_ptr<Line> line, const QJsonObject &object);

    Line &operator=(const Line &line);

    QString getText() {
        return m_text;
    }
    std::shared_ptr<QVector<int>> getCursor() {
        return m_cursor;
    }
    std::shared_ptr<QVector<StyleSpan>> getStyles() {
        return m_styles;
    }
    void setAssoc(std::shared_ptr<TextLine> assoc) {
        m_assoc = assoc;
    }
    std::shared_ptr<TextLine> assoc() {
        return m_assoc;
    }

private:
    QString m_text;
    std::shared_ptr<QVector<int>> m_cursor;
    std::shared_ptr<QVector<StyleSpan>> m_styles;
    std::shared_ptr<TextLine> m_assoc;
};

namespace {

class LineCacheState : UnfairLock {
private:
    // auto waitingForLines //async
};

}; // namespace

class LineCache {
public:
    explicit LineCache();
    void applyUpdate(const QJsonObject &json);
    inline int height() { return m_height; }
    std::shared_ptr<Line> getLine(int ix);
    std::vector<std::shared_ptr<Line>> getLines(const RangeI &range);

private:
    std::vector<std::shared_ptr<Line>> m_lines;
    int m_height;
    int m_invalidBefore;
    int m_invalidAfter;
    int m_revision;
};

} // namespace xi

#endif // LINECACHE_H
