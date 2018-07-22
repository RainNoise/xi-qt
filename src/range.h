#ifndef RANGE_H
#define RANGE_H

#include <QPair>


namespace xi {

// A half-open range representing lines in a document.
template <typename T>
class Range
{
public:
	Range(T start = 0, T end = 0) {
		m_pair.first = start;
		m_pair.second = end;
	}

	inline T start() const {
		return m_pair.first;
	}
	inline T end() const {
		return m_pair.second;
	}

private:
	QPair<T, T> m_pair;
};

using RangeI = Range<qint64>;
using RangeF = Range<qreal>;

} // xi

#endif // RANGE_H
