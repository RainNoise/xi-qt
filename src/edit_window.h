#ifndef EDITWINDOW_H
#define EDITWINDOW_H

#include <QTabWidget>
#include <QHash>

#include <memory>

#include "core_connection.h"
#include "file.h"


namespace xi {

class EditView;

// Tab
class EditWindow : public QTabWidget
{
    Q_OBJECT
public:
    explicit EditWindow(QWidget *parent = nullptr);

    void init(const std::shared_ptr<CoreConnection>& connection);

	void openFile(const QString& viewId, const QString& filePath);
	void openFile();
	void closeFile(const QString& viewId);
	void saveFile(bool saveAs = false);
	void saveAsFile();

    int insertViewTab(int idx, const std::shared_ptr<File>& file, QWidget *view);
	int appendViewTab(const std::shared_ptr<File>& file, QWidget *view);
    void removeViewTab(int idx);

	void resizeEvent(QResizeEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *e) override;

	int find(const QString& viewId, const QString& filePath);

    inline EditView *tab(int idx);
	void newTab();
	void newTabWithOpenFile();
	void closeCurrentTab();
	void saveCurrentTab();
	void saveAllTab();

	void setupShortcuts();
	void setupCoreHandler();

signals:

public slots:
	void updateHandler(const QString &viewId, const QJsonObject &update);
	void scrollHandler(const QString &viewId, int line, int column);
	void pluginStartedHandler(const QString &viewId, const QString &pluginName);
	void pluginStoppedHandler(const QString &viewId, const QString &pluginName);
	void availablePluginsHandler(const QString &viewId, const QVector<QJsonObject> &plugins);
	void updateCommandsHandler(const QString &viewId, const QVector<QString> &commands);
	void configChangedHandler(const QString &viewId, const QJsonObject &changes);
	void defineStyleHandler(const QJsonObject &params);
	void availableThemesHandler(const QVector<QString> &themes);
	void themeChangedHandler(const QString &name, const Theme &theme);
	void alertHandler(const QString &text);

	void rpcResponseFinishHandler(const QJsonObject &json);

private:
    std::shared_ptr<CoreConnection> m_connection;
	QHash<QString, QWidget*> m_router;
};

} // xi

#endif // EDITWINDOW_H
