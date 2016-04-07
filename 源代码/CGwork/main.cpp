#include "opencvwork.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>
#include <QLabel>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	OpencvWork w;

	w.show();
	return a.exec();
}
