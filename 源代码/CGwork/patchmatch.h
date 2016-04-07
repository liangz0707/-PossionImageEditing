#include "opencv\cv.h"
#include "opencv\highgui.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include<qdebug.h>

void extern  bridge(double *  ,double *  ,int *  ,int,int ,int,int, double * distance);
class patchmatch{
public:
	int radius;

	double *source_data;
	double *target_data;
	double *reslut_data;//±£´æ½á¹û
	int *ralation;
	double *generate;

	int source_cols,source_rows;
	int target_cols,target_rows;

	void InitRandom();
	void Generate();
	void patchmatchGPU();

	 void fromImage2Array(cv::Mat,double **data,int cols,int rows);
	 void fromArray2Image(cv::Mat,double *data,int cols,int rows);
	 void fromImage2Array(cv::Mat,int **data,int cols,int rows);
	 void fromArray2Image(cv::Mat,int *data,int cols,int rows);
};