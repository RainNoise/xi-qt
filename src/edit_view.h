#ifndef EDITTEXTAREA_H
#define EDITTEXTAREA_H

#include <QWidget>

#include <memory>

#include "core_connection.h"
#include "file.h"
#include "font.h"


class QScrollBar;

namespace xi {

class EditWindow;
class ContentView;

// Containner
class EditView : public QWidget {
    Q_OBJECT
public:
    explicit EditView(const std::shared_ptr<File>& file,
		const std::shared_ptr<CoreConnection>& connection, 
		QWidget *parent = nullptr);

	void updateHandler(const QJsonObject &json);
	void scrollHandler(int line, int column);
	void themeChangedHandler(const QString &name, const Theme &theme);

	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *e) override;
	virtual void wheelEvent(QWheelEvent *event) override;

	void relayoutScrollBar();
	std::shared_ptr<File> getFile() const;

	void focusOnEdit();

public slots:
	void scrollBarVChanged(int y);
	void scrollBarHChanged(int x);

private:
	EditWindow* m_editWindow;
	ContentView *m_content;
	QScrollBar *m_scrollBarV;
	QScrollBar *m_scrollBarH;
};

class GutterWidget : public QWidget {
public:

private:
};

} // xi

#endif // EDITTEXTAREA_H
