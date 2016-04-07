#include <stdlib.h>
#include <opencv2/highgui/highgui.hpp>

#include "possionedit.h"

using namespace std;
using namespace cv;

	//����x�����ϵ��ݶȣ�
	void possionedit::getGradientX(const Mat &img, Mat &gx)
	{
		Mat kernel = Mat::zeros(1, 3, CV_8S);//1��3���8����Ӧchar�� 0 -1 1
		kernel.at<char>(0, 2) = 1;
		kernel.at<char>(0, 1) = -1;
		filter2D(img, gx, CV_32F, kernel);//����kernel�����ݶ�
	}
	//����y�����ϵ��ݶȣ�
	void possionedit::getGradientY(const Mat &img, Mat &gy)
	{
		Mat kernel = Mat::zeros(3, 1, CV_8S);
		kernel.at<char>(2, 0) = 1;
		kernel.at<char>(1, 0) = -1;
		filter2D(img, gy, CV_32F, kernel);
	}

	//img��ԭʼͼ��ˮƽ�����ϵ��ݶȡ��������Ƕ��ݶ������ݶȣ�-1 1
	void possionedit::getLaplaceX(const Mat &img, Mat &gxx)
	{
		Mat kernel = Mat::zeros(1, 3, CV_8S);
		kernel.at<char>(0, 0) = -1;
		kernel.at<char>(0, 1) = 1;
		filter2D(img, gxx, CV_32F, kernel);
	}
	
	void possionedit::getLaplaceY(const Mat &img, Mat &gyy)
	{
		Mat kernel = Mat::zeros(3, 1, CV_8S);
		kernel.at<char>(0, 0) = -1;
		kernel.at<char>(1, 0) = 1;
		filter2D(img, gyy, CV_32F, kernel);
	}

	//��ɢ���ұ任���ο����룻���������Ȼ�źţ�����������ͼ�񣩵���������������ɢ���ұ任��ĵ�Ƶ����
	//�ο���http://www.mathworks.com/help/pde/ug/dst.html
	//�ο���http://www.mathworks.com/help/pde/ug/fast-solution-of-poissons-equation.html
	void possionedit::DST(double *mod_diff, double *sineTransform, int h, int w)
	{
		unsigned long int idx;

		Mat temp = Mat(2 * h + 2, 1, CV_32F);
		Mat res = Mat(h, 1, CV_32F);

		Mat planes[] = { Mat_<float>(temp), Mat::zeros(temp.size(), CV_32F) };

		Mat result;
		int p = 0;
		for (int i = 0; i < w; i++)
		{
			temp.at<float>(0, 0) = 0.0;

			for (int j = 0, r = 1; j < h; j++, r++)
			{
				idx = j*w + i;
				temp.at<float>(r, 0) = (float)mod_diff[idx];
			}

			temp.at<float>(h + 1, 0) = 0.0;

			for (int j = h - 1, r = h + 2; j >= 0; j--, r++)
			{
				idx = j*w + i;
				temp.at<float>(r, 0) = (float)(-1.0 * mod_diff[idx]);
			}

			merge(planes, 2, result);

			dft(result, result, 0, 0);

			Mat planes1[] = { Mat::zeros(result.size(), CV_32F), Mat::zeros(result.size(), CV_32F) };

			split(result, planes1);

			std::complex<double> two_i = std::sqrt(std::complex<double>(-1));

			double factor = -2 * imag(two_i);

			for (int c = 1, z = 0; c < h + 1; c++, z++)
			{
				res.at<float>(z, 0) = (float)(planes1[1].at<float>(c, 0) / factor);
			}

			for (int q = 0, z = 0; q < h; q++, z++)
			{
				idx = q*w + p;
				sineTransform[idx] = res.at<float>(z, 0);
			}
			p++;
		}
	}
	//����ɢ���ұ任���ο����룻
	void possionedit::IDST(double *mod_diff, double *sineTransform, int h, int w)
	{
		int nn = h + 1;
		unsigned long int idx;
		DST(mod_diff, sineTransform, h, w);
		for (int i = 0; i < h; i++)
		for (int j = 0; j < w; j++)
		{
			idx = i*w + j;
			sineTransform[idx] = (double)(2 * sineTransform[idx]) / nn;
		}

	}
	//һά�����еĶ�άͼ��ת�ã�
	void possionedit::T(double *mat, double *mat_t, int h, int w)
	{
		Mat tmp = Mat(h, w, CV_32FC1);
		unsigned long int idx;
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				idx = i*(w)+j;
				tmp.at<float>(i, j) = (float)mat[idx];
			}
		}
		Mat tmp_t = tmp.t();//ת�ã�

		for (int i = 0; i < tmp_t.size().height; i++)
		for (int j = 0; j < tmp_t.size().width; j++)
		{
			idx = i*tmp_t.size().width + j;
			mat_t[idx] = tmp_t.at<float>(i, j);
		}
	}
	//�Ⲵ�ɷ���:ֱ�ӽⷨ
	void possionedit::solve(const Mat &img, double *mod_diff, Mat &result)
	{
		int w = img.size().width;
		int h = img.size().height;
		//��������id Ϊͼ��2ά��һά�����е�������
		unsigned long int idx, idx1;

		//���ұ任��
		double *sineTransform = new double[(h - 2)*(w - 2)];// h-2 �� w-2��ʾ��Ҫ�߽磻
		//���ұ任��ת�ã�
		double *sineTransform_t = new double[(h - 2)*(w - 2)];

		double *denom = new double[(h - 2)*(w - 2)];//
		//�����ұ任��
		double *invsineTransform = new double[(h - 2)*(w - 2)];
		//�����ұ任��ת�ã�
		double *invsineTransform_t = new double[(h - 2)*(w - 2)];
		//һάͼ��
		double *img_d = new double[(h)*(w)];
		//�������img_d
	
		DST(mod_diff, sineTransform, h - 2, w - 2);
		T(sineTransform, sineTransform_t, h - 2, w - 2);

		DST(sineTransform_t, sineTransform, w - 2, h - 2);
		T(sineTransform, sineTransform_t, w - 2, h - 2);

		int cy = 1;

		for (int i = 0; i < w - 2; i++, cy++)
		{
			for (int j = 0, cx = 1; j < h - 2; j++, cx++)
			{
				idx = j*(w - 2) + i;
				denom[idx] = (float)2 * cos(CV_PI*cy / ((double)(w - 1))) - 2 + 2 * cos(CV_PI*cx / ((double)(h - 1))) - 2;//0��-8�����Ͻ���0��
			}
		}

		for (idx = 0; idx < (unsigned)(w - 2)*(h - 2); idx++)
		{
			sineTransform_t[idx] = sineTransform_t[idx] / denom[idx];
		}

		IDST(sineTransform_t, invsineTransform, h - 2, w - 2);

		T(invsineTransform, invsineTransform_t, h - 2, w - 2);

		IDST(invsineTransform_t, invsineTransform, w - 2, h - 2);

		T(invsineTransform, invsineTransform_t, w - 2, h - 2);

		//��ά��һά��img -�� idx���ⲿ������
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				idx = i*w + j;
				img_d[idx] = (double)img.at<uchar>(i, j);
			}
		}
		//��ά��һά��idx����ͼ��߽�Ϊ img �ڲ�Ϊ0��
		for (int i = 1; i < h - 1; i++)
		{
			for (int j = 1; j < w - 1; j++)
			{
				idx = i*w + j;
				img_d[idx] = 0.0;
			}
		}
		//�ڲ��ı仯��
		for (int i = 1, id1 = 0; i < h - 1; i++, id1++)
		{
			for (int j = 1, id2 = 0; j < w - 1; j++, id2++)
			{
				idx = i*w + j;
				idx1 = id1*(w - 2) + id2;
				img_d[idx] = invsineTransform_t[idx1];
			}
		}

		//�������:��img_d����ȡ�����
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				idx = i*w + j;
				if (img_d[idx] < 0.0) {
					result.at<uchar>(i, j) = 0;
				}
				else if (img_d[idx] > 255.0)
					result.at<uchar>(i, j) = 255;
				else {
					result.at<uchar>(i, j) = (uchar)img_d[idx];
				}
			}
		}

		delete[] sineTransform;
		delete[] sineTransform_t;
		delete[] denom;
		delete[] invsineTransform;
		delete[] invsineTransform_t;
		delete[] img_d;
	}

	//��img��lap����ϳɽ��, ע��ʵ����lap�д�������
	void possionedit::poisson_solver(const Mat &img, Mat &gxx, Mat &gyy, Mat &result)
	{

		int w = img.size().width;
		int h = img.size().height;

		unsigned long int idx;

		Mat lap = Mat(img.size(), CV_32FC1);

		lap = gxx + gyy;

		Mat bound = img.clone();
		
		//rectangle ��Χ����ԭ����rect�ڲ���Ϊscalar
		rectangle(bound, Point(1, 1), Point(img.cols - 2, img.rows - 2), Scalar::all(0), -1);
		//rectangle(bound, Point(1, 1), Point(img.cols - 50, img.rows - 50), Scalar::all(0), -1);

		double *boundary_point = new double[h*w];

		//Ϊ��ֵ�Ĳ���Ϊ0��
		for (int i = 1; i < h - 1; i++)
		for (int j = 1; j < w - 1; j++)
		{
			idx = i*w + j;
			boundary_point[idx] = -4 * (int)bound.at<uchar>(i, j) + (int)bound.at<uchar>(i, (j + 1)) + (int)bound.at<uchar>(i, (j - 1))
				+ (int)bound.at<uchar>(i - 1, j) + (int)bound.at<uchar>(i + 1, j);//5����;����	��;
		}

		//�����ֵΪ0��
		Mat diff = Mat(h, w, CV_32FC1);
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				idx = i*w + j;
				diff.at<float>(i, j) = (float)(lap.at<float>(i, j) - boundary_point[idx]);//ԭͼ����ݶ� ��ȥĿ��ͼ����ݶȣ�//��
			}
		}
		
		//��άתһά��
		double *mod_diff = new double[(h - 2)*(w - 2)];
		for (int i = 0; i < h - 2; i++)
		{
			for (int j = 0; j < w - 2; j++)
			{
				idx = i*(w - 2) + j;
				mod_diff[idx] = diff.at<float>(i + 1, j + 1); //�������ݶȵĲ�ֵ��
			}
		}
		///////////////////////////////////////////////////// Find DST  /////////////////////////////////////////////////////
		solve(img, mod_diff, result);

		delete[] mod_diff;
		delete[] boundary_point;
	}
	//��ʼ����
	void possionedit::initialization(Mat &I, Mat &mask, Mat &wmask)
	{
		grx = Mat(I.size(), CV_32FC3);
		gry = Mat(I.size(), CV_32FC3);
		sgx = Mat(I.size(), CV_32FC3);
		sgy = Mat(I.size(), CV_32FC3);

		split(I, rgb_channel);

		smask = Mat(wmask.size(), CV_32FC1);
		srx32 = Mat(I.size(), CV_32FC3);
		sry32 = Mat(I.size(), CV_32FC3);
		smask1 = Mat(wmask.size(), CV_32FC1);
		grx32 = Mat(I.size(), CV_32FC3);
		gry32 = Mat(I.size(), CV_32FC3);

		//grx, gry �ֱ��ʾdest��x��y������ݶ�
		getGradientX(I, grx);
		getGradientY(I, gry);

		//sgx, sgy �ֱ��ʾ��mask�����ڵ�sourceͼƬ��x��y������ݶ�
		getGradientX(mask, sgx);
		getGradientY(mask, sgy);

		Mat Kernel(Size(3, 3), CV_8UC1);
		Kernel.setTo(Scalar(1));
		//��ʴ����Ҫ����ʴ���Σ���Ϊmask���ݶ���Χ��һƬ��ɫ ������Ҫ��ʴ��ȥ������ȷ��λ�õ��ݶȣ�
		erode(wmask, wmask, Kernel, Point(-1, -1), 3);
	
		wmask.convertTo(smask, CV_32FC1, 1.0 / 255.0);
		I.convertTo(srx32, CV_32FC3, 1.0 / 255.0);
		I.convertTo(sry32, CV_32FC3, 1.0 / 255.0);	
		
	}

	//mat1 = mat3.mul(mat2(:))  mat3Ϊ��ͨ����һ��Ϊmask
	void possionedit::array_product(Mat mat1, Mat mat2, Mat mat3)
	{
		vector <Mat> channels_temp1;
		vector <Mat> channels_temp2;
		split(mat1, channels_temp1);
		split(mat2, channels_temp2);
		multiply(channels_temp2[2], mat3, channels_temp1[2]);
		multiply(channels_temp2[1], mat3, channels_temp1[1]);
		multiply(channels_temp2[0], mat3, channels_temp1[0]);
		merge(channels_temp1, mat1);
	}
	//������һ��ԭʼͼƬ���������ݶȣ�
	void possionedit::poisson(Mat &I, Mat &gx, Mat &gy, Mat &sx, Mat &sy)
	{
		//fx, fy��������ϵ��ݶ�
		Mat fx = Mat(I.size(), CV_32FC3);
		Mat fy = Mat(I.size(), CV_32FC3);

		//�ݶ�ֱ����ϣ��м���һ��������������
		fx = gx + sx;
		fy = gy + sy;

		Mat gxx = Mat(I.size(), CV_32FC3);
		Mat gyy = Mat(I.size(), CV_32FC3);

		//gxx, gyy ����x��y�����laplacian����
		getLaplaceX(fx, gxx);
		getLaplaceY(fy, gyy);

		split(gxx, rgbx_channel);
		split(gyy, rgby_channel);

		split(I, output);

		//��ÿ��ͨ���ֱ���
		poisson_solver(rgb_channel[2], rgbx_channel[2], rgby_channel[2], output[2]);
		poisson_solver(rgb_channel[1], rgbx_channel[1], rgby_channel[1], output[1]);
		poisson_solver(rgb_channel[0], rgbx_channel[0], rgby_channel[0], output[0]);
	}

	void possionedit::evaluate(Mat &I, Mat &wmask, Mat &cloned)
	{
		//maskȡ��
		bitwise_not(wmask, wmask);

		wmask.convertTo(smask1, CV_32FC1, 1.0 / 255.0);
		I.convertTo(grx32, CV_32FC3, 1.0 / 255.0);
		I.convertTo(gry32, CV_32FC3, 1.0 / 255.0);

		//ȡ�������ȫ����
		array_product(grx32, grx, smask1);
		array_product(gry32, gry, smask1);

		poisson(I, grx32, gry32, srx32, sry32);

		merge(output, cloned);
	}
	//���п�¡��
	void possionedit::seamless_clone(Mat &I, Mat &mask, Mat &wmask, Mat &cloned)
	{
		int w = I.size().width;
		int h = I.size().height;
		int channel = I.channels();

		//��ʼ�������洢������mat��������ԭͼ��Ŀ��ͼ��x��y�����ݶȣ���mask��ʴ
		initialization(I, mask, wmask);

		//srx32, sry32��sgx��sgy��mask����
		//�õ��������������ؿ�����

		array_product(srx32, sgx, smask);
		array_product(sry32, sgy, smask);

		evaluate(I, wmask, cloned);
	}
