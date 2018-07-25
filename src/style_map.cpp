#include "style_map.h"

#include <QJsonArray>

#include "perference.h"

namespace xi {

std::shared_ptr<QVector<StyleSpan>> StyleSpan::styles(const QJsonArray &json, const QString &text) {
    auto vss = std::make_shared<QVector<StyleSpan>>();
    auto ix = 0;
    for (auto i = 0; i < json.size(); i += 3) {
        auto start = ix + json.at(i).toInt();
        auto end = start + json.at(i + 1).toInt();
        auto style = start + json.at(i + 2).toInt();
        vss->append(StyleSpan(style, RangeI(start, end - start)));
        ix = end;
    }
    return vss;
}

QColor colorFromArgb(quint32 argb) {
    return QColor::fromRgbF(
        qreal((argb >> 16) & 0xff) * 1.0 / 255,
        qreal((argb >> 8) & 0xff) * 1.0 / 255,
        qreal(argb & 0xff) * 1.0 / 255,
        qreal((argb >> 24) & 0xff) * 1.0 / 255);
}

void StyleMap::defStyle(const QJsonObject &json) {
    auto theme = Perference::shared()->theme();
    QColor fgColor(QColor::Invalid);
    QColor bgColor(QColor::Invalid);

    auto styleId = json["id"].toInt();
    if (json.contains("fg_color")) {
        fgColor = colorFromArgb(json["fg_color"].toVariant().toULongLong());
    } else {
        fgColor = theme.foreground();
    }

    if (json.contains("bg_color")) {
        bgColor = colorFromArgb(json["bg_color"].toInt());
    }

    auto underline = false;
    auto italic = false;
    auto weight = 0;

    if (json.contains("underline")) {
        underline = json["underline"].toBool();
    }
    if (json.contains("italic")) {
        italic = json["italic"].toBool();
    }
    if (json.contains("weight")) {
        weight = json["weight"].toInt();
    }

    auto style = std::make_shared<Style>();

    while (m_styles.count() < styleId) {
        m_styles.append(nullptr);
    }
    if (m_styles.count() == styleId) {
        m_styles.append(style);
    } else {
        m_styles[styleId] = style;
    }
}

} // namespace xi
