#pragma once

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "ui_CrystalVolume.h"

#include "CrystalProfile.h"

class CrystalVolume : public QMainWindow
{
	Q_OBJECT

	class Config {
	public:
		enum {
			MinimumWidth = 200, 
			MiniMunHeight = 80,
			
			TabBarOffsetMaximumWidth = -45,
			AddButtonOffsetX = -4, 
			BorderWidth = 4
		};
	};

public:
	CrystalVolume(QWidget *parent = nullptr);
	~CrystalVolume();

	bool createExplorerTab();
	bool createExplorerTab(const QString& tabDir);
	void setCurrentDir(HWND explorerHwnd);

public slots:
	void onCreateExplorerWindowInTabThreadHwndSignal(HWND handle);
	void onCreateExplorerTabSignal(const QString& tabDir);
	void onTabAllClosedSignal();

protected:
	void resizeEvent(QResizeEvent* _event) override;
	void closeEvent(QCloseEvent* _event) override;

	// use this move window system is smoother than native, but why?
	void mousePressEvent(QMouseEvent* _event) override;
	void mouseMoveEvent(QMouseEvent* _event) override;
	void mouseReleaseEvent(QMouseEvent* _event) override;
	void mouseDoubleClickEvent(QMouseEvent* _event) override;

	void keyPressEvent(QKeyEvent* _event) override;

	void dragEnterEvent(QDragEnterEvent* _event) override;
	void dropEvent(QDropEvent* _event) override;

protected slots:
	void onClickedPushButtonAddTab();

	void onUiTimer();

private:
	void initProfile();
	void saveProfile();

	Ui::CrystalVolumeClass ui;
	QTimer uiTimer;

	// this variable for multi-thread
	std::atomic<bool> explorerTabThreadAlreadly = true;

	// for move window
	bool moveEnabled = false;
	QPoint relativePressedPos;

	// for drag window borders size
	bool editBorderREnabled = false; 
	bool editBorderLEnabled = false; 
	bool editBorderTEnabled = false; 
	bool editBorderBEnabled = false;

	// for half-screen recognize
	bool halfScreenState = false;
	QSize standardSize;
	
	QString defaultExplorerTabPath;
	QStringList wattingExplorerTabPaths;

	CrystalProfile profile;
};
