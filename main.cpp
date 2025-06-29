#include "CrystalVolume.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication application(argc, argv);
	CrystalVolume crystalVolume;
	crystalVolume.show();
	return application.exec();
}
