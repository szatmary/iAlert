#include "storagedialog.h"
#include "ui_storagedialog.h"

StorageDialog::StorageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StorageDialog)
{
    ui->setupUi(this);
}

StorageDialog::~StorageDialog()
{
    delete ui;
}
