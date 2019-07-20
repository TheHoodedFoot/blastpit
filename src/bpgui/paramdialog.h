#ifndef PARAMDIALOG_H
#define PARAMDIALOG_H

#include <QDialog>

namespace Ui {
class paramDialog;
}

class ParamDialog : public QDialog {
	Q_OBJECT

      public:
	explicit ParamDialog(QWidget *parent = 0);
	~ParamDialog();

      private:
	Ui::paramDialog *ui;
};
#endif  // PARAMDIALOG_H
