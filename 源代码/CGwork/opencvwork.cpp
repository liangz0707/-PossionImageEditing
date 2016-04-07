#include "opencvwork.h"
#include "QFileDialog.h"
#include "qDebug"
#include "QMessageBox"
#include <string>
#include <QPushButton>

/**
*ͨ��Ϊ QLabel ���mouseEvent���������Ӧ�¼��� 
*/
OpencvWork::OpencvWork(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.srcImgLabel->setStyleSheet("QLabel{border:3px solid gray}");
	ui.dstImgLabel->setStyleSheet("QLabel{border:3px solid gray}");
	ui.srcImgLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
	ui.dstImgLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);


	//��Ӧ�˵�������һ��
	connect(ui.openActionA,SIGNAL(triggered()),this,SLOT(openImgActionA()));
	connect(ui.openActionB,SIGNAL(triggered()),this,SLOT(openImgActionB()));
	connect(ui.exitAction,SIGNAL(triggered()),this,SLOT(close()));

	//��ఴ����
	connect(ui.srcImgButton,SIGNAL(clicked()),this,SLOT(showSrcInCV()));
	connect(ui.dstImgButton,SIGNAL(clicked()),this,SLOT(showDstInCV()));
	connect(ui.returnMaskButton,SIGNAL(clicked()),this,SLOT(returnMask()));
	
	//����¼�:ֻ������ͼ��
	connect(ui.dstImgLabel,SIGNAL(mouseMove()),this,SLOT(mouseMoveDst()));	
	connect(ui.srcImgLabel,SIGNAL(mouseMove()),this,SLOT(mouseMoveSrc()));

	
	connect(ui.PIEButton,SIGNAL(clicked()),this,SLOT(PossionEditing()));	
	connect(ui.PMButton,SIGNAL(clicked()),this,SLOT(PatchMatch()));
}
//��ͼƬ:��ȫ�ֿռ䣺��¼ͼƬ��·����ͼƬ�����֣�	��QT CV���ַ�ʽ�ֱ𱣴棻
void OpencvWork::openImgActionA()
{
	//ʹ��opencv�ķ�ʽ��ȡͼƬ��
	fileDir =  QFileDialog::getOpenFileName (this,"Choose Image","D:/workspace/VS2012/OpencvHomework/data","Image File(*.jpg)");
	srcMat = cv::imread((const char *)fileDir.toLocal8Bit());//ʹ��cv��ʽ��ȡ����

	if(fileDir.isEmpty()){
		return ;
	}
	//ͼƬ��ʽת��ΪQT������ʹ������ʱ����
	srcMat.copyTo(tmpMat);
	formCV2QT(tmpMat,srcImage);

	//ͨ��QT���̶���С��ʾͼƬ��
	QPixmap pix = QPixmap::fromImage(srcImage);
	int h = ui.srcImgLabel->height();
	int w = ui.srcImgLabel->width();

	r_src= countR( h ,  w ,  pix.height()  , pix.width() );

	pix = pix.scaled( pix.width()*r_src,pix.height()*r_src);

	ui.srcImgLabel->setPixmap(pix);  
	ui.srcImgLabel->show();  
	srcMat.copyTo(srcBak);
	updateMask();
}
void OpencvWork::openImgActionB()
{
	//ʹ��opencv�ķ�ʽ��ȡͼƬ��
	fileDir =  QFileDialog::getOpenFileName (this,"Choose Image","D:/workspace/VS2012/OpencvHomework/data","Image File(*.jpg)");
	dstMat = cv::imread((const char *)fileDir.toLocal8Bit());//ʹ��cv��ʽ��ȡ����

	if(fileDir.isEmpty()){
		return ;
	}
	//ͼƬ��ʽת��ΪQT������ʹ������ʱ����
	dstMat.copyTo(tmpMat);
	formCV2QT(tmpMat,dstImage);

	//ͨ��QT���̶���С��ʾͼƬ��
	QPixmap pix = QPixmap::fromImage(dstImage);
	int h = ui.dstImgLabel->height();
	int w = ui.dstImgLabel->width();

	r_dst = countR( h ,  w ,  pix.height()  , pix.width() );

	pix = pix.scaled( pix.width()*r_dst,pix.height()*r_dst);

	ui.dstImgLabel->setPixmap(pix);  
	ui.dstImgLabel->show();  
	dstMat.copyTo(dstBak);
	updateMask();

}

