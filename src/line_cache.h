#ifndef LINECACHE_H
#define LINECACHE_H

#include <QJsonArray>
#include <QList>
#include <QObject>
#include <QSemaphore>
#include <QVector>
#include <qopengl.h>

#include <list>
#include <vector>

#include "style_map.h"
#include "unfair_lock.h"

namespace xi {

class Line;
class TextLine;

using CacheLines = QList<std::shared_ptr<Line>>;

class InvalSet {
public:
    QList<RangeI> ranges() {
        return m_ranges;
    }

    void addRange(int start, int end) {
        if (m_ranges.size() != 0 && m_ranges.last().end() == start) {
            m_ranges.last() = RangeI(m_ranges.last().start(), end);
        } else {
            m_ranges.append(RangeI(start, end));
        }
    }

    void addRangeN(int start, int n) {
        addRange(start, start + n);
    }

private:
    QList<RangeI> m_ranges;
};

class Line {
public:
    friend class LineCache;
    friend class TextLine;

    Line(const QJsonObject &object);
    Line(std::shared_ptr<Line> line, const QJsonObject &object);

    Line &operator=(const Line &line);

    QString getText() {
        return m_text;
    }
    bool containsCursor() {
        return m_cursor->count() > 0;
    }
    std::shared_ptr<QList<int>> getCursor() {
        return m_cursor;
    }
    std::shared_ptr<QList<StyleSpan>> getStyles() {
        return m_styles;
    }
    void setAssoc(const std::shared_ptr<TextLine> &assoc) {
        m_assoc = assoc;
    }
    std::shared_ptr<TextLine> assoc() {
        return m_assoc;
    }

private:
    QString m_text;
    std::shared_ptr<QList<int>> m_cursor;
    std::shared_ptr<QList<StyleSpan>> m_styles;
    std::shared_ptr<TextLine> m_assoc;
};

class LineCacheState : public UnfairLock {
    friend class LineCacheLocked;

public:
    LineCacheState() {
        m_waitingForLines = std::make_unique<QSemaphore>(0);
    }

    bool isEmpty() {
        return m_lines.count() == 0;
    }

    int height() {
        return m_invalidBefore + m_lines.count() + m_invalidAfter;
    }

    int revision() {
        return m_revision;
    }

    void setAssoc(int ix, std::shared_ptr<TextLine> assoc) {
        Q_ASSERT(ix >= m_invalidBefore);
        ix -= m_invalidBefore;
        Q_ASSERT(ix < m_lines.count());
        m_lines[ix]->setAssoc(assoc);
    }

    void flushAssoc() {
        foreach (const std::shared_ptr<Line> &line, m_lines) {
            line->setAssoc(nullptr);
        }
    }

    CacheLines linesForRange(const RangeI &range) {
        CacheLines lines;
        for (auto i = range.start(); i < range.end(); ++i) {
            lines.append(get(i));
        }
        return lines;
    }

    std::shared_ptr<Line> get(int ix) {
        if (ix < m_invalidBefore) return nullptr;
        ix -= m_invalidBefore;
        if (ix < m_lines.count()) {
            return m_lines[ix];
        }
        return nullptr;
    }

    InvalSet cursorInval() {
        InvalSet inval;
        for (auto i = 0; i < m_lines.count(); ++i) {
            const auto &line = m_lines[i];
            if (line->containsCursor()) {
                inval.addRangeN(i + m_invalidBefore, 1);
            }
        }
        return inval;
    }

    InvalSet applyUpdate(const QJsonObject &json);

private:
    std::unique_ptr<QSemaphore> m_waitingForLines;
    bool m_isWaiting = false;
    int m_revision = 1;
    int m_invalidBefore = 0;
    int m_invalidAfter = 0;
    CacheLines m_lines;
};

class LineCacheLocked {
public:
    LineCacheLocked(const std::shared_ptr<LineCacheState> &mutex) {
        m_inner = mutex;
        m_inner->lock();
    }

    ~LineCacheLocked() {
        m_inner->unlock();
        if (m_shouldSignal) {
            m_inner->m_waitingForLines->release();
            m_shouldSignal = false;
        }
    }

    int MAX_BLOCK_MS = 30;

    bool isEmpty() {
        return m_inner->isEmpty();
    }

    int height() {
        return m_inner->height();
    }

    int revision() {
        return m_inner->revision();
    }

    std::shared_ptr<Line> get(int ix) {
        return m_inner->get(ix);
    }

    void setAssoc(int ix, const std::shared_ptr<TextLine> &assoc) {
        m_inner->setAssoc(ix, assoc);
    }

    void flushAssoc() {
        m_inner->flushAssoc();
    }

    CacheLines blockingGet(const RangeI &range) {
        auto lines = m_inner->linesForRange(range);
        auto missingLines = isMissingLines(lines, range);
        if (missingLines) {
            m_inner->m_isWaiting = true;
            m_inner->unlock();
            auto waitResult = m_inner->m_waitingForLines->tryAcquire(1, MAX_BLOCK_MS);
            m_inner->lock();

            if (m_inner->m_isWaiting) {
                m_inner->m_isWaiting = false;
            } else {
                if (!waitResult) {
                    m_inner->m_waitingForLines->acquire();
                }
            }
        } else {
            return lines;
        }
        return m_inner->linesForRange(range);
    }

    bool isMissingLines(const CacheLines &lines, const RangeI &range) {
        foreach (const std::shared_ptr<Line> &line, lines) {
            if (!line) return true;
        }
        return false;
    }

    InvalSet cursorInval() {
        return m_inner->cursorInval();
    }

    InvalSet applyUpdate(const QJsonObject &json) {
        return m_inner->applyUpdate(json);
    }

private:
    std::shared_ptr<LineCacheState> m_inner;
    bool m_shouldSignal = false;
};

class LineCache {
public:
    LineCache() {
        m_state = std::make_shared<LineCacheState>();
    }
    std::shared_ptr<LineCacheLocked> locked() {
        return std::make_shared<LineCacheLocked>(m_state);
    }

    bool isEmpty() {
        return locked()->isEmpty();
    }

    int height() {
        return locked()->height();
    }

    InvalSet cursorInval() {
        return locked()->cursorInval();
    }

private:
    std::shared_ptr<LineCacheState> m_state;
};

} // namespace xi

#endif // LINECACHE_H
