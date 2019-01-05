#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHostAddress>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include "pugixml.hpp"

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

class BlastpitGui : public QMainWindow {
	Q_OBJECT

      public:
	explicit BlastpitGui(QWidget *parent = 0);
	~BlastpitGui();

      private:
	Ui::MainWindow *ui;

      public slots:
	void UpdateDial(int x, int y, int z, int rx, int ry, int rz);
	void UpdateButton(int button, bool state);
	void SetTrayBalloon(QString, QString);
	void Parse(int command, struct BpPacket *netbuf);
	void Test();
	void ToggleDoor();
	void ToggleLight();
	void ToggleRun();
	/*! \brief Generate positioning help from marking objects
	 *
	 * Converts the geometry database into custom
	 * objects optimised for display using the positioning help laser
	 */
	void GeneratePosHelp();

      protected:
	void closeEvent(QCloseEvent *event);

      private slots:
	void SetIcon();
	void IconActivated(QSystemTrayIcon::ActivationReason reason);

      public:
      private:
	void CreateActions();
	void InitTray();

	void Listen();
	void NuLOOQ();

	int LoadSVG(struct BpPacket *netbuf);
	int SendXML(QHostAddress, pugi::xml_document *);

	QAction *minimizeAction;
	QAction *restoreAction;
	QAction *quitAction;
	QAction *testAction;

	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;
};
#endif  // MAINWINDOW_H
