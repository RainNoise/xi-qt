#include "shortcuts.h"

#include <QMap>
#include <QPair>
#include <QVector>


namespace xi {

using Item = QPair<QKeySequence, QShortcut*>;
using Bundle = QVector<Item>;
using Seq = QMap<QWidget*, Bundle>;

static Seq sequences;

Shortcuts::~Shortcuts() {
}

Shortcuts* Shortcuts::instance() {
	static Shortcuts instance;
	return &instance;
}

void Shortcuts::append(QWidget *widget, const QKeySequence &seq, CallbackType callback) {
	auto shortcut = new QShortcut(seq, widget);
	if (callback) callback(shortcut);
	auto &bundle = sequences[widget];
	bundle.append(Item(seq, shortcut));
}

void Shortcuts::erase(QWidget *widget) {
	auto &bundle = sequences[widget];
	for( auto i = bundle.cbegin(), e = bundle.cend(); i != e; ++i) {
		delete i->second;
	}
	sequences.erase(sequences.find(widget));
}

HasShortcuts::~HasShortcuts() {
	Shortcuts::instance()->erase(m_toErase);
}

} // xi
