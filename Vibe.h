#ifndef VIBE_H  
#define VIBE_H  

#include "opencv2/opencv.hpp"  

using namespace cv;
using namespace std;


#define NUM_SAMPLES 10      //每个像素点的样本个数  
#define MIN_MATCHES 2       //#min指数 作为检测阈值  
#define RADIUS 10           //Sqthere半径 与像素间的欧氏距离相比较  
#define SUBSAMPLE_FACTOR 16 //子采样概率 有 1/UBSAMPLE_FACTOR 概率更新自己的样本值  

#define background 0        //背景像素  
#define foreground 255      //前景像素  


class ViBe_BGS
{
public:
	ViBe_BGS(void);
	~ViBe_BGS(void);

	void init(const Mat image);             //初始化 分配空间  
	void processFirstFrame(const Mat image);//从第一帧中初始化模型  
	void testAndUpdate(const Mat _image);   //测试新的帧并更新模型  
	Mat getMask(void){ return m_mask; }       //得到处理过的二值图像  

private:
	Mat m_samples[NUM_SAMPLES];  //每个像素有NUM_SAMPLES个采样点  
	Mat m_foregroundMatchCount;  //某个像素点连续N次被检测为前景,则认为一块静止区域被，将其更新为背景点。  
	Mat m_mask; //输出的二值图像  

};

#endif