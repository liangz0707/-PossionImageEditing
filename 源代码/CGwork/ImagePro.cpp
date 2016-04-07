#include"ImagePro.h"
#include<qdebug.h>
#include<string>
#include<opencv2\photo\photo_c.h>
#include<opencv2\imgproc\imgproc_c.h>
ImagePro::ImagePro(){
}
ImagePro::~ImagePro(){
}
//����ģ�壻
void ImagePro::writeMask(cv::Mat& s,cv::Mat& image,cv::Mat& mask,int  x, int  y,double radius){
	cv::Mat tmp;
	image.copyTo(tmp);
	
	for(int row = -radius; row < radius + 1; row++){
		for(int col = -radius; col < radius + 1; col++){
			//�ƶ���ĵ��ڷ�Χ�ڣ�
			if(x+ col < 0 || y + row < 0 ||x + col >= tmp.cols ||y + row >= tmp.rows)
				continue;
			if( (col )*(col ) + (row)*(row) > radius*radius)
				continue;
			tmp.at<cv::Vec3b>(  y + row,x +col).val[2] = s.at<cv::Vec3b>(  y + row,x +col).val[2];
			tmp.at<cv::Vec3b>(  y + row,x +col).val[1] = s.at<cv::Vec3b>(  y + row,x +col).val[1]*0.5;
			tmp.at<cv::Vec3b>(  y + row,x +col).val[0] = s.at<cv::Vec3b>(  y + row,x +col).val[0]*0.9;
			mask.at<uchar>(  y + row,x +col) = 255;//ģ���λ�ñ�ɺ�ɫ����Ҫ�޸��ĵ㣻
		}
	}
	tmp.copyTo(image);
}
void ImagePro::PossionEditing(cv::Mat& Source,cv::Mat& sMask,cv::Mat& Target,cv::Mat &tMask,cv::Mat& Result){
	Source.copyTo(Result);
}
void ImagePro::PatchMatch(cv::Mat&Simg ,cv::Mat& Timg,cv::Mat&Result){
	cv::Mat source;
	cv::Mat target;
	patchmatch *pm;
	cv::Mat result;
	Simg.convertTo(source,CV_32FC3,1/255.f);//���Ų�ת������һ���������ͣ� Ҳ�����ŵ�0��1��ת���ɸ����ͣ�
	Timg.convertTo(target,CV_32FC3,1/255.f);
	//����Lab�ռ䣻
	cvtColor(source,source,CV_RGB2Lab);
	cvtColor(target,target,CV_RGB2Lab);

	//���Ƶ���ά���飻
	pm = new patchmatch();
	pm->target_cols= target.cols;
	pm->target_rows= target.rows;
	pm->source_cols= source.cols;
	pm->source_rows= source.rows;

	pm->fromImage2Array(target	,		&pm->target_data	,target.cols,target.rows);
	pm->fromImage2Array(source	,		&pm->source_data,source.cols,source.rows);
	pm->fromImage2Array(target,			&pm->reslut_data	,target.cols,target.rows);
 
	//GPU���ٹ��̣�
	pm->InitRandom();
	pm->radius=3;
    pm->patchmatchGPU();
	pm->Generate();

	pm->fromArray2Image(target,pm->reslut_data,target.cols,target.rows);

	cvtColor(target,target,CV_Lab2RGB);
	target.convertTo(Result,CV_8UC3,255.0);

}