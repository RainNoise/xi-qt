#ifndef XIFILE_H
#define XIFILE_H

#include <QString>
#include <QVector>

namespace xi {

//struct FileInfo {
//	QString path;
//	QString name;
//	QString viewId;
//	QWidget* widget;
//	bool memory;
//};

//class RecentFileList {
//
//};

// database??
// FileBase::shared()->create("");
//

//class FileBase {
//public:
//	static FileBase* shared();
//
//	File open(const QString& path);
//
//	void close();
//
//	void update(const File& file);
//
//private:
//	QVector<File> m_files;
//
//	FileBase();
//};

//class FileInfoManager {
//
//};

class File {
public:
    File();
    File(const File &file);

    void setPath(const QString &path);
    void setViewId(const QString &viewId);
    void setTempName(const QString &name);

    QString viewId() const;
    QString path() const;
    QString name() const;
    QString tempName() const;
    QString displayName() const;

private:
    QString m_path;
    QString m_viewId;
    QString m_tempName; //new file save dialog
};

} // namespace xi

#endif // FILE_H