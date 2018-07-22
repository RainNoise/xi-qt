#include "style_map.h"

#include <QJsonArray>


namespace xi {

StyleSpan::StyleSpan(StyleIdentifier style, RangeI range) :	m_style(style), m_range(range) {
}

StyleSpan::StyleSpan() : m_style(-1) {

}

std::shared_ptr<QVector<StyleSpan>> StyleSpan::styles(const QJsonArray& json, const QString& text) {
	auto vss = std::make_shared<QVector<StyleSpan>>();
	auto ix = 0;
	for (auto i = 0; i < json.size(); i+=3) {
		auto start = ix + json.at(i).toInt();
		auto end = start + json.at(i + 1).toInt();
		auto style = start + json.at(i + 2).toInt();
		vss->append(StyleSpan(style, RangeI(start, end - start)));
		ix = end;
	}
	return vss;
}

} // xi
