#pragma once
#include <QTabWidget>
#include <map>
class CrystalTabWidget : public QTabWidget {
	Q_OBJECT
public:
	CrystalTabWidget(QWidget* parent = nullptr);
	int addContainerTab(QWidget* widget, const QString& tabName, HWND hwnd);
	int addExplorerTab(QWidget* widget, HWND hwnd);
	void removeContainerTab(int index);
	const QString& configFilePath();
	HWND hwnd(int index);

protected:
	void resizeEvent(QResizeEvent* _event) override;
	void closeEvent(QCloseEvent* _event) override;

protected slots:
	void onTabCloseRequested(int index);
	void onCurrentChanged(int index);

signals:
	void tabAllClosedSignal();

private:
	QString historyPath = "./history";
	std::map<QWidget*, HWND> hwndMap;
};

