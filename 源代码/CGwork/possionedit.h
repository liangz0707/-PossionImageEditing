#include "opencv\cv.h"
#include "opencv\highgui.h"
#include"opencv2\imgproc\imgproc.hpp"
#include"opencv2\photo\photo.hpp"
#include <iostream>
#include <stdlib.h>
#include <complex>
#include "math.h"

using namespace std;
using namespace cv;

class possionedit
{
	public:
	//output: ÿ��ͨ���ĺϳɽ������
	//rbgx_channel, rgby_channel��gxx�� gyy ��ͨ�����
	vector <Mat> rgb_channel, rgbx_channel, rgby_channel, output;
	//smask��sourceͼƬ��mask�� smask1��smaskȡ���Ľ��
	//grx, gry ��dstͼƬ���ݶȡ� grx32�� gry32��smask1������ݶ�
	//sgx, sgy ��sourceͼƬ���ݶȡ� srx32, sry32��smask������ݶ�
	Mat grx, gry, sgx, sgy, srx32, sry32, grx32, gry32, smask, smask1;
	
	void initialization(Mat &I, Mat &mask, Mat &wmask);
	void scalar_product(Mat mat, float r, float g, float b);
	void array_product(Mat mat1, Mat mat2, Mat mat3);
	void poisson(Mat &I, Mat &gx, Mat &gy, Mat &sx, Mat &sy);
	void evaluate(Mat &I, Mat &wmask, Mat &cloned);
	void getGradientX(const Mat &img, Mat &gx);
	void getGradientY(const Mat &img, Mat &gy);
	void getLaplaceX(const Mat &img, Mat &gxx);
	void getLaplaceY(const Mat &img, Mat &gyy);
	void DST(double *mod_diff, double *sineTransform, int h, int w);
	void IDST(double *mod_diff, double *sineTransform, int h, int w);
	void T(double *mat, double *mat_t, int h, int w);
	void solve(const Mat &img, double *mod_diff, Mat &result);
	void poisson_solver(const Mat &img, Mat &gxx, Mat &gyy, Mat &result);
	void seamless_clone(Mat &I, Mat &mask, Mat &wmask, Mat &cloned);
};