#pragma once
#include <QObject>
#include <QThread>
#include "CrystalTabWidget.h"

class CreateExplorerTabThread :  public QThread {
	Q_OBJECT

public:
	class Config {
	public:
		enum {
			MaximumFindCount = 100
		};
	};

	CreateExplorerTabThread(const QString& tabPath);

protected:
	void run() override;

signals:
	void hwndSignal(HWND hwnd);

private:
	const QString& rTabPath;
};


class CreateCmdThread : public QThread {
	Q_OBJECT

public:
	CreateCmdThread(const QString& cmdCommand);

protected:

	void run() override;

private:
	QString command;
};

