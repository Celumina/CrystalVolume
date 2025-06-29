#include "CrystalWindowsLibrary.h"

QString GetWindowTitle(HWND hwnd) {
	int len = GetWindowTextLength(hwnd);
	if (!len) {
		return QString();
	}

	auto wcharBuffer = new wchar_t[len + 1];
	GetWindowTextW(hwnd, wcharBuffer, len + 1);
	QString	title = QString::fromWCharArray(wcharBuffer);
	delete[] wcharBuffer;

	return title;
}

QString GetWindowClassName(HWND hwnd) {
	// maximum classname len == 256, + 1 endchar
	int len = 257;
	auto wcharBuffer = new wchar_t[len];
	GetClassName(hwnd, wcharBuffer, len);
	QString className = QString::fromWCharArray(wcharBuffer);
	delete[] wcharBuffer;

	return className;
}

HWND GetChildToolbarHwnd(HWND explorerHwnd) {
	HWND toolbarHwnd = 0;
	toolbarHwnd = FindWindowEx(explorerHwnd, 0, L"WorkerW", 0);
	toolbarHwnd = FindWindowEx(toolbarHwnd, 0, L"ReBarWindow32", 0);
	toolbarHwnd = FindWindowEx(toolbarHwnd, 0, L"Address Band Root", 0);
	toolbarHwnd = FindWindowEx(toolbarHwnd, 0, L"msctls_progress32", 0);
	toolbarHwnd = FindWindowEx(toolbarHwnd, 0, L"Breadcrumb Parent", 0);
	toolbarHwnd = FindWindowEx(toolbarHwnd, 0, L"ToolbarWindow32", 0);
	return toolbarHwnd;
}

QString GetExplorerDir(HWND explorerHwnd) {
	QString dir = GetWindowTitle(GetChildToolbarHwnd(explorerHwnd));
	return dir.mid(dir.indexOf(' ') + 1, dir.length());
}


QString GetExplorerFolderName(HWND explorerHwnd) {
	QString dir = GetWindowTitle(GetChildToolbarHwnd(explorerHwnd));
	auto begPos = dir.lastIndexOf('\\');
	if (begPos == -1 || begPos == dir.length() - 1) {
		begPos = dir.indexOf(' ');
	}
	return dir.mid(begPos + 1, dir.length());
}

std::set<HWND> GetAllExplorerHwnd() {
	std::set<HWND> hwndSet;
	EnumWindows(
		reinterpret_cast<WNDENUMPROC>(CrystalWindowsLibraryPrivate::EnumWindowsAllExplorer), 
		reinterpret_cast<LPARAM>(&hwndSet)
	);
	return hwndSet;
}

bool CrystalWindowsLibraryPrivate::EnumWindowsAllExplorer(HWND hwnd, LPARAM pSet) {
	auto hwndSet = reinterpret_cast<std::set<HWND>*>(pSet);
	if (GetWindowClassName(hwnd) == "CabinetWClass") {
		hwndSet->insert(hwnd);
	}
	return true;
}
