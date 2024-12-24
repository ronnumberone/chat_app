#include "GroupChatDialog.h"
#include "ui_GroupChatDialog.h"
#include <QCheckBox>

GroupChatDialog::GroupChatDialog(QWidget *parent, const QStringList &members)
    : QDialog(parent)
    , ui(new Ui::GroupChatDialog)
{
    ui->setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout(ui->scrollArea);
    for (const QString &member : members) {
        QCheckBox *checkBox = new QCheckBox(member, this);
        layout->addWidget(checkBox);
        memberCheckBoxes.append(checkBox);

        connect(checkBox, &QCheckBox::stateChanged, this, [this]() {
            for (const QCheckBox *cb : memberCheckBoxes) {
                if (cb->isChecked()) {
                    ui->scrollArea->setStyleSheet("border: none");
                    break;
                }
            }
        });
    }
    ui->scrollArea->setLayout(layout);
}

GroupChatDialog::~GroupChatDialog()
{
    delete ui;
}

QString GroupChatDialog::getGroupName() const
{
    return ui->nameGroupEdit->text().trimmed();
}

QStringList GroupChatDialog::getSelectedMembers() const
{
    QStringList selectedMembers;

    for (const QCheckBox *checkBox : memberCheckBoxes) {
        if (checkBox->isChecked()) {
            selectedMembers.append(checkBox->text());
        }
    }

    return selectedMembers;
}

void GroupChatDialog::on_confirmBtn_clicked()
{
    bool isValid = true;

    if (ui->nameGroupEdit->text().trimmed().isEmpty()) {
        ui->nameGroupEdit->setStyleSheet("border: 2px solid red;");
        isValid = false;
    }

    bool hasChecked = false;
    for (const QCheckBox *checkBox : memberCheckBoxes) {
        if (checkBox->isChecked()) {
            hasChecked = true;
            break;
        }
    }

    if (!hasChecked) {
        ui->scrollArea->setStyleSheet("border: 1px solid red;");
        isValid = false;
    }

    if (isValid) {
        accept();
    }
}

void GroupChatDialog::on_cancelBtn_clicked()
{
    reject();
}

void GroupChatDialog::on_nameGroupEdit_textChanged(const QString &text)
{
    if(!text.trimmed().isEmpty()) {
        ui->nameGroupEdit->setStyleSheet("border: none;");
    }
}

