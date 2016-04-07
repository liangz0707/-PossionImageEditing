#ifndef CGWORK_H
#define CGWORK_H

#include <QtWidgets/QMainWindow>
#include "ui_cgwork.h"

class CGwork : public QMainWindow
{
	Q_OBJECT

public:
	CGwork(QWidget *parent = 0);
	~CGwork();

private:
	Ui::CGworkClass ui;
};

#endif // CGWORK_H
