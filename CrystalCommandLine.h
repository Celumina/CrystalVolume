#pragma once

#include <QLineEdit>
#include <QProcess>

class CrystalCommandLine : public QLineEdit {
	Q_OBJECT

public:
	class Config {
	public:
		enum {
			maximumCommandBufferCount = 128
		};
	};

	CrystalCommandLine(QWidget* parent = nullptr);

signals:
	void createExplorerTabSignal(const QString& dir);

protected:
	void dragEnterEvent(QDragEnterEvent* _event) override;
	void dropEvent(QDropEvent* _event) override;

	void keyPressEvent(QKeyEvent* _event) override;
	void mousePressEvent(QMouseEvent* _event) override;

private:
	void addCommandBuffer(const QString& text);
	void flipCommandText(int offset);

	QPalette palette;
	QProcess cmdProcess;

	int commandBufferIndex = 0;
	QStringList commandBuffers;
};

