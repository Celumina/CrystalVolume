#include "CrystalCommandLine.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QDir>

#include "CrystalThreadLibrary.h"

CrystalCommandLine::CrystalCommandLine(QWidget* parent) : 
QLineEdit(parent) {
	setAcceptDrops(true);
	palette.setColor(QPalette::PlaceholderText, QColor(0xD0D0D0));
	palette.setColor(QPalette::Text, QColor(0x7070AA));

	setPalette(palette);
}

void CrystalCommandLine::keyPressEvent(QKeyEvent* _event) {

	if (_event->key() == Qt::Key_Return) {
		auto simplifiedText = text().simplified().remove('\"');
		// custom command: add a selected tab
		if (QDir(simplifiedText).exists()) {
			emit createExplorerTabSignal(simplifiedText.replace('/', '\\'));
		}
		else {
			addCommandBuffer(text());
			auto cmdThread = new CreateCmdThread(text());
			cmdThread->start();
		}
		clear();
	}

	if (_event->key() == Qt::Key_Up) {
		flipCommandText(-1);
	}
	else if (_event->key() == Qt::Key_Down) {
		flipCommandText(1);
	}

	QLineEdit::keyPressEvent(_event);
}

void CrystalCommandLine::mousePressEvent(QMouseEvent* _event) {
	setFocus();
	activateWindow();
	QLineEdit::mousePressEvent(_event);
}

void CrystalCommandLine::addCommandBuffer(const QString& text) {
	if (!commandBuffers.isEmpty()) {
		if (commandBufferIndex != commandBuffers.length()) {
			commandBuffers.removeAt(commandBufferIndex);
		}
		if (commandBuffers.last() == "") {
			commandBuffers.removeLast();
		}
	}

	commandBuffers.push_back(text);

	if (commandBuffers.length() > Config::maximumCommandBufferCount) {
		commandBuffers.removeFirst();
	}
	commandBufferIndex = commandBuffers.length();
}

void CrystalCommandLine::flipCommandText(int offset) {
	if (commandBuffers.isEmpty()) {
		return;
	}

	if (commandBufferIndex < commandBuffers.length() && 
		commandBuffers[commandBufferIndex] != text() && 
		text() != "") {
		commandBuffers[commandBufferIndex] = text();
	}

	commandBufferIndex += offset;

	if (commandBufferIndex < 0) {
		commandBufferIndex = 0;
	}
	else if (commandBufferIndex >= commandBuffers.length()) {
		if (text() != "" && offset > 0) {
			addCommandBuffer("");
		}
		commandBufferIndex = commandBuffers.length() - 1;
	}

	setText(commandBuffers[commandBufferIndex]);
}

void CrystalCommandLine::dragEnterEvent(QDragEnterEvent* _event) {
	_event->setAccepted(true);
	QLineEdit::dragEnterEvent(_event);
}

void CrystalCommandLine::dropEvent(QDropEvent* _event) {
	auto mimeData = _event->mimeData();
	if (mimeData->hasText()) {
		// only accept first url now
		QString texts = " ";
		for (const auto& url : mimeData->urls()) {
			// protected char ' ' and '\"'
			texts = texts + "\"" + url.toLocalFile() + "\" ";
		}
		insert(texts);
	}

	// Take back the focus.
	setFocus();
	activateWindow();

	// don't do this, its has own idea.
	//QLineEdit::dropEvent(_event);
}
