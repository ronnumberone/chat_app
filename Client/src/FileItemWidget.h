#ifndef FILEITEMWIDGET_H
#define FILEITEMWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

namespace Ui {
class FileItemWidget;
}

class FileItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileItemWidget(QWidget *parent = nullptr);
    ~FileItemWidget();
    void setFileInfo(QString sender, QString fileName, qint64 fileSize, QByteArray fileData, bool isMyFile = false);

private slots:
    void on_fileBtn_clicked();

private:
    QString formatFileSize(qint64 size);
    Ui::FileItemWidget *ui;
    QString _fileName;
    QByteArray _fileData;
};

#endif // FILEITEMWIDGET_H
