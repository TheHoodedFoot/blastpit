#include "lpdialog.h"
#include "ui_lpdialog.h"

LpDialog::LpDialog(QWidget *parent) : QDialog(parent), ui(new Ui::lpDialog)
{
	ui->setupUi(this);
}

LpDialog::~LpDialog() { delete ui; }
