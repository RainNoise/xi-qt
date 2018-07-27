#include "line_cache.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "core_connection.h"

namespace xi {

LineCache::LineCache() {
    m_height = 0;
    m_revision = 0;
    m_invalidBefore = 0;
    m_invalidAfter = 0;
}

void LineCache::applyUpdate(const QJsonObject &json) {
    if (!json.contains("ops") || !json["ops"].isArray()) {
        return;
    }

    auto oldHeight = m_height;
    int newInvalidBefore = 0;
    int newInvalidAfter = 0;
    int oldIdx = 0;
    CacheLines newLines;

    auto ops = json["ops"].toArray();
    for (auto opref : ops) {
        auto op = opref.toObject();
        auto opTypeStr = op["op"].toString();
        qDebug() << "update--> op type: " << opTypeStr;
        auto opType = CoreConnection::ops(opTypeStr);
        auto n = op["n"].toInt();
        switch (opType) {
        case CoreConnection::Ops_Invalidate: {
            //auto curLine = newInvalidBefore + newLines.size() + newInvalidAfter;
            //auto ix = curLine - m_invalidBefore;
            //if (ix + n > 0 && ix < m_lines.size()) {
            //	for (auto i = std::max((qint32)ix, (qint32)0); i < std::min((qint32)ix + n, (qint32)m_lines.size()); ++i) {
            //		if (m_lines[i] != nullptr) {
            //			//...
            //		}
            //	}
            //}
            if (newLines.size() == 0) {
                newInvalidBefore += n;
            } else {
                newInvalidAfter += n;
            }
        } break;
        case CoreConnection::Ops_Ins: {
            for (int i = 0; i < newInvalidAfter; ++i) {
                newLines.push_back(nullptr);
            }
            newInvalidAfter = 0;
            QJsonArray jsonLines = op["lines"].toArray();
            for (auto jsonLine : jsonLines) {
                newLines.push_back(std::make_shared<Line>(jsonLine.toObject()));
            }
        } break;
        case CoreConnection::Ops_Copy:
        case CoreConnection::Ops_Update: {
            auto nRemaining = n;
            if (oldIdx < m_invalidBefore) {
                auto nInvalid = qMin(n, m_invalidBefore - oldIdx);
                if (newLines.size() == 0) {
                    newInvalidBefore += nInvalid;
                } else {
                    newInvalidAfter += nInvalid;
                }
                oldIdx += nInvalid;
                nRemaining -= nInvalid;
            }
            if (nRemaining > 0 && oldIdx < m_invalidBefore + m_lines.size()) {
                for (int i = 0; i < newInvalidAfter; ++i) {
                    newLines.push_back(nullptr);
                }
                newInvalidAfter = 0;
                auto nCopy = qMin(nRemaining, m_invalidBefore + (int)m_lines.size() - oldIdx);
                if (oldIdx != newInvalidBefore + newLines.size() || opType != CoreConnection::Ops_Copy) {
                    //...
                }
                auto startIx = oldIdx - m_invalidBefore;
                if (opType == CoreConnection::Ops_Copy) {
                    for (auto i = startIx; i < startIx + nCopy; ++i) {
                        newLines.push_back(std::move(m_lines[i]));
                    }
                } else { // CoreConnection::Ops_Update
                    QJsonArray jsonLines = op["lines"].toArray();
                    auto jsonIx = n - nRemaining;
                    for (auto ix = startIx; ix < startIx + nCopy; ++ix) {
                        newLines.push_back(std::make_shared<Line>(m_lines[ix], jsonLines[ix].toObject()));
                        //m_lines[ix]->update(jsonLines[ix].toObject());
                        //newLines.push_back(std::move(m_lines[i]));
                        jsonIx += 1;
                    }
                }
                oldIdx += nCopy;
                nRemaining -= nCopy;
            }
            if (newLines.size() == 0) {
                newInvalidBefore += nRemaining;
            } else {
                newInvalidAfter += nRemaining;
            }
            oldIdx += nRemaining;
        } break;
        case CoreConnection::Ops_Skip:
            oldIdx += n;
            break;
        default:
            qDebug() << "unknown op type " << opTypeStr;
            break;
        }
    }

    m_invalidBefore = newInvalidBefore;
    m_lines = newLines;
    m_invalidAfter = newInvalidAfter;
    m_height = m_invalidBefore + m_lines.size() + m_invalidAfter;
    m_revision++;
}

std::shared_ptr<xi::Line> LineCache::getLine(int ix) {
    return m_lines[ix];
}

LineCache::CacheLines LineCache::getLines(const RangeI &range) {
    CacheLines result;
    for (auto i = qMax(0, range.start() - m_invalidBefore); i < range.end(); ++i) {
        if (i < m_lines.size()) {
            result.push_back(m_lines[i]);
        } else {
            result.push_back(nullptr);
        }
    }
    return result;
}

Line::Line(const QJsonObject &json) {
    m_assoc = nullptr;
    m_styles = std::make_shared<QVector<StyleSpan>>();
    m_cursor = std::make_shared<QVector<int>>();

    if (json["text"].isString()) {
        m_text = json["text"].toString();
        if (json.contains("cursor")) {
            auto jsonCursors = json["cursor"].toArray();
            for (auto jsonCursor : jsonCursors) {
                m_cursor->append(jsonCursor.toInt());
            }
        }
        if (json.contains("styles")) {
            auto jsonStyles = json["styles"].toArray();
            m_styles = StyleSpan::styles(jsonStyles, m_text);
        } else {
            m_styles->clear();
        }
    }
}

// std::shared_ptr<Line> line
// const Line *line
Line::Line(std::shared_ptr<Line> line, const QJsonObject &json) {
    if (!line) { return; }
    m_assoc = nullptr;
    m_text = line->m_text;
    if (json.contains("cursor")) {
        m_cursor = std::make_shared<QVector<int>>();
        auto jsonCursors = json["cursor"].toArray();
        for (auto jsonCursor : jsonCursors) {
            m_cursor->append(jsonCursor.toInt());
        }
    } else {
        m_cursor = line->m_cursor;
    }
    if (json.contains("styles")) {
        auto jsonStyles = json["styles"].toArray();
        m_styles = StyleSpan::styles(jsonStyles, m_text);
    } else {
        m_styles = line->m_styles;
    }
}

Line &Line::operator=(const Line &line) {
    if (this == &line) {
        return *this;
    }
    m_text = line.m_text;
    m_cursor = line.m_cursor;
    m_styles = line.m_styles;
    m_assoc = line.m_assoc;
    return *this;
}

} // namespace xi
