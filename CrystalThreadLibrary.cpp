#include "CrystalThreadLibrary.h"
#include <QProcess>

#include "CrystalWindowsLibrary.h"

CreateExplorerTabThread::CreateExplorerTabThread(const QString& tabPath) :
	rTabPath(tabPath) {
	connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}


void CreateExplorerTabThread::run() {
	auto excludeHwnds = GetAllExplorerHwnd();

	// cmd "start" would lock thread because can only open one same window, ontherwise make window active, but not create
	QProcess::startDetached("explorer ", QStringList{ rTabPath });

	// case Dictory is Disk-Flag Only
	QString path = rTabPath;

	// DEV: deprecated wPath
	//const wchar_t* wPath = reinterpret_cast<const wchar_t*>(path.utf16());
	//if (path == "") {
	//	wPath = 0;
	//}

	HWND explorerHwnd = 0;
	HWND toolbarHwnd = 0;

	// DEV: BUG: will capture same name depend-explorer, and close it make restart explorer..
	//  in this case, if first close depend-explorer, than close CV, it make one explorer leak..
	int findCount = 0;
	while (true) {
		explorerHwnd = FindWindowEx(0, explorerHwnd, L"CabinetWClass", 0);
		if (excludeHwnds.count(explorerHwnd)) {
			continue;
		}

		//
		//// OFF : very aggressive no check mode, very fast but explorer no be complete loaded.
		//if (explorerHwnd) {
		//	break;
		//}


		//
		// ON : Stable mode
		toolbarHwnd = GetChildToolbarHwnd(explorerHwnd);
		if (toolbarHwnd) {
			auto rawDir = GetWindowTitle(toolbarHwnd);
			if (rawDir.mid(rawDir.indexOf(' ') + 1, rawDir.length()) == path) {
				break;
			}
		}
		// if has explorer but toolbar different, continue no sleep.
		if (explorerHwnd) {
			continue;
		}
		//

		Sleep(1);

		++findCount;
		if (findCount > Config::MaximumFindCount) {
			break;
		}
	}
	emit hwndSignal(explorerHwnd);
}

CreateCmdThread::CreateCmdThread(const QString& cmdCommand) : 
command(cmdCommand) {
	connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void CreateCmdThread::run() {
	system(command.toUtf8().data());
}
