#include "RecordItemWidget.h"
#include "ui_RecordItemWidget.h"

#include <QTime>
#include <QDebug>
#include <QTemporaryFile>

RecordItemWidget::RecordItemWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RecordItemWidget)
{
    ui->setupUi(this);
    mediaPlayer = new QMediaPlayer(this);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        ui->timeLb->setText(formatTime(duration));
    });

    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        qint64 remainingTime = mediaPlayer->duration() - position;
        ui->timeLb->setText(formatTime(remainingTime));
    });

    connect(mediaPlayer, &QMediaPlayer::stateChanged, this, [this](QMediaPlayer::State state) {
        if (state == QMediaPlayer::StoppedState) {
            ui->timeLb->setText(formatTime(mediaPlayer->duration())); // Reset lại thời gian
            ui->playBtn->setIcon(QIcon(":/icons/play.png"));
            mediaPlayer->setPosition(0);
        }
    });
}

RecordItemWidget::~RecordItemWidget()
{
    delete ui;
}

void RecordItemWidget::setRecordInfo(QString sender, QByteArray fileData, bool isMyFile)
{
    QTemporaryFile tempFile;
    if (tempFile.open()) {
        tempFile.write(fileData);
        tempFile.flush();
        tempFile.close();

        mediaPlayer->setMedia(QUrl::fromLocalFile(tempFile.fileName()));
    }

    if(isMyFile)
    {
        ui->horizontalLayout->removeItem(ui->verticalLayout);
        ui->horizontalLayout->insertLayout(0, ui->verticalLayout);
        ui->horizontalLayout->setStretch(0, 7);
        ui->nameLb->setText("You, " + QTime::currentTime().toString("HH:mm"));
        ui->nameLb->setAlignment(Qt::AlignRight);
        ui->timeLb->setAlignment(Qt::AlignRight);
    }
    else
    {
        ui->nameLb->setText(sender + ", " + QTime::currentTime().toString("HH:mm"));
    }
}

void RecordItemWidget::playRecord(QByteArray fileData)
{
    QTemporaryFile tempFile;
    if (tempFile.open()) {
        tempFile.write(fileData);
        tempFile.flush();
        tempFile.close();

        mediaPlayer->setMedia(QUrl::fromLocalFile(tempFile.fileName()));
        mediaPlayer->play();
    }
}

void RecordItemWidget::on_playBtn_clicked()
{
    if (mediaPlayer->state() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
        ui->playBtn->setIcon(QIcon(":/icons/play.png"));
    } else {
        mediaPlayer->play();
        ui->playBtn->setIcon(QIcon(":/icons/pause.png"));
    }
}

QString RecordItemWidget::formatTime(qint64 timeMs) const {
    int seconds = (timeMs / 1000) % 60;
    int minutes = (timeMs / 1000) / 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

