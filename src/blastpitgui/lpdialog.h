#ifndef LPDIALOG_H
#define LPDIALOG_H

#include <QDialog>

namespace Ui {
class lpDialog;
}

class LpDialog : public QDialog {
	Q_OBJECT

      public:
	explicit LpDialog(QWidget *parent = 0);
	~LpDialog();

      private:
	Ui::lpDialog *ui;
};
#endif  // LPDIALOG_H
