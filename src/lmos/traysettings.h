#ifndef TRAYSETTINGS_H
#define TRAYSETTINGS_H

#include <QDialog>

namespace Ui {
class traysettings;
}

class traysettings : public QDialog {
	Q_OBJECT

      public:
	explicit traysettings(QWidget *parent = nullptr);
	~traysettings();

      public slots:
	void storesettings();

      private:
	Ui::traysettings *ui;
};

#endif	// TRAYSETTINGS_H
