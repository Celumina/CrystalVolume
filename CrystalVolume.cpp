#include "CrystalVolume.h"

#include <QWindow>
#include <QMouseEvent>
#include <QTabBar>
#include <QFile>
#include <QDir>
#include <QMimeData>

#include "CrystalThreadLibrary.h"
#include "CrystalWindowsLibrary.h"

// DEV: History Tab record and reopen

// DEV: drag delay while lot of window were opened...
// DEV: maybe can sleep some tab if not used (Limit Active Native Window Count).

CrystalVolume::CrystalVolume(QWidget *parent)
	: QMainWindow(parent), 
	defaultExplorerTabPath("C:\\") {
	ui.setupUi(this);
	initProfile();
	// move screen to center
	auto screenRes = QGuiApplication::primaryScreen()->size();
	move((screenRes.width() - width()) / 2, (screenRes.height() - height()) / 2);

	// after UI requirement.
	standardSize = size();

	setWindowIcon(QIcon(":/CrystalVolume/Design/crystalVolume.ico"));

	setWindowFlags(Qt::FramelessWindowHint);
	setMinimumSize(Config::MinimumWidth, Config::MiniMunHeight);

	// Magic Bar!
	SetWindowPos(reinterpret_cast<HWND>(ui.labelBar->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	// register type for connect
	qRegisterMetaType<HWND>("HWND");

	connect(&uiTimer, SIGNAL(timeout()), this, SLOT(onUiTimer()));
	connect(ui.tabWidgetMain, SIGNAL(tabAllClosedSignal()), this, SLOT(onTabAllClosedSignal()));
	connect(ui.lineEditCommand, SIGNAL(createExplorerTabSignal(const QString&)), this, SLOT(onCreateExplorerTabSignal(const QString&)));

	setAcceptDrops(true);

	uiTimer.setInterval(500);
	uiTimer.start();

	QFile configFile(ui.tabWidgetMain->configFilePath());
	if (!configFile.open(QIODevice::ReadOnly)) {
		createExplorerTab(defaultExplorerTabPath);
		return;
	}
	QTextStream configStream(&configFile);
	while (true) {
		auto line = configStream.readLine();
		if (line.isNull()) {
			break;
		}
		if (QDir(line).exists()) {
			wattingExplorerTabPaths.push_back(line);
		}
	}
	if (wattingExplorerTabPaths.isEmpty()) {
		wattingExplorerTabPaths.push_back(defaultExplorerTabPath);
	}
	createExplorerTab();
}

CrystalVolume::~CrystalVolume() {

}

bool CrystalVolume::createExplorerTab() {
	if (explorerTabThreadAlreadly && !wattingExplorerTabPaths.isEmpty()) {
		explorerTabThreadAlreadly = false;
		auto thread = new CreateExplorerTabThread(wattingExplorerTabPaths.first());
		connect(thread, SIGNAL(hwndSignal(HWND)), this, SLOT(onCreateExplorerWindowInTabThreadHwndSignal(HWND)));
		thread->start();
		return true;
	}
	return false;
}

bool CrystalVolume::createExplorerTab(const QString& tabPath) {
	wattingExplorerTabPaths.push_back(tabPath);
	return createExplorerTab();
}

void CrystalVolume::setCurrentDir(HWND explorerHwnd) {
	if (QDir::setCurrent(GetExplorerDir(explorerHwnd))) {
		ui.lineEditCommand->setPlaceholderText("CurrentDir >_");
	}
	else {
		QDir::setCurrent(QCoreApplication::applicationDirPath());
		ui.lineEditCommand->setPlaceholderText(">_");
	}
}

void CrystalVolume::onCreateExplorerTabSignal(const QString& tabDir) {
	createExplorerTab(tabDir);
}

void CrystalVolume::onTabAllClosedSignal() {
	close();
}

void CrystalVolume::resizeEvent(QResizeEvent* _event) {
	ui.tabWidgetMain->setGeometry(
		Config::BorderWidth,
		Config::BorderWidth, 
		width() - Config::BorderWidth * 2, 
		height() - Config::BorderWidth * 2
	);
	ui.labelBar->setGeometry(
		Config::BorderWidth,
		ui.labelBar->y(),
		width() - Config::BorderWidth * 2,
		ui.labelBar->height()
	);
	ui.tabWidgetMain->tabBar()->setMaximumWidth(width() + Config::TabBarOffsetMaximumWidth - Config::BorderWidth);

	ui.pushButtonAddTab->setGeometry(
		width() - ui.pushButtonAddTab->width() + Config::AddButtonOffsetX - Config::BorderWidth,
		ui.pushButtonAddTab->y(),
		ui.pushButtonAddTab->width(),
		ui.pushButtonAddTab->height()
		);

	ui.lineEditCommand->setGeometry(
		width() / 4 + Config::BorderWidth,
		ui.lineEditCommand->y(),
		width() / 2 - Config::BorderWidth * 2,
		ui.lineEditCommand->height()
	);

	QMainWindow::resizeEvent(_event);
}

void CrystalVolume::closeEvent(QCloseEvent* _event) {
	// restore current path for save history
	QDir::setCurrent(QCoreApplication::applicationDirPath());
	ui.tabWidgetMain->close();

	saveProfile();
	QMainWindow::closeEvent(_event);
}


// DEV : add drag window size
void CrystalVolume::mousePressEvent(QMouseEvent* mouseEvent) {
	if (mouseEvent->button() == Qt::LeftButton) {
		// right side
		if (width() + x() - mouseEvent->globalPos().x() <= Config::BorderWidth) {
			editBorderREnabled = true;
		}
		// bottom side
		if (height() + y() - mouseEvent->globalPos().y() <= Config::BorderWidth) {
			editBorderBEnabled = true;
		}
		// left side
		if (mouseEvent->globalPos().x() - x() <= Config::BorderWidth) {
			editBorderLEnabled = true;
		}
		// top side
		if (mouseEvent->globalPos().y() - y() <= Config::BorderWidth) {
			editBorderTEnabled = true;
		}

		// move window reg
		relativePressedPos = mouseEvent->globalPos() - pos();
		moveEnabled = true;
	}
	QMainWindow::mousePressEvent(mouseEvent);
}

// don' t use mouseEvent->pos(), it return a position which is unstable.
void CrystalVolume::mouseMoveEvent(QMouseEvent* mouseEvent) {
	// DEV:: Cursor Shape, release for reset
	// right side
	//	setCursor(Qt::SizeHorCursor);
	//	setCursor(Qt::ArrowCursor);

	if (editBorderREnabled || editBorderLEnabled || editBorderTEnabled || editBorderBEnabled) {
		auto newSize = size();
		auto newPos = pos();
		if (editBorderREnabled) {
			newSize.setWidth(mouseEvent->globalPos().x() - x());
		}
		if (editBorderBEnabled) {
			newSize.setHeight(mouseEvent->globalPos().y() - y());
		}
		if (editBorderLEnabled) {
			newPos.setX(mouseEvent->globalPos().x() - relativePressedPos.x());
			newSize.setWidth(x() - newPos.x() + width());
		}
		if (editBorderTEnabled) {
			newPos.setY(mouseEvent->globalPos().y() - relativePressedPos.y());
			newSize.setHeight(y() - newPos.y() + height());
		}
		move(newPos);
		resize(newSize);

		// not good, dithering both side
		// setGeometry(newPos.x(), newPos.y(), newSize.width(), newSize.height());
	}
	else if (moveEnabled) {
		if (isMaximized()) {
			relativePressedPos.setX(static_cast<float>(mouseEvent->globalPos().x()) / width() * normalGeometry().width());
			showNormal();
		}
		else if (halfScreenState) {
			relativePressedPos.setY(static_cast<float>(mouseEvent->globalPos().y()) / height() * standardSize.height());
			resize(standardSize);
			halfScreenState = false;
		}

		move(mouseEvent->globalPos() - relativePressedPos);

		if (mouseEvent->globalPos().y() == 0) {
			showMaximized();
		}
		else if (mouseEvent->globalPos().x() == 0) {
			float pixelRatio = QGuiApplication::primaryScreen()->devicePixelRatio();
			setGeometry(
				0, 
				0, 
				GetSystemMetrics(SM_CXFULLSCREEN) / 2 / pixelRatio, 
				GetSystemMetrics(SM_CYFULLSCREEN) / pixelRatio
			);
			halfScreenState = true;
		}
		else if (mouseEvent->globalPos().x() == QGuiApplication::primaryScreen()->size().width() - 1) {
			float pixelRatio = QGuiApplication::primaryScreen()->devicePixelRatio();
			setGeometry(GetSystemMetrics(SM_CXFULLSCREEN) / 2 / pixelRatio,
				0,
				GetSystemMetrics(SM_CXFULLSCREEN) / 2 / pixelRatio,
				GetSystemMetrics(SM_CYFULLSCREEN) / pixelRatio
			);
			halfScreenState = true;
		}
	}
	QMainWindow::mouseMoveEvent(mouseEvent);
}

void CrystalVolume::mouseReleaseEvent(QMouseEvent* mouseEvent) {
	if (editBorderREnabled || editBorderLEnabled || editBorderTEnabled || editBorderBEnabled) {
		standardSize = size();
	}
	editBorderREnabled = false;
	editBorderLEnabled = false;
	editBorderTEnabled = false;
	editBorderBEnabled = false;
	moveEnabled = false;
	QMainWindow::mouseReleaseEvent(mouseEvent);
}

void CrystalVolume::mouseDoubleClickEvent(QMouseEvent* mouseEvent) {
	if (isMaximized()) {
		showNormal();
	}
	else {
		showMaximized();
	}
	QMainWindow::mouseDoubleClickEvent(mouseEvent);
}

void CrystalVolume::keyPressEvent(QKeyEvent* _event) {
	if (_event->modifiers() == Qt::ControlModifier) {
		if (_event->key() == Qt::Key_Tab) {
			ui.tabWidgetMain->setCurrentIndex(ui.tabWidgetMain->currentIndex() + 1);
		}
	}
	// shift + tab doesn't work...

	QMainWindow::keyPressEvent(_event);
}

void CrystalVolume::dragEnterEvent(QDragEnterEvent* _event) {
	_event->setAccepted(true);
	QMainWindow::dragEnterEvent(_event);
}

void CrystalVolume::dropEvent(QDropEvent* _event) {
	auto mimeData = _event->mimeData();
	if (mimeData->hasUrls()) {
		for (const auto& url : mimeData->urls()) {
			createExplorerTab(url.toLocalFile().replace('/', '\\'));
		}
	}
}

void CrystalVolume::onUiTimer() {
	// only update current tab
	int index = ui.tabWidgetMain->currentIndex();
	auto explorerHwnd = ui.tabWidgetMain->hwnd(index);
	ui.tabWidgetMain->setTabText(index, GetExplorerFolderName(explorerHwnd));
	setCurrentDir(explorerHwnd);
}

void CrystalVolume::initProfile() {
	if (profile.exists("width") && profile.exists("height")) {
		QSize size = { profile["width"].toInt(), profile["height"].toInt() };
		resize(size);
	}
}

void CrystalVolume::saveProfile() {
	profile.set("width", width());
	profile.set("height", height());
}

void CrystalVolume::onCreateExplorerWindowInTabThreadHwndSignal(HWND hwnd) {
	auto explorerWindow = QWindow::fromWinId(reinterpret_cast<WId>(hwnd));
	// parent must be "this", otherwise the magic bar will disable.
	int currentIndex = ui.tabWidgetMain->addExplorerTab(QWidget::createWindowContainer(explorerWindow, this), hwnd);
	wattingExplorerTabPaths.removeFirst();
	ui.tabWidgetMain->setCurrentIndex(currentIndex);
	explorerTabThreadAlreadly = true;

	if (!wattingExplorerTabPaths.isEmpty()) {
		createExplorerTab();
	}
}

void CrystalVolume::onClickedPushButtonAddTab() {
	createExplorerTab(defaultExplorerTabPath);
}