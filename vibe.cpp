#include "stdafx.h"
#include "vibe.h"  

int c_xoff[9] = { -1, 0, 1, 1, 1, 0, -1, -1, 0 };  //x���ھӵ�  
int c_yoff[9] = { -1, -1, -1, 0, 1, 1, 1, 0, 0 };  //y���ھӵ�  

/*  9�������
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
��ʼ��Vibe�㷨������
*/
void ViBe_BGS::init(const Mat _image)
{
	for (int i = 0; i < NUM_SAMPLES; i++)
	{
		m_samples[i] = Mat::zeros(_image.size(), CV_8UC1);//�տ�ʼ����0ֵ��ʼ��  
	}
	m_mask = Mat::zeros(_image.size(), CV_8UC1);
	m_foregroundMatchCount = Mat::zeros(_image.size(), CV_8UC1);//ǰ��ƥ��ͼ��  
}


/**************** Init model from first frame ********************/
void ViBe_BGS::processFirstFrame(const Mat _image)//�����һ֡ͼ��  
{
	RNG rng;//RNG:�����������  
	int row, col;

	for (int i = 0; i < _image.rows; i++)//�����ش���  
	{
		for (int j = 0; j < _image.cols; j++)
		{
			for (int k = 0; k < NUM_SAMPLES; k++)//ȡNUM_SAMPLES��������  
			{
				// Random pick up NUM_SAMPLES pixel in neighbourhood to construct the model  
				int random = rng.uniform(0, 9);//����һ��0-9������  

				row = i + c_yoff[random]; //�����ʾ���������������8����Χ��ѡ�����Ϊ������  
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

			while (matches < MIN_MATCHES && count < NUM_SAMPLES) //#minָ������С����  
			{
				dist = abs(m_samples[count].at<uchar>(i, j) - _image.at<uchar>(i, j));//���ȼ�����ŷ�Ͼ���          //???????????????????
				if (dist < RADIUS) //����������趨�Ĳ����뾶֮�ڣ�ƥ�����+1  
					matches++;
				count++;
			}
		
			if (matches >= MIN_MATCHES)//#min ��С��������Ҫ��  
			{
				// It is a background pixel  
				m_foregroundMatchCount.at<uchar>(i, j) = _image.at<uchar>(i, j);
				//˵���õ�����Χ���ںϵñȽϺã�������Ϊ��������  
				//ĳ�����ص�����N�α����Ϊǰ��,����Ϊһ�龲ֹ��������Ϊ�˶����������Ϊ�����㡣�����輰ʱ����.  

				// Set background pixel to 0  
				m_mask.at<uchar>(i, j) = 0; //��Ϊͼ�񱳾���                                            //�м��˳�����
				// ���һ�������Ǳ����㣬��ô���� 1 / defaultSubsamplingFactor �ĸ���ȥ�����Լ���ģ������ֵ  
				int random = rng.uniform(0, SUBSAMPLE_FACTOR);//  
				if (random == 0) // 1/SUBSAMPLE_FACTOR�ĸ���ȥ�����Լ���������  
				{
					random = rng.uniform(0, NUM_SAMPLES);
					m_samples[random].at<uchar>(i, j) = _image.at<uchar>(i, j);
				}

				// ͬʱҲ�� 1 / defaultSubsamplingFactor �ĸ���ȥ���������ھӵ��ģ������ֵ  
				random = rng.uniform(0, SUBSAMPLE_FACTOR);
				if (random == 0) // 1/SUBSAMPLE_FACTOR�ĸ���ȥ����8����Χ�ڵ�������  
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
			else //����̫Զ��ɫ��̫�󡣵�ǰ���á�  
			{
				// It is a foreground pixel  
				m_foregroundMatchCount.at<uchar>(i, j)++;

				// Set background pixel to 255  
				m_mask.at<uchar>(i, j) = 255;
				
				//���ĳ�����ص�����N�α����Ϊǰ��������Ϊһ�龲ֹ��������Ϊ�˶����������Ϊ������  
				if (m_foregroundMatchCount.at<uchar>(i, j) > 25)
				{
					int random = rng.uniform(0, NUM_SAMPLES);		
					m_samples[random].at<uchar>(i, j) = _image.at<uchar>(i, j);
	
				}
			}
			
		}
		
	}

	
}
