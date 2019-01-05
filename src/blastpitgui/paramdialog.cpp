#include "paramdialog.h"
#include "ui_paramdialog.h"

ParamDialog::ParamDialog(QWidget *parent)
	: QDialog(parent), ui(new Ui::paramDialog)
{
	ui->setupUi(this);
}

ParamDialog::~ParamDialog() { delete ui; }
