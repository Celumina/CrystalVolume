#pragma once
#include <QString>
#include <set>
#include <Windows.h>

namespace CrystalWindowsLibraryPrivate {
	bool CALLBACK EnumWindowsAllExplorer(HWND hwnd, LPARAM lParam);
}

QString GetWindowTitle(HWND hwnd);

QString GetWindowClassName(HWND hwnd);

HWND GetChildToolbarHwnd(HWND explorerHwnd);

QString GetExplorerDir(HWND explorerHwnd);

QString GetExplorerFolderName(HWND explorerHwnd);

std::set<HWND> GetAllExplorerHwnd();