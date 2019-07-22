#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QMainWindow>
#include <QSystemTrayIcon>

//#include "bplmos.h"
#include "lmos.hpp"

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class LmosTray : public QMainWindow {
	Q_OBJECT

      public:
	explicit LmosTray(QWidget *parent = 0);
	~LmosTray();

      public slots:
	void setTrayBalloon(QString);
	// void loadgeos( size_t size, char *data );
	void log(QString entry);
	void alert(const QString &name, int argc, void *argv);
	void log(int, const char *, QString);
	void retval(const char *function, bool value);
	void retval(const char *function, int value);

      signals:
	void bptest();

      protected:
	void closeEvent(QCloseEvent *event);

      private slots:
	void setIcon();
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
	// void test();
	void clearLog();
	void showSettings();

      private:
	void createActions();
	void initTray();

	Ui::MainWindow *ui;

	QAction *minimizeAction;
	QAction *restoreAction;
	QAction *quitAction;
	QAction *clearLogAction;
	QAction *settingsAction;

	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;

	// LMOS *lwrap;

	// void createLMOS();
	// void testLMOS();
	// void destroyLMOS();

	void listen();
};
#endif  // MAINWINDOW_H
