#include "ChatItemWidget.h"
#include "ui_ChatItemWidget.h"
#include "ChatProtocol.h"
#include <QTime>

ChatItemWidget::ChatItemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatItemWidget)
{
    ui->setupUi(this);
}

ChatItemWidget::~ChatItemWidget()
{
    delete ui;
}

void ChatItemWidget::setMessage(QString message, QString sender, bool isMyMessage)
{
    ui->lblMessage->setText(message);
    ui->lblSender->setText(sender + ", " + QTime::currentTime().toString("HH:mm"));
    if(isMyMessage) {
        ui->lblMessage->setAlignment(Qt::AlignRight);
        ui->lblSender->setAlignment(Qt::AlignRight);
        ui->lblSender->setText("You, " + QTime::currentTime().toString("HH:mm"));
    }
}
