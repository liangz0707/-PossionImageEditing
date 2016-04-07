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
	double r_src,r_dst;//���ű�
	Point p;
	int startX , startY;//��¼��꿪ʼ�ͽ����㣻

	~OpencvWork();
	void formCV2QT(cv::Mat , QImage &);
	void formQT2CV( QImage ,cv::Mat & );

private:
	QProcess a;
	Ui::CGworkClass ui;
	QString fileDir;//�ʼѡ����ļ�����Ŀ¼��
	QImage srcImage;//ԭʼͼƬ��
	cv::Mat srcMat;//ԭʼcvͼƬ��
	cv::Mat srcMask;//ģ�壻
	cv::Mat srcBak;//ģ�壻

	QImage dstImage;//���ͼƬ��
	cv::Mat dstMat;//���cvͼƬ��
	cv::Mat dstMask;//ģ�壻
	cv::Mat dstBak;//ģ�壻

	cv::Mat tmpMat;//��ʱ����

	int src_h;
	int src_w;

	int dst_h;
	int dst_w;

	bool savedLatest;//��¼�Ƿ񱣴������µĽ��ͼƬ��
	void showSrc();
	void showDst();

	double countR(int h , int w , int pixh ,int pixw);
	
	 bool checkValidateSrc();
	 bool checkValidateDst();

private slots:
	//��һ�˵����ݣ�����ͼƬ�򿪱������˳���
	void openImgActionA();
	void openImgActionB();

	//����ϵİ�ť��
	void showSrcInCV();//ʹ��opencv�Ŀռ�����ʾ�鿴ԭʼͼƬ��
    void showDstInCV();//ʹ��opencv�Ŀռ�����ʾ�鿴���ͼƬ��

	//�߼�����ʹ�ã�
	void returnMask();
	void updateMask();

	//�����Ӧ������
	void mouseMoveDst();
	void mouseMoveSrc();

	//�㷨ִ�к�����
	void PossionEditing();
	void PatchMatch();
};

#endif // OPENCVWORK_H
