#include "opencv/cv.h"
#include "opencv/highgui.h"
#include<math.h>
#include"possionedit.h"
#include"patchmatch.h"
class ImagePro{
public:
	ImagePro();
	~ImagePro();
	//处理鼠标事件：
	static void writeMask(cv::Mat& ,cv::Mat& ,cv::Mat& ,int , int ,double);
	static void PossionEditing(cv::Mat& ,cv::Mat& ,cv::Mat&,cv::Mat &,cv::Mat& );
	static void PatchMatch(cv::Mat& ,cv::Mat& ,cv::Mat&);
};