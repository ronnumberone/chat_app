#include "FileItemWidget.h"
#include "ui_FileItemWidget.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTime>

FileItemWidget::FileItemWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileItemWidget)
{
    ui->setupUi(this);
    ui->fileButton->setIcon(QIcon(":/icons/file.png"));
    ui->fileButton->setIconSize(QSize(50, 50));
    ui->fileButton->setFixedSize(QSize(50,50));
}

FileItemWidget::~FileItemWidget()
{
    delete ui;
}

void FileItemWidget::setFileInfo(QString sender, QString fileName, qint64 fileSize, QByteArray fileData, bool isMyFile)
{
    _fileName = fileName;
    _fileData = fileData;
    ui->fileNameLb->setText(fileName);
    ui->fileSizeLb->setText(formatFileSize(fileSize));
    ui->namelb->setText(sender + ", " + QTime::currentTime().toString("HH:mm"));
    if(isMyFile) {
        ui->namelb->setAlignment(Qt::AlignRight);
        ui->namelb->setText("You, " + QTime::currentTime().toString("HH:mm"));
    }
}

void FileItemWidget::on_fileButton_clicked()
{
    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString savePath = QDir(downloadsPath).filePath(_fileName);

    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(_fileData);
        file.close();
        QMessageBox::information(this, "Download Complete", "File has been downloaded successfully to " + savePath);
    } else {
        QMessageBox::warning(this, "Error", "Could not save the file.");
    }
}

QString FileItemWidget::formatFileSize(qint64 size) {
    if (size < 1024)
        return QString::number(size) + " B";
    else if (size < 1024 * 1024)
        return QString::number(size / 1024.0, 'f', 2) + " KB";
    else if (size < 1024 * 1024 * 1024)
        return QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
    else
        return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

