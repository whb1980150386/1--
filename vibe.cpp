#include "stdafx.h"
#include "vibe.h"  

int c_xoff[9] = { -1, 0, 1, 1, 1, 0, -1, -1, 0 };  //x的邻居点  
int c_yoff[9] = { -1, -1, -1, 0, 1, 1, 1, 0, 0 };  //y的邻居点  

/*  9个邻域点
*
*                A B C
*                H I D
*                G F E
*
*/


ViBe_BGS::ViBe_BGS(void)
{

}
ViBe_BGS::~ViBe_BGS(void)
{

}


/**************** Assign space and init ***************************/
/*
初始化Vibe算法各变量
*/
void ViBe_BGS::init(const Mat _image)
{
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		m_samples[i] = Mat::zeros(_image.size(), CV_8UC1);//刚开始都给0值初始化  
	}
	m_mask = Mat::zeros(_image.size(), CV_8UC1);
	m_foregroundMatchCount = Mat::zeros(_image.size(), CV_8UC1);//前景匹配图像  
}


/**************** Init model from first frame ********************/
void ViBe_BGS::processFirstFrame(const Mat _image)//处理第一帧图像  
{
	RNG rng;//RNG:随机数生成器  
	int row, col;

	for (int i = 0; i < _image.rows; i++)//逐像素处理  
	{
		for (int j = 0; j < _image.cols; j++)
		{
			for (int k = 0; k < NUM_SAMPLES; k++)//取NUM_SAMPLES个采样点  
			{
				// Random pick up NUM_SAMPLES pixel in neighbourhood to construct the model  
				int random = rng.uniform(0, 9);//产生一个0-9的数字  

				row = i + c_yoff[random]; //这里表示产生的随机数会在8领域范围内选择点作为采样点  
				if (row < 0)
					row = 0;
				if (row >= _image.rows)
					row = _image.rows - 1;

				col = j + c_xoff[random];
				if (col < 0)
					col = 0;
				if (col >= _image.cols)
					col = _image.cols - 1;

				m_samples[k].at<uchar>(i, j) = _image.at<uchar>(row, col);
			}
		}
	}
}


/**************** Test a new frame and update model ********************/
void ViBe_BGS::testAndUpdate(const Mat _image)
{
	RNG rng;
	int x, y;
	for (int i = 0; i < _image.rows; i++)
	{
		for (int j = 0; j < _image.cols; j++)
		{
			int matches(0), count(0);
			float dist;

			while (matches < MIN_MATCHES && count < NUM_SAMPLES) //#min指数，最小交集  
			{
				dist = abs(m_samples[count].at<uchar>(i, j) - _image.at<uchar>(i, j));//这先计算里欧氏距离          //???????????????????
				if (dist < RADIUS) //如果在我们设定的采样半径之内，匹配计数+1  
					matches++;
				count++;
			}
		
			if (matches >= MIN_MATCHES)//#min 最小交集符合要求  
			{
				// It is a background pixel  
				m_foregroundMatchCount.at<uchar>(i, j) = _image.at<uchar>(i, j);
				//说明该点与周围点融合得比较好，可以作为背景处理  
				//某个像素点连续N次被检测为前景,则认为一块静止区域被误判为运动，将其更新为背景点。这里需及时清零.  

				// Set background pixel to 0  
				m_mask.at<uchar>(i, j) = 0; //作为图像背景点                                            //中间退出崩溃
				// 如果一个像素是背景点，那么它有 1 / defaultSubsamplingFactor 的概率去更新自己的模型样本值  
				int random = rng.uniform(0, SUBSAMPLE_FACTOR);//  
				if (random == 0) // 1/SUBSAMPLE_FACTOR的概率去更新自己的样本。  
				{
					random = rng.uniform(0, NUM_SAMPLES);
					m_samples[random].at<uchar>(i, j) = _image.at<uchar>(i, j);
				}

				// 同时也有 1 / defaultSubsamplingFactor 的概率去更新它的邻居点的模型样本值  
				random = rng.uniform(0, SUBSAMPLE_FACTOR);
				if (random == 0) // 1/SUBSAMPLE_FACTOR的概率去更新8领域范围内的样本。  
				{
					int row, col;
					random = rng.uniform(0, 9);
					row = i + c_yoff[random];
					if (row < 0)
						row = 0;
					if (row >= _image.rows)
						row = _image.rows - 1;

					random = rng.uniform(0, 9);
					col = j + c_xoff[random];
					if (col < 0)
						col = 0;
					if (col >= _image.cols)
						col = _image.cols - 1;

					random = rng.uniform(0, NUM_SAMPLES);
					m_samples[random].at<uchar>(row, col) = _image.at<uchar>(i, j);
				}
			}
			else //距离太远，色差太大。当前景用。  
			{
				// It is a foreground pixel  
				m_foregroundMatchCount.at<uchar>(i, j)++;

				// Set background pixel to 255  
				m_mask.at<uchar>(i, j) = 255;
				
				//如果某个像素点连续N次被检测为前景，则认为一块静止区域被误判为运动，将其更新为背景点  
				if (m_foregroundMatchCount.at<uchar>(i, j) > 25)
				{
					int random = rng.uniform(0, NUM_SAMPLES);		
					m_samples[random].at<uchar>(i, j) = _image.at<uchar>(i, j);
	
				}
			}
			
		}
		
	}

	
}
