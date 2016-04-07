#ifndef OPENCVWORK_H
#define OPENCVWORK_H

#include <QtWidgets/QMainWindow>
#include "ui_cgwork.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include"ImagePro.h"
#include"QProcess"

class OpencvWork : public QMainWindow
{
	Q_OBJECT

public:
	OpencvWork(QWidget *parent = 0);
	double r_src,r_dst;//缩放比
	Point p;
	int startX , startY;//记录鼠标开始和结束点；

	~OpencvWork();
	void formCV2QT(cv::Mat , QImage &);
	void formQT2CV( QImage ,cv::Mat & );

private:
	QProcess a;
	Ui::CGworkClass ui;
	QString fileDir;//最开始选择的文件保存目录；
	QImage srcImage;//原始图片；
	cv::Mat srcMat;//原始cv图片；
	cv::Mat srcMask;//模板；
	cv::Mat srcBak;//模板；

	QImage dstImage;//结果图片；
	cv::Mat dstMat;//结果cv图片；
	cv::Mat dstMask;//模板；
	cv::Mat dstBak;//模板；

	cv::Mat tmpMat;//临时矩阵；

	int src_h;
	int src_w;

	int dst_h;
	int dst_w;

	bool savedLatest;//记录是否保存了最新的结果图片；
	void showSrc();
	void showDst();

	double countR(int h , int w , int pixh ,int pixw);
	
	 bool checkValidateSrc();
	 bool checkValidateDst();

private slots:
	//第一菜单内容：处理图片打开保保存退出；
	void openImgActionA();
	void openImgActionB();

	//面板上的按钮；
	void showSrcInCV();//使用opencv的空间来显示查看原始图片；
    void showDstInCV();//使用opencv的空间来显示查看结果图片；

	//高级功能使用：
	void returnMask();
	void updateMask();

	//鼠标响应函数；
	void mouseMoveDst();
	void mouseMoveSrc();

	//算法执行函数：
	void PossionEditing();
	void PatchMatch();
};

#endif // OPENCVWORK_H
