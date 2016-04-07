#ifndef ACTIVELABEL_H
#define ACTIVELABEL_H

#include <QLabel>
#include "ui_activelabel.h"
#include<QMouseEvent>
class ActiveLabel : public QLabel
{
	Q_OBJECT

public:
	ActiveLabel(QWidget *parent = 0);
	~ActiveLabel();
	// Û±ÍµƒŒª÷√£ª
	int x , y;
private:
	Ui::ActiveLabel ui;

protected:
	void mouseReleaseEvent(QMouseEvent * ev);
	void mouseMoveEvent(QMouseEvent * ev);
	void mousePressEvent(QMouseEvent * ev);
signals: 
	void mouseMove(); 
	void mouseRelease(); 
	void mousePress(); 
};

#endif // ACTIVELABEL_H


