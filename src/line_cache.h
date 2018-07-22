#ifndef LINECACHE_H
#define LINECACHE_H

//! The line cache (text, styles and cursors for a view).

#include <QObject>
#include <QList>
#include <QJsonArray>
#include <QVector>
#include <qopengl.h>

#include <list>
#include <vector>

#include "style_map.h"
#include "unfair_lock.h"


namespace xi {

class Line;

class Line
{
public:
    friend class LineCache;
    friend class TextLine;

	Line(const QJsonObject &object);
	Line(const Line* line, const QJsonObject &object);
	// Line(std::shared_ptr<Line> line, const QJsonObject &object);

	Line& operator=(const Line& line) {
		if (this == &line) {
			return *this;
		}
		m_text = line.m_text;
		m_cursor = line.m_cursor;
		m_styles = line.m_styles;
		return *this;
	}

	//bool operator==(const Line& line) {
	//	return 
	//}
	QString getText() {
		return m_text;
	}
	std::shared_ptr<QVector<qint32>> getCursor() {
		return m_cursor;
	}
	std::shared_ptr<QVector<StyleSpan>> getStyles() {
		return m_styles;
	}

private:
	QString m_text;
	std::shared_ptr<QVector<qint32>> m_cursor;
    std::shared_ptr<QVector<StyleSpan>> m_styles;
};

namespace
{

class LineCacheState : UnfairLock {
private:
	// auto waitingForLines //async
};

};

class LineCache
{
public:
    explicit LineCache();
	void applyUpdate(const QJsonObject &json);
	inline int height() { return m_height; }
	std::vector<std::shared_ptr<Line>> getLines(const RangeI& range) {
		std::vector<std::shared_ptr<Line>> result;
		for (auto i = range.start() - m_invalidBefore; i < range.end(); ++i) {
			if (i < m_lines.size()) {
				result.push_back(m_lines[i]);
			} else {
				result.push_back(nullptr);
			}
		}
		return result;
	}

private:
    std::vector<std::shared_ptr<Line>> m_lines;
	qint32 m_height;
	qint32 m_invalidBefore;
	qint32 m_invalidAfter;
	qint32 m_revision;
};

} // xi

#endif // LINECACHE_H