//��opencv��ʽ��ͼƬת��ΪqImage��ʽ�������ͨ����Ҳ���޸�mat
void OpencvWork::formCV2QT(cv::Mat mat, QImage &qimg){
	//������ԭʼͼ����Ϊ�����ߣ�
	//��������ʱ���������޷���ȡ���ݣ����߳����⣿
	cv::cvtColor(mat,mat,CV_BGR2RGB);
	uchar* data =mat.data;//��ȡ����
	int h = mat.rows;
	int w = mat.cols;
	int step = mat.step;
	int channels = mat.channels();
	qimg = QImage(data,w,h,step,QImage::Format_RGB888,0,0);
}
//qImage��ʽ��ת��Ϊopencv��ʽ��
void OpencvWork::formQT2CV( QImage qimg,cv::Mat &mat ){

}
//��ʾ���:��ʾȫ���Ǵ�cv�ж�ȡ���ݣ�
void OpencvWork::showDst(){
	//ͼƬ��ʽת��ΪQT��
	dstMat.copyTo(tmpMat);
	formCV2QT(tmpMat,dstImage);

	//ͨ��QT���̶���С��ʾͼƬ:ͼƬ��С������
	int h = ui.dstImgLabel->height();
	int w = ui.dstImgLabel->width();
	
	QPixmap pix = QPixmap::fromImage(dstImage);
	dst_h=pix.height();
	dst_w=	pix.width();

	pix = pix.scaled( dst_w*r_dst,dst_h*r_dst);
	ui.dstImgLabel->resize(pix.size());
	ui.dstImgLabel->setPixmap(pix);  
	ui.dstImgLabel->show();  
}
void OpencvWork::showSrc(){
	//ͼƬ��ʽת��ΪQT��
	srcMat.copyTo(tmpMat);
	formCV2QT(tmpMat,srcImage);

	//ͨ��QT���̶���С��ʾͼƬ:ͼƬ��С������
	int h = ui.srcImgLabel->height();
	int w = ui.srcImgLabel->width();
	
	QPixmap pix = QPixmap::fromImage(srcImage);
	src_h=pix.height();
	src_w=	pix.width();
	
	pix = pix.scaled( src_w*r_src,src_h*r_src);
	ui.srcImgLabel->resize(pix.size());
	ui.srcImgLabel->setPixmap(pix);  
	ui.srcImgLabel->show();  
}
//��opencv����ʾԭͼ��
void OpencvWork::showSrcInCV(){
	if(srcMat.data!=NULL){
		cv::imshow("source image",srcMat);
	}else{
		 QMessageBox::question(this,"warning","Please load a image first!",QMessageBox::Ok);
	}
}
//��opencv����ʾ�����
void OpencvWork::showDstInCV(){
	if(dstMat.data!=NULL){
		cv::imshow("destinaton image",dstMat);
	}
}
//��openCV�г���ģ�壻
void OpencvWork::returnMask(){
		if(checkValidateDst()&&checkValidateSrc()){
			cv::cvtColor(srcMat, srcMask, CV_BGR2GRAY);
			for(int row = 0 ; row < srcMat.size().height ; row ++){
				for(int col = 0 ; col <srcMat.size().width  ; col ++){
					srcMask.at<char>(row, col) = 0; //255����ֵ�ĵ㣬�������޸��������Ѿ��޸��ĵ㣻0��û��ֵ�ĵ�
				}
			}
			cv::cvtColor(dstMat, dstMask, CV_BGR2GRAY);
			for(int row = 0 ; row < dstMask.size().height ; row ++){
				for(int col = 0 ; col <dstMask.size().width  ; col ++){
					dstMask.at<char>(row, col) = 0; //255����ֵ�ĵ㣬�������޸��������Ѿ��޸��ĵ㣻0��û��ֵ�ĵ�
				}
			}
			srcBak.copyTo(srcMat);showSrc();
			dstBak.copyTo(dstMat);showDst();
		}
}
//�������ű�����
double OpencvWork::countR(int h , int w , int pixh ,int pixw){
	double r;
	 r = (h+0.0)/pixh <  (w+0.0)/pixw?  (h+0.0)/pixh:(w+0.0)/pixw;
	return r;
}
//��֤�Ϸ��ԣ�
bool OpencvWork::checkValidateSrc(){
	if(srcMat.data!=NULL){
		return true;
	}else{
		// QMessageBox::question(this,"warning","Please load a image first!",QMessageBox::Ok);
		 return false;
	}
}
bool OpencvWork::checkValidateDst(){
	if(dstMat.data!=NULL){
		return true;
	}else{
		// QMessageBox::question(this,"warning","Please load a image first!",QMessageBox::Ok);
		 return false;
	}
}
//��괦��
void OpencvWork::mouseMoveDst(){
	//���ԣ�
	if(checkValidateDst()){
		int x,y;
		int h = ui.srcImgLabel->height();
		int w = ui.srcImgLabel->width();
		x =ui.dstImgLabel->x - (w - r_dst*dst_w) /2.0 ;
		y =ui.dstImgLabel->y - ( h - r_dst*dst_h) /2.0 ;
	//	mask
		ImagePro::writeMask(dstBak,dstMat ,dstMask, x/r_dst,y/r_dst,ui.brushSpinBox->value());
		showDst();
	}
}
void OpencvWork::mouseMoveSrc(){
	//���ԣ�
	if(checkValidateSrc()){
			int x,y;
			int h = ui.srcImgLabel->height();
			int w = ui.srcImgLabel->width();
			x =ui.srcImgLabel->x - (w - r_src*src_w) /2.0 ;
			y =ui.srcImgLabel->y - (h - r_src*src_h) /2.0 ;
	//	mask
			ImagePro::writeMask(srcBak,srcMat ,srcMask, x/r_src,y/r_src,ui.brushSpinBox->value());
			showSrc();
	}
}
//����������
OpencvWork::~OpencvWork()
{	
}	
void OpencvWork::updateMask(){
		if(checkValidateSrc()){
			cv::cvtColor(srcMat, srcMask, CV_BGR2GRAY);
			for(int row = 0 ; row < srcMat.size().height ; row ++){
				for(int col = 0 ; col <srcMat.size().width  ; col ++){
					srcMask.at<uchar>(row, col) = 0; //255����ֵ�ĵ㣬�������޸��������Ѿ��޸��ĵ㣻0��û��ֵ�ĵ�
				}
			}
		}
		if(checkValidateDst()){
			cv::cvtColor(dstMat, dstMask, CV_BGR2GRAY);
			for(int row = 0 ; row < dstMask.size().height ; row ++){
				for(int col = 0 ; col <dstMask.size().width  ; col ++){
					dstMask.at<uchar>(row, col) = 0; //255����ֵ�ĵ㣬�������޸��������Ѿ��޸��ĵ㣻0��û��ֵ�ĵ�
				}
			}
		}

}

