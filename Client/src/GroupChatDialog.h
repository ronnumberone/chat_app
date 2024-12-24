#ifndef GROUPCHATDIALOG_H
#define GROUPCHATDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QCheckBox>

namespace Ui {
class GroupChatDialog;
}

class GroupChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GroupChatDialog(QWidget *parent = nullptr, const QStringList &members = {});
    ~GroupChatDialog();

    QString getGroupName() const;
    QStringList getSelectedMembers() const;

private slots:
    void on_confirmBtn_clicked();

    void on_nameGroupEdit_textChanged(const QString &text);

    void on_cancelBtn_clicked();

private:
    Ui::GroupChatDialog *ui;
    QList<QCheckBox*> memberCheckBoxes;
};

#endif // GROUPCHATDIALOG_H
