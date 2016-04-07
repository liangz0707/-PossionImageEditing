#include "activelabel.h"

ActiveLabel::ActiveLabel(QWidget *parent)
	: QLabel(parent)
{
	ui.setupUi(this);
}

ActiveLabel::~ActiveLabel()
{

}
void ActiveLabel::mouseReleaseEvent(QMouseEvent * ev) 
{//定义鼠标左键点击事件 
	if(ev->button() == Qt::LeftButton)
	{
		x= ev->x();
		y= ev->y();
		Q_UNUSED(ev) 
			emit mouseRelease();
	}
}
void ActiveLabel::mouseMoveEvent(QMouseEvent * ev)
{
		x= ev->x();
		y= ev->y();
		Q_UNUSED(ev) 
			emit mouseMove();
}
void ActiveLabel::mousePressEvent(QMouseEvent * ev){
	if(ev->button() == Qt::LeftButton)
	{
		x= ev->x();
		y= ev->y();
		Q_UNUSED(ev) 
			emit mousePress();
	}
}