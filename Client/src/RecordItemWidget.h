#ifndef RECORDITEMWIDGET_H
#define RECORDITEMWIDGET_H

#include <QWidget>
#include <QAudioOutput>
#include <QBuffer>
#include <QTimer>
#include <QMediaPlayer>

namespace Ui {
class RecordItemWidget;
}

class RecordItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecordItemWidget(QWidget *parent = nullptr);
    ~RecordItemWidget();
    void setRecordInfo(QString sender, QByteArray fileData, bool isMyFile = false);
    void playRecord(QByteArray fileData);

private slots:
    void on_playBtn_clicked();

private:
    Ui::RecordItemWidget *ui;
    QString formatTime(qint64 timeMs) const;
    QAudioOutput *audioOutput;
    QBuffer *buffer;
    QTimer *timer;
    int duration;
    int remainingTime;
    QByteArray _fileData;

    bool isPlaying = false;
    QMediaPlayer* mediaPlayer;
};

#endif // RECORDITEMWIDGET_H