//�㷨��
//Possion Image Editing;
void  OpencvWork::PossionEditing(){
		int minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;
		for(int row = 0 ; row < dstMask.size().height ; row ++){
			for(int col = 0 ; col <dstMask.size().width  ; col ++){
				if (dstMask.at<uchar>(row,col)==255)
				{
					minx = std::min(minx,col);
					maxx = std::max(maxx,col);
					miny = std::min(miny, row);
					maxy = std::max(maxy, row);
				}
			}
		}
		p.x = (minx+ maxx) /2;
		p.y = (miny+maxy) /2;

		Mat src = srcBak;
		Mat dest = dstBak;
		Mat mask =srcMask;
		Mat blend;
		
		blend = Mat::zeros(dest.size(), CV_8UC3);

		minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;
		int h = mask.size().height;
		int w = mask.size().width;

		Mat gray = Mat(mask.size(), CV_8UC1);
		Mat dst_mask = Mat::zeros(dest.size(), CV_8UC1);
		Mat cs_mask = Mat::zeros(src.size(), CV_8UC3);
		Mat cd_mask = Mat::zeros(dest.size(), CV_8UC3);

		if (mask.channels() == 3)
			cvtColor(mask, gray, COLOR_BGR2GRAY);
		else
			gray = mask;

		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{	
				if (gray.at<uchar>(i, j)  ==255)
				{
					
					minx = std::min(minx, i);
					maxx = std::max(maxx, i);
					miny = std::min(miny, j);
					maxy = std::max(maxy, j);
				}
			}
		}
		int lenx = maxx - minx;
		int leny = maxy - miny;

		int minxd = p.y - lenx / 2;
		int maxxd = p.y + lenx / 2;
		int minyd = p.x - leny / 2;
		int maxyd = p.x + leny / 2;
		//src ԭͼ��destĿ��ͼ �� grayģ��ͼ�� blend ���ͼ��
		//m** ��Χ: ԭͼ��λ�÷�Χ��
		//m**d ��Χ��Ŀ��ͼ��λ�÷�Χ��
		CV_Assert(minxd >= 0 && minyd >= 0 && maxxd <= dest.rows && maxyd <= dest.cols);

		Rect roi_d(minyd, minxd, leny, lenx);
		Rect roi_s(miny, minx, leny, lenx);

		Mat destinationROI = dst_mask(roi_d);
		Mat sourceROI = cs_mask(roi_s);

		gray(roi_s).copyTo(destinationROI);
		src(roi_s).copyTo(sourceROI, gray(roi_s));

		destinationROI = cd_mask(roi_d);
		cs_mask(roi_s).copyTo(destinationROI);

		possionedit obj;

		obj.seamless_clone(dest, cd_mask, dst_mask, blend);
		cv::imshow("Possion Editing Result",blend);
}
//Patchmatch;
void  OpencvWork::PatchMatch(){
	ImagePro::PatchMatch(srcBak,dstBak,tmpMat);
	cv::imshow("PatchMatch Result",tmpMat);
}