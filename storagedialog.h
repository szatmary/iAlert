#ifndef STORAGEDIALOG_H
#define STORAGEDIALOG_H

#include <QDialog>

namespace Ui {
    class StorageDialog;
}

class StorageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StorageDialog(QWidget *parent = 0);
    ~StorageDialog();

private:
    Ui::StorageDialog *ui;
};

#endif // STORAGEDIALOG_H
