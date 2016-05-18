#ifndef VIBE_H  
#define VIBE_H  

#include "opencv2/opencv.hpp"  

using namespace cv;
using namespace std;


#define NUM_SAMPLES 10      //ÿ�����ص����������  
#define MIN_MATCHES 2       //#minָ�� ��Ϊ�����ֵ  
#define RADIUS 10           //Sqthere�뾶 �����ؼ��ŷ�Ͼ�����Ƚ�  
#define SUBSAMPLE_FACTOR 16 //�Ӳ������� �� 1/UBSAMPLE_FACTOR ���ʸ����Լ�������ֵ  

#define background 0        //��������  
#define foreground 255      //ǰ������  


class ViBe_BGS
{
public:
	ViBe_BGS(void);
	~ViBe_BGS(void);

	void init(const Mat image);             //��ʼ�� ����ռ�  
	void processFirstFrame(const Mat image);//�ӵ�һ֡�г�ʼ��ģ��  
	void testAndUpdate(const Mat _image);   //�����µ�֡������ģ��  
	Mat getMask(void){ return m_mask; }       //�õ�������Ķ�ֵͼ��  

private:
	Mat m_samples[NUM_SAMPLES];  //ÿ��������NUM_SAMPLES��������  
	Mat m_foregroundMatchCount;  //ĳ�����ص�����N�α����Ϊǰ��,����Ϊһ�龲ֹ���򱻣��������Ϊ�����㡣  
	Mat m_mask; //����Ķ�ֵͼ��  

};

#endif