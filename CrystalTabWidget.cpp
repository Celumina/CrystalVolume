#include "CrystalTabWidget.h"

#include <QFile>

#include "CrystalWindowsLibrary.h"

CrystalTabWidget::CrystalTabWidget(QWidget* parent) : 
QTabWidget(parent){
	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(onCurrentChanged(int)));
}

int CrystalTabWidget::addContainerTab(QWidget* widget, const QString& tabName, HWND hwnd) {
	widget->setAttribute(Qt::WA_DeleteOnClose, true);
	if (hwnd) {
		hwndMap[widget] = hwnd;
		return QTabWidget::addTab(widget, tabName);
	}
	else {
		widget->close();
		return currentIndex();
	}
	
}

int CrystalTabWidget::addExplorerTab(QWidget* widget, HWND hwnd) {
	return addContainerTab(widget, GetExplorerFolderName(hwnd), hwnd);
}


void CrystalTabWidget::removeContainerTab(int index) {
	SendMessage(hwnd(index), WM_CLOSE, 0, 0);
	// DEV: Yes! is these windows make delay
		// OMG: HWND, PID and Handle these all different.
	DWORD explorerPid = 0;
	GetWindowThreadProcessId(hwnd(index), &explorerPid);
	TerminateProcess(OpenProcess(PROCESS_ALL_ACCESS, 0, explorerPid), 0);

	// if (widget(index)->objectName() != "DONT_DESTROY") {
	// }

	widget(index)->close();
	hwndMap.erase(widget(index));

	QTabWidget::removeTab(index);

	// last no-empty tab will be closed
	if (count() == 0) {
		emit tabAllClosedSignal();
	}
}

const QString& CrystalTabWidget::configFilePath() {
	return historyPath;
}

HWND CrystalTabWidget::hwnd(int index) {
	return hwndMap[widget(index)];
}

void CrystalTabWidget::resizeEvent(QResizeEvent* _event) {
	QTabWidget::resizeEvent(_event);	

	// solving the graphic ruin, but expensive.
	// not indeed because magic bar was cover it!
	//int index = currentIndex();
	//setCurrentIndex(0);
	//repaint();
	//setCurrentIndex(index);
}

void CrystalTabWidget::closeEvent(QCloseEvent* _event) {
	// CONFIG: Out ConfigPath 
	auto configFile = QFile(historyPath);
	configFile.open(QIODevice::WriteOnly);
	QString tabTexts;
	while (count()) {
		// Filtting text which is not a path.
		auto dir = GetExplorerDir(hwnd(0));
		if (dir.length() > 1 && dir[1] == ':') {
			tabTexts +=  dir + '\n';
		}
		removeContainerTab(0);
	}
	configFile.write(tabTexts.toUtf8().data());
	configFile.close();
	QTabWidget::closeEvent(_event);
}

void CrystalTabWidget::onCurrentChanged(int index) {

}

void CrystalTabWidget::onTabCloseRequested(int index) {
	removeContainerTab(index);
}