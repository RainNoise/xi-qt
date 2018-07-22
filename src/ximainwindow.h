#ifndef XIMAINWINDOW_H
#define XIMAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

#include <memory>

#include "core_connection.h"
#include "edit_window.h"


namespace xi {

// Start
class XiMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit XiMainWindow(QWidget *parent = 0);
    ~XiMainWindow();
	virtual void keyPressEvent(QKeyEvent *e) override;

private:
    void setupUI();
    void setupCore();
	void setupEditWindow();
	void setupStartPage();
	QString defaultConfigDirectory();

private:
    std::shared_ptr<CoreConnection> m_coreConnection;
    EditWindow* m_editWindow;
	QStackedWidget m_stack;
};

} // xi

#endif // XIMAINWINDOW_H
