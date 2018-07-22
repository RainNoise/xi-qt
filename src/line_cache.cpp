#include "line_cache.h"

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

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
	auto newInvalidBefore = 0;
	std::vector<std::shared_ptr<Line>> newLines;
	auto newInvalidAfter = 0;
	auto oldIdx = 0;

	auto ops = json["ops"].toArray();
	for (auto opref : ops) {
		auto op = opref.toObject();
		auto opTypeStr = op["op"].toString();
		qDebug() << "update--> op type: " << opTypeStr;
		auto opType = CoreConnection::ops(opTypeStr);
		qint32 n = op["n"].toInt();
		switch (opType) {
		case CoreConnection::Ops_Invalidate:
		{
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
		}
		break;
		case CoreConnection::Ops_Ins:
		{
			for (int i = 0; i < newInvalidAfter; ++i) {
				newLines.push_back(nullptr);
			}
			newInvalidAfter = 0;
			QJsonArray jsonLines = op["lines"].toArray();
			for (auto jsonLine : jsonLines) {
				newLines.push_back(std::make_shared<Line>(jsonLine.toObject()));
			}
		}
		break;
		case CoreConnection::Ops_Copy:
		case CoreConnection::Ops_Update:
		{
			auto nRemaining = n;
			if (oldIdx < m_invalidBefore) {
				auto nInvalid = std::min(n, m_invalidBefore - oldIdx);
				if (newLines.size() == 0) {
					newInvalidBefore += nInvalid;
				} else {
					newInvalidAfter += nInvalid;
				}
				oldIdx += nInvalid;
				nRemaining -= nInvalid;
			}
			if (nRemaining > 0 && oldIdx < m_invalidBefore + m_lines.size()) {
				for (auto i = 0; i < newInvalidAfter; ++i) {
					newLines.push_back(nullptr);
				}
				newInvalidAfter = 0;
				auto nCopy = std::min(nRemaining, m_invalidBefore + (qint32)m_lines.size() - oldIdx);
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
						newLines.push_back(std::make_shared<Line>(m_lines[ix].get(), jsonLines[ix].toObject()));
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
		}
		break;
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

Line::Line(const QJsonObject &json) {
	m_styles = std::make_shared<QVector<StyleSpan>>();
	m_cursor = std::make_shared<QVector<qint32>>();

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

Line::Line(const Line* line, const QJsonObject &json) {
	if (!line) {
		return;
	}
	m_text = line->m_text;
	if (json.contains("cursor")) {
		m_cursor = std::make_shared<QVector<qint32>>();
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

} // xi
