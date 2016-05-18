/*******************************************************
Copyright 2008-2011 Digital Technology Co., Ltd.
��  ����	RealPlayDlg.cpp
������λ��	����
��  д��	shizhiping
��  �ڣ�	2009.5
��  ����	ʵʱԤ���Ի���
��  �ģ�
********************************************************/

#include "stdafx.h"
#include "RealPlay.h"
#include "RealPlayDlg.h"
#include "DlgPTZCruise.h"
#include <iostream>
#include <highgui.h>
#include <opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include <cv.h>
#include "CvvImage.h"
#include "vibe.h" 

using namespace cv;
using namespace std;

#define USECOLOR 1 

#define WM_SHOW_END_IMAGE (WM_USER + 1002)
#define WM_TEST_SHOWIPC  (WM_USER + 1001)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define FishHeight               1260/3/16*9
#define FishWidth                1260/3
#define IPHeight                 1260/3/4*3
#define IPWidth                 1260/3

int inMonitor = 0;
const int CONTOUR_MAX_AERA = 20;
int countsInMonitor = 0;
IplImage* motion = 0;
//Mat mat;
int countGray = 0;
ViBe_BGS Vibe_Bgs;
BOOL DrawRange_flag = FALSE;
//��ʼ�����������¼���־
BOOL RecordFish_flag = FALSE;
//¼��ť�����־
BOOL ButtonRecordFish_flag = FALSE;
int x1;
int y11;
int a1;
int b1;
int c1;
int d1;
double w1;
double k1;
double z1;
double o1;
double p1;
CPoint m_startPoint1;
CPoint m_OldPoint1;
BOOL m_startRect1 = FALSE;
char buffer[20];

int iPicNum = 0;//Set channel NO.
LONG nPort = -1;
LONG nPort1 = -1;
FILE *Videofile = NULL;
FILE *Audiofile = NULL;
char filename[100];
CString ErrorNum;
CRect rectItem;
BOOL start_init = FALSE;
int start;
void *cRPD;
CvVideoWriter* writer;
LPVOID testa[300];
int countx = 0;
int countx1 = 0;
BOOL AUTOSIZE = FALSE;
CWinThread* MyThread = NULL;
BOOL FishRecord = FALSE;
HWND PicHwnd;

void TestOutline(IplImage* src, IplImage* dst);
//��������ͼ��
bool YV12_get_RGB24(unsigned char* pYV12, IplImage *pRGB24, IplImage *Defish, int iWidth, int iHeight, float k, float d)
{
	float xq;
	float yq;
	float L;
	float theta;
	float fd = 5;
	float R;
	float xc, yc;
	xc = (Defish->width) / 2;
	yc = (Defish->height) / 2;
	int x, y;

	const long nYLen = long(iHeight * iWidth);
	const int nHfWidth = (iWidth >> 1);
	unsigned char* yData = pYV12;//y���׵�ַ
	unsigned char* vData = &yData[nYLen];//u���׵�ַ
	unsigned char* uData = &vData[nYLen >> 2];  //v���׵�ַ 
	int rgb[3];
	int i1, j1, m1, n1, x1, y1;
	m1 = -iWidth;
	n1 = -nHfWidth;
	for (y1 = 0; y1 < Defish->height; y1++)
	{
		m1 += iWidth;  //��һ�е�һ��Ԫ�ص�����
		if (!(y1 % 2))
			n1 += nHfWidth;
		for (x1 = 0; x1 < Defish->width; x1++)
		{

			xq = (float)(x1 - xc);
			yq = (float)(y1 - yc);
			L = sqrt(xq*xq + yq*yq);
			if (L == 0){ continue; }
			theta = atan(L / d);
			R = d*k*sin(theta) / sqrt(1 - k*k * sin(theta)*sin(theta));
			y = int((y1 - yc)*R / L*fd + iHeight / 2);   // ��
			x = int((x1 - xc)*R / L*fd + iWidth / 2);   // ��

			if ((x < 0) || (x >= iWidth) || (y < 0) || (y >= iHeight))
			{
				continue;
			}

			i1 = y * iWidth + x;
			j1 = (y / 2 ) * (iWidth / 2) + (x / 2 );

			rgb[2] = int(yData[i1] + 1.370705 * (vData[j1] - 128)); // r����ֵ
			rgb[1] = int(yData[i1] - 0.698001 * (uData[j1] - 128) - 0.703125 * (vData[j1] - 128)); // g����ֵ
			rgb[0] = int(yData[i1] + 1.732446 * (uData[j1] - 128)); // b����ֵ
			for (int j = 0; j < 3; j++)
			{
				if (rgb[j] >= 0 && rgb[j] <= 255 && (x > 0) && (x<iWidth) && (y>0) && (y < iHeight))
				{

					pRGB24->imageData[pRGB24->widthStep*(y)+(x * 3 + j)] = rgb[j];

					if ((x > 0) && (x<iWidth) && (y>0) && (y < iHeight))
					{
						Defish->imageData[Defish->widthStep*(y1)+(x1 * 3 + j)] = pRGB24->imageData[pRGB24->widthStep*(y)+(x * 3 + j)];
					}
					else
					{
						((uchar*)(Defish->imageData + Defish->widthStep*y1))[x1 * 3 + j] = 255;
					}
				}
				else
				{
					pRGB24->imageData[i1 + j] = (rgb[j] < 0) ? 0 : 255;

				}
			}


		}
	}
	yData = NULL;
	vData = NULL;
	uData = NULL;
	return true;
}

////����ص����� ��ƵΪYUV����(YV12)����ƵΪPCM����
void CALLBACK DecCBFun(long nPort, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{

	/*if (pFrameInfo->dwFrameNum % 8 == 0)
	{*/

//		TRACE("֡��   %d   ֡��   %d\n", pFrameInfo->nFrameRate, pFrameInfo->dwFrameNum);

		long lFrameType = pFrameInfo->nType;

		if (lFrameType == T_YV12)
		{
#if USECOLOR  
			IplImage*  pImgYCrCb = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), IPL_DEPTH_8U, 3);//�õ�ͼ���Y����    
			unsigned char * fishimg;
			fishimg = (unsigned char *)pBuf;

			IplImage* Defish = cvCreateImage(cvSize(pFrameInfo->nWidth / 2, pFrameInfo->nHeight /2), IPL_DEPTH_8U, 3);

			//		int start = clock();
			YV12_get_RGB24(fishimg, pImgYCrCb, Defish, pFrameInfo->nWidth, pFrameInfo->nHeight, 0.4, 500);

			SendMessage(((CRealPlayDlg *)cRPD)->m_hWnd, WM_SHOW_END_IMAGE, 0, (LPARAM)Defish);
			// 		int end = clock();
			// 		TRACE("============   %d\n", end - start);

#else  
			IplImage* pImg = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), IPL_DEPTH_8U, 1);
			memcpy(pImg->imageData, pBuf, pFrameInfo->nWidth*pFrameInfo->nHeight);

#endif  
			//cvShowImage("ipimage", Defish);	

			cvWaitKey(1);
#if USECOLOR  
			cvReleaseImage(&pImgYCrCb);
			cvReleaseImage(&Defish);
			pImgYCrCb = NULL;
			Defish = NULL;

#else  
			cvReleaseImage(&pImgYCrCb);
			cvReleaseImage(&pImg);

#endif  

		}
//	}
}

///ʵʱ���ص�
void CALLBACK fRealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, DWORD pUser)
{

	//		TRACE("��ȡԴ������ʣ������ ��С   %d\n", PlayM4_GetSourceBufferRemain(nPort));
	DWORD dRet = 0;
	BOOL inData = FALSE;

	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD:
		if (nPort >= 0)
		{
			break; //ͬһ·��������Ҫ��ε��ÿ����ӿ�
		}

		if (!PlayM4_GetPort(&nPort))
		{
			break;
		}
		if (!PlayM4_OpenStream(nPort, pBuffer, dwBufSize, 2560 * 1440))
		{
			dRet = PlayM4_GetLastError(nPort);
			break;
		}

		// 			//���ý�������   1ֻ��ؼ�֡    ����ҳ������I֡���Ϊ5
		 			if (!PlayM4_SetDecodeFrameType( nPort,1))
		 			{
		 				dRet = PlayM4_GetLastError(nPort);
						break;
		 			}

		// 		���ý���ص����� ֻ���벻��ʾ
		if (!PlayM4_SetDecCallBack(nPort, DecCBFun))
		{
			dRet = PlayM4_GetLastError(nPort);
			break;
		}
		//����ص�������     1��Ƶ���� 2��Ƶ���� 3������
		PlayM4_SetDecCBStream(nPort, 1);

		//����Ƶ����
		if (!PlayM4_Play(nPort, NULL))
		{
			dRet = PlayM4_GetLastError(nPort);
			break;
		}

	case NET_DVR_STREAMDATA:
		inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);

		while (!inData)
		{
			break;
			// 			Sleep(10);
			// 			inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
			OutputDebugString("PlayM4_InputData failed \n");
		}
		break;
	default:
		inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
		while (!inData)
		{

			break;
			OutputDebugString("PlayM4_InputData failed \n");
		}
		break;
	}

}
bool YV12_to_RGB24(unsigned char* pYV12, unsigned char* pRGB24, int iWidth, int iHeight)
{
	if (!pYV12 || !pRGB24)
		return false;
	const long nYLen = long(iHeight * iWidth);
	const int nHfWidth = (iWidth >> 1);
	if (nYLen < 1 || nHfWidth < 1)
		return false;
	unsigned char* yData = pYV12;//y���׵�ַ
	unsigned char* vData = &yData[nYLen];//u���׵�ַ
	unsigned char* uData = &vData[nYLen >> 2];  //v���׵�ַ 
	if (!uData || !vData)
		return false;
	// Convert YV12 to RGB24
	int rgb[3];
	int i, j, m, n, x, y;
	m = -iWidth;
	n = -nHfWidth;
	for (y = 0; y < iHeight; y++)
	{
		m += iWidth;  //��һ�е�һ��Ԫ�ص�����
		if (!(y % 2))
			n += nHfWidth;
		for (x = 0; x < iWidth; x++)
		{
			i = m + x;
			j = n + (x >> 1);
			rgb[2] = int(yData[i] + 1.370705 * (vData[j] - 128)); // r����ֵ
			rgb[1] = int(yData[i] - 0.698001 * (uData[j] - 128) - 0.703125 * (vData[j] - 128)); // g����ֵ
			rgb[0] = int(yData[i] + 1.732446 * (uData[j] - 128)); // b����ֵ
			j = nYLen - iWidth - m + x;
			i = (j << 1) + j;
			for (int j = 0; j < 3; j++)
			{
				if (rgb[j] >= 0 && rgb[j] <= 255)
					pRGB24[i + j] = rgb[j];
				else
					pRGB24[i + j] = (rgb[j] < 0) ? 0 : 255;
			}
		}
	}
	yData = NULL;
	vData = NULL;
	uData = NULL;
	return true;
}
//����ص�IPC
void CALLBACK DecCBFunIPC(long nPort1, char * pBuf, long nSize, FRAME_INFO * pFrameInfo, long nReserved1, long nReserved2)
{
	/*if (pFrameInfo->dwFrameNum % 8 == 0)
	{*/
		Mat mat,gray, mask;
		IplImage*  pImgYCrCb = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), IPL_DEPTH_8U, 3);//�õ�ͼ���Y����    
		unsigned char * pImgimageData;
		pImgimageData = (unsigned char *)pImgYCrCb->imageData;
		unsigned char * fishimg;
		fishimg = (unsigned char *)pBuf;

		int qw1 = clock();
		YV12_to_RGB24(fishimg, pImgimageData, pFrameInfo->nWidth, pFrameInfo->nHeight);

		int qw2 = clock();
		TRACE("YV12_to_RGB24ת��ʱ��  %d\n", qw2 - qw1);

		//��תͼ��
		IplImage*  pImgYCrCb1 = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight),
			IPL_DEPTH_8U, 3);
		cvConvertImage(pImgYCrCb, pImgYCrCb1, CV_CVTIMG_FLIP);
		//		cvShowImage("1111",pImgYCrCb1);

		//��Сͼ��
		IplImage *dst1 = 0;   //Ŀ��ͼ��ָ��1  
		float scale1 = 0.255;  //����ΪԭʼͼƬ��scale1��С  
		CvSize dst1_cvsize;   //Ŀ��ͼ��1�ߴ�   
		dst1_cvsize.width = pImgYCrCb1->width * scale1;  //Ŀ��ͼ��1�Ŀ�ΪԴͼ����scale1��  
		dst1_cvsize.height = pImgYCrCb1->height * scale1; //Ŀ��ͼ��1�ĸ�ΪԴͼ��ߵ�scale1��  
		dst1 = cvCreateImage(dst1_cvsize, pImgYCrCb1->depth, pImgYCrCb1->nChannels); //����Ŀ��ͼ��1  
		cvResize(pImgYCrCb1, dst1, CV_INTER_LINEAR); //����Դͼ��Ŀ��ͼ��1  
		//	cvShowImage("src", pImgYCrCb1);  //��ʾԴͼ��  
		//		cvShowImage("dst1", dst1);  //��ʾĿ��ͼ��1 

		//�ƶ����
		mat = cvarrToMat(dst1);        //IplImageתMat
		countGray++;
		cvtColor(mat, gray, CV_RGB2GRAY);//ɫ�ʿռ�ת��  ת��Ϊ�Ҷ�ͼ
		if (countGray == 1)//�����һ֡  
		{
			Vibe_Bgs.init(gray);
			Vibe_Bgs.processFirstFrame(gray);
			cout << " Training GMM complete!" << endl;
		}
		else //��������  
		{
			Vibe_Bgs.testAndUpdate(gray);
			mask = Vibe_Bgs.getMask();			
			morphologyEx(mask, mask, MORPH_OPEN, Mat());
			//ǰ��ͼ
//			imshow("result", mask);
			
			IplImage *image2 = (&(IplImage)mask);
			IplImage* dst = cvCreateImage(cvGetSize(image2), 8, 3);
			//��������
			TestOutline(image2, dst);
		
			           ////��ͨ��ת��ͨ�� �ı���ͨ����ɫ
			//IplImage* canny_Img = cvCreateImage(cvSize(mask.cols, mask.rows), IPL_DEPTH_8U, 3);//canny_Imag��3ͨ��ͼ  
			//cvCvtColor(image2, canny_Img, CV_GRAY2BGR);//���б任  	
			//Mat mat1;
			//mat1 = cvarrToMat(canny_Img);			
			//for (int i = 0; i < mask.rows; i++)
			//{
			//	for (int j = 0; j < mask.cols; j++)
			//	{
			//		if ((mask.at<uchar>(i, j) == 255))
			//		{
			//			mat1.at<uchar>(i, j * 3) = 0;
			//			mat1.at<uchar>(i, j * 3 + 1) = 255; //����ɫ��Ϊ��ɫ
			//			mat1.at<uchar>(i, j * 3+2) = 0;
			//		}
			//		
			//	}
			//}

			//IPlImageתMat
			Mat mat1;
			mat1 = cvarrToMat(dst);

         //��ǰ���뱳���ں�
			cv::Mat imageROI;
			imageROI = mat(cv::Rect(0, 0, mat1.cols, mat1.rows));
			mat1.copyTo(imageROI, mat1);
//			cv::imshow("result11111", mat);                
			//������Ϣ   ��ʾͼ��
			IplImage *image3 = (&(IplImage)mat);
			SendMessage(((CRealPlayDlg *)cRPD)->m_hWnd, WM_TEST_SHOWIPC, 0, (LPARAM)image3);
			cvReleaseImage(&dst);
			dst = NULL;
			image2 = NULL;
			image3 = NULL;
		}
		cvWaitKey(1);
		cvReleaseImage(&pImgYCrCb);
		cvReleaseImage(&pImgYCrCb1);
		cvReleaseImage(&dst1);
		pImgYCrCb = NULL;
		pImgYCrCb1 = NULL;
		dst1 = NULL;		
		fishimg = NULL;
		pImgimageData = NULL;
	
}
///ʵʱ���ص�
void CALLBACK fRealDataCallBackIPC(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, DWORD pUser)
{
	DWORD dRet = 0;
	BOOL inData = FALSE;

	switch (dwDataType)
	{
	case NET_DVR_SYSHEAD:
		if (nPort1 >= 0)
		{
			break; //ͬһ·��������Ҫ��ε��ÿ����ӿ�
		}

		if (!PlayM4_GetPort(&nPort1))
		{
			break;
		}
		if (!PlayM4_OpenStream(nPort1, pBuffer, dwBufSize, 2560 * 1440))
		{
			dRet = PlayM4_GetLastError(nPort1);
			break;
		}
		//������󻺳�֡��
	//	PlayM4_SetDisplayBuf(nPort1, 12);
	


		// 			//���ý�������   1ֻ��ؼ�֡       ����ҳ������I֡���Ϊ5
		if (!PlayM4_SetDecodeFrameType(nPort1, 1))
			 			{
			 				dRet = PlayM4_GetLastError(nPort1);
			 				break;
			 			}

		// 		���ý���ص����� ֻ���벻��ʾ
		if (!PlayM4_SetDecCallBack(nPort1, DecCBFunIPC))
		{
			dRet = PlayM4_GetLastError(nPort1);
			break;
		}
		//����ص�������     1��Ƶ���� 2��Ƶ���� 3������
		PlayM4_SetDecCBStream(nPort1, 1);

		//����Ƶ����
		if (!PlayM4_Play(nPort1, NULL))
		{
			dRet = PlayM4_GetLastError(nPort1);
			break;
		}

	case NET_DVR_STREAMDATA:
		inData = PlayM4_InputData(nPort1, pBuffer, dwBufSize);
		

		while (!inData)
		{
			break;
			// 			Sleep(10);
			// 			inData = PlayM4_InputData(nPort, pBuffer, dwBufSize);
			OutputDebugString("PlayM4_InputData failed \n");
		}
	//	PlayM4_ResetBuffer(nPort1, BUF_VIDEO_RENDER);
		
		break;
	default:
		inData = PlayM4_InputData(nPort1, pBuffer, dwBufSize);
		while (!inData)
		{
			break;
			OutputDebugString("PlayM4_InputData failed \n");
		}
		break;
	}

	 
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	//afx_msg LRESULT DrawIplImage2DC(WPARAM wParam, LPARAM lParam);
	//	afx_msg LRESULT OnDrawIplimage(WPARAM wParam, LPARAM lParam);
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	// No message handlers
	//}}AFX_MSG_MAP
	//ON_MESSAGE(WM_DRAWIPLIMAGE, &CAboutDlg::DrawIplImage2DC)
	//	ON_MESSAGE(WM_DRAWIPLIMAGE, &CAboutDlg::OnDrawIplimage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRealPlayDlg dialog

CRealPlayDlg::CRealPlayDlg(CWnd* pParent /*=NULL*/)
: CDialog(CRealPlayDlg::IDD, pParent)
{

	//{{AFX_DATA_INIT(CRealPlayDlg)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_nDevPort = 8000;
	m_csUser = _T("admin");
	m_csPWD = _T("admin123");
	m_bIsLogin = FALSE;
	m_bIsPlaying = FALSE;
	m_bIsPlaying1 = FALSE;
	m_bIsRecording = FALSE;
	m_iCurChanIndex = -1;
	m_lPlayHandle = -1;
	m_lPlayHandle1 = -1;
	m_bIsOnCruise = FALSE;
	m_bTrackRun = FALSE;
	m_bAutoOn = FALSE;
	m_bLightOn = FALSE;
	m_bWiperOn = FALSE;
	m_bFanOn = FALSE;
	m_bHeaterOn = FALSE;
	m_bAuxOn1 = FALSE;
	m_bAuxOn2 = FALSE;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRealPlayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRealPlayDlg)
	DDX_Control(pDX, IDC_COMBO_JPG_QUALITY, m_coJpgQuality);
	DDX_Control(pDX, IDC_COMBO_JPG_SIZE, m_coJpgSize);
	DDX_Control(pDX, IDC_COMBO_PIC_TYPE, m_coPicType);
	DDX_Control(pDX, IDC_COMBO_SEQ, m_comboSeq);
	DDX_Control(pDX, IDC_COMBO_PRESET, m_comboPreset);
	DDX_Control(pDX, IDC_COMBO_PTZ_SPEED, m_comboPTZSpeed);
	DDX_Control(pDX, IDC_TREE_CHAN, m_ctrlTreeChan);
	DDX_Control(pDX, IDC_IPADDRESS_DEV, m_ctrlDevIp);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nDevPort);
	DDX_Text(pDX, IDC_EDIT_USER, m_csUser);
	DDX_Text(pDX, IDC_EDIT_PWD, m_csPWD);




	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC_PLAY2, m_showRectangle);
	//DDX_Control(pDX, IDC_BUTTON2, img);
}

BEGIN_MESSAGE_MAP(CRealPlayDlg, CDialog)
	//{{AFX_MSG_MAP(CRealPlayDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, OnButtonLogin)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_CHAN, OnDblclkTreeChan)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_CHAN, OnSelchangedTreeChan)
	ON_BN_CLICKED(IDC_BUTTON_RECORD, OnButtonRecord)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, OnButtonCapture)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_CBN_SELCHANGE(IDC_COMBO_PRESET, OnSelchangeComboPreset)
	ON_BN_CLICKED(IDC_BUTTON_PRESET_GOTO, OnButtonPresetGoto)
	ON_BN_CLICKED(IDC_BUTTON_PRESET_SET, OnButtonPresetSet)
	ON_BN_CLICKED(IDC_BUTTON_PRESET_DEL, OnButtonPresetDel)
	ON_BN_CLICKED(IDC_BUTTON_SEQ_GOTO, OnButtonSeqGoto)
	ON_BN_CLICKED(IDC_BUTTON_SEQ_SET, OnButtonSeqSet)
	ON_BN_CLICKED(IDC_BUTTON_TRACK_RUN, OnButtonTrackRun)
	ON_BN_CLICKED(IDC_BUTTON_TRACK_START, OnButtonTrackStart)
	ON_BN_CLICKED(IDC_BUTTON_TRACK_STOP, OnButtonTrackStop)
	ON_BN_CLICKED(IDC_BTN_PTZ_AUTO, OnBtnPtzAuto)
	ON_CBN_SELCHANGE(IDC_COMBO_PIC_TYPE, OnSelchangeComboPicType)
	ON_WM_CLOSE()
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS_DEV, &CRealPlayDlg::OnIpnFieldchangedIpaddressDev)
	ON_WM_MOVING()
	ON_WM_NCXBUTTONDOWN()
	ON_WM_MENURBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCMBUTTONDOWN()
	ON_WM_NCMBUTTONUP()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_BUTTON1, &CRealPlayDlg::OnBnClickedButton1)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_NCMBUTTONDOWN()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOVE()
	ON_MESSAGE(WM_SHOW_END_IMAGE, &CRealPlayDlg::OnShowEndImage)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON2, &CRealPlayDlg::OnBnClickedButton2)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON3, &CRealPlayDlg::OnBnClickedButton3)
	ON_WM_TIMER()
	ON_MESSAGE(WM_TEST_SHOWIPC, &CRealPlayDlg::OnTestShowipc)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRealPlayDlg message handlers

BOOL CRealPlayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.

	flag_First = false;
	//��edit control����label
	GetDlgItem(IDC_EDIT1_IP)->SetWindowTextA("Dev IP");
	GetDlgItem(IDC_EDIT1_IP2_Port)->SetWindowTextA("Port");
	GetDlgItem(IDC_EDIT1_IP2_User)->SetWindowTextA("User");
	GetDlgItem(IDC_EDIT1_IP2_Pwd)->SetWindowTextA("Password");
	GetDlgItem(IDC_EDIT1_IP2_Speed)->SetWindowTextA("��̨�ٶ�");
	GetDlgItem(IDC_EDIT1_IP2_Notes)->SetWindowTextA("�켣��¼");
	GetDlgItem(IDC_EDIT1_IP2_Preset)->SetWindowTextA("Ԥ�õ�");
	GetDlgItem(IDC_EDIT1_IP2_Route)->SetWindowTextA("Ѳ��·��");
	GetDlgItem(IDC_EDIT1_IP2_JEPG)->SetWindowTextA("JEPG����");


	//�������
//	::ShowWindow(this->m_hWnd, SW_SHOWMAXIMIZED);

	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_STATIC_PLAY2);
	pWnd->SetWindowPos(NULL, 0, 0, FishWidth, FishHeight, SWP_NOMOVE);
	CWnd *pWnd1;
	pWnd1 = GetDlgItem(IDC_STATIC_PLAY);
	pWnd1->SetWindowPos(NULL, 0, 0, IPWidth, IPHeight, SWP_NOMOVE);  // IDC_STATIC_Z


	//	GetDlgItem(IDC_STATIC_PLAY2)->EnableWindow(FALSE);
	// 	SetWindowLong(m_hWnd, GWL_EXSTYLE,
	// 		GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	// 	// Make this window xx% alpha
	// 	SetLayeredWindowAttributes(0, (255 * 40) / 100, LWA_ALPHA);

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	//Ĭ��IP
	m_ctrlDevIp.SetAddress(192, 168, 1, 64);

	//��̨�ٶ�
	m_comboPTZSpeed.SetCurSel(5);

	//��̨��ť
	m_btnPtzUp.SubclassDlgItem(IDC_BTN_PTZ_UP, this);
	m_btnPtzDown.SubclassDlgItem(IDC_BTN_PTZ_DOWN, this);
	m_btnPtzLeft.SubclassDlgItem(IDC_BTN_PTZ_LEFT, this);
	m_btnPtzRight.SubclassDlgItem(IDC_BTN_PTZ_RIGHT, this);
	m_btnZoomOut.SubclassDlgItem(IDC_BTN_ZOOM_OUT, this);
	m_btnZoomIn.SubclassDlgItem(IDC_BTN_ZOOM_IN, this);
	m_btnFocusNear.SubclassDlgItem(IDC_BTN_FOCUS_NEAR, this);
	m_btnFocusFar.SubclassDlgItem(IDC_BTN_FOCUS_FAR, this);
	m_btnIrisOpen.SubclassDlgItem(IDC_BTN_IRIS_OPEN, this);
	m_btnIrisClose.SubclassDlgItem(IDC_BTN_IRIS_CLOSE, this);
	m_btnPtzUpleft.SubclassDlgItem(IDC_BTN_PTZ_UPLEFT, this);
	m_btnPtzUpright.SubclassDlgItem(IDC_BTN_PTZ_UPRIGHT, this);
	m_btnPtzDownleft.SubclassDlgItem(IDC_BTN_PTZ_DOWNLEFT, this);
	m_btnPtzDownright.SubclassDlgItem(IDC_BTN_PTZ_DOWNRIGHT, this);

	//ץͼcombo
	m_coPicType.SetCurSel(0);
	m_coJpgSize.SetCurSel(0);
	m_coJpgQuality.SetCurSel(0);
//	GetDlgItem(IDC_STATIC_JPGPARA)->EnableWindow(TRUE);
	m_coJpgSize.EnableWindow(TRUE);
	m_coJpgQuality.EnableWindow(TRUE);



	//����������
	start_init = TRUE;
	AUTOSIZE = TRUE;
	//�˴�����  
	//CRect rect;
	//GetClientRect(&rect);     //ȡ�ͻ�����С    
	//old.x = rect.right - rect.left;
	//old.y = rect.bottom - rect.top;
	/*int cx = GetSystemMetrics(SM_CXFULLSCREEN);
	int cy = GetSystemMetrics(SM_CYFULLSCREEN);
	CRect rt;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rt, 0);
	cy = rt.bottom;*/


	
	//���ô���λ�á���С
	SetWindowPos(NULL, 0, 0, 1260, 730, SWP_SHOWWINDOW);
	

	pWnd = NULL;
	pWnd1 = NULL;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRealPlayDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRealPlayDlg::OnPaint()
{

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRealPlayDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;
}

/*************************************************
������:    	OnButtonLogin
��������:	ע��/ע�� ��ť
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonLogin()
{
	if (!m_bIsLogin)    //login
	{
		if (!DoLogin())
			return;
		DoGetDeviceResoureCfg();  //��ȡ�豸��Դ��Ϣ	
		CreateDeviceTree();        //����ͨ����
		GetDecoderCfg();                           //��ȡ��̨��������Ϣ
		InitDecoderReferCtrl();         //��ʼ����������ؿؼ�      
		GetDlgItem(IDC_BUTTON_LOGIN)->SetWindowText("Logout");
		m_bIsLogin = TRUE;
	}
	else      //logout
	{
		exit(0);
	}
}

/*************************************************
������:    	DoLogin
��������:	���豸ע��
�������:
�������:
����ֵ:
**************************************************/
BOOL CRealPlayDlg::DoLogin()
{

	UpdateData(TRUE);
	CString DeviceIp;
	BYTE nField0, nField1, nField2, nField3;
	m_ctrlDevIp.GetAddress(nField0, nField1, nField2, nField3);
	DeviceIp.Format("%d.%d.%d.%d", nField0, nField1, nField2, nField3);

	NET_DVR_DEVICEINFO_V30 DeviceInfoTmp;
	memset(&DeviceInfoTmp, 0, sizeof(NET_DVR_DEVICEINFO_V30));
	NET_DVR_DEVICEINFO_V30 DeviceInfoTmp1;
	memset(&DeviceInfoTmp1, 0, sizeof(NET_DVR_DEVICEINFO_V30));


	LONG lLoginID = NET_DVR_Login_V30(DeviceIp.GetBuffer(DeviceIp.GetLength()), m_nDevPort, \
		m_csUser.GetBuffer(m_csUser.GetLength()), m_csPWD.GetBuffer(m_csPWD.GetLength()), &DeviceInfoTmp);
	LONG lLoginID1 = NET_DVR_Login_V30("192.168.1.65", m_nDevPort, \
		m_csUser.GetBuffer(m_csUser.GetLength()), m_csPWD.GetBuffer(m_csPWD.GetLength()), &DeviceInfoTmp1);
	if (lLoginID == -1 && lLoginID1 == -1)
	{
		MessageBox("Login to Device failed!\n");
		return FALSE;
	}

	m_struDeviceInfo.lLoginID = lLoginID;
	m_struDeviceInfo.iDeviceChanNum = DeviceInfoTmp.byChanNum;
	m_struDeviceInfo.iIPChanNum = DeviceInfoTmp.byIPChanNum;
	m_struDeviceInfo.iStartChan = DeviceInfoTmp.byStartChan;
	m_struDeviceInfo.iIPStartChan = DeviceInfoTmp.byStartDChan;

	m_struDeviceInfo1.lLoginID = lLoginID1;
	m_struDeviceInfo1.iDeviceChanNum = DeviceInfoTmp1.byChanNum;
	m_struDeviceInfo1.iIPChanNum = DeviceInfoTmp1.byIPChanNum;
	m_struDeviceInfo1.iStartChan = DeviceInfoTmp1.byStartChan;
	m_struDeviceInfo1.iIPStartChan = DeviceInfoTmp1.byStartDChan;

	return TRUE;
}


/*************************************************
������:    	DoGetDeviceResoureCfg
��������:	��ȡ�豸��ͨ����Դ
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::DoGetDeviceResoureCfg()
{
	NET_DVR_IPPARACFG_V40 IpAccessCfg;
	memset(&IpAccessCfg, 0, sizeof(IpAccessCfg));
	DWORD  dwReturned;

	m_struDeviceInfo.bIPRet = NET_DVR_GetDVRConfig(m_struDeviceInfo.lLoginID, NET_DVR_GET_IPPARACFG_V40, 0, &IpAccessCfg, sizeof(NET_DVR_IPPARACFG_V40), &dwReturned);
	//m_struDeviceInfo.bIPRet = NET_DVR_GetDVRConfig(m_struDeviceInfo1.lLoginID, NET_DVR_GET_IPPARACFG_V40, 0, &IpAccessCfg, sizeof(NET_DVR_IPPARACFG_V40), &dwReturned);

	int i;
	if (!m_struDeviceInfo.bIPRet || !m_struDeviceInfo1.bIPRet)   //��֧��ip����,9000�����豸��֧�ֽ���ģ��ͨ��
	{
		for (i = 0; i < MAX_ANALOG_CHANNUM; i++)
		{
			if (i < m_struDeviceInfo.iDeviceChanNum + m_struDeviceInfo1.iDeviceChanNum)
			{
				sprintf(m_struDeviceInfo.struChanInfo[i].chChanName, "IPcamera%d", i + m_struDeviceInfo.iStartChan);
				m_struDeviceInfo.struChanInfo[i].iChanIndex = i + m_struDeviceInfo.iStartChan;  //ͨ����
				m_struDeviceInfo.struChanInfo[i].bEnable = TRUE;
			}
			else
			{
				m_struDeviceInfo.struChanInfo[i].iChanIndex = -1;
				m_struDeviceInfo.struChanInfo[i].bEnable = FALSE;
				sprintf(m_struDeviceInfo.struChanInfo[i].chChanName, "");

				m_struDeviceInfo1.struChanInfo[i].iChanIndex = -1;
				m_struDeviceInfo1.struChanInfo[i].bEnable = FALSE;
				sprintf(m_struDeviceInfo1.struChanInfo[i].chChanName, "");
			}
		}

	}
	else        //֧��IP���룬9000�豸
	{
		for (i = 0; i < MAX_ANALOG_CHANNUM; i++)  //ģ��ͨ��
		{
			if (i < m_struDeviceInfo.iDeviceChanNum)
			{
				sprintf(m_struDeviceInfo.struChanInfo[i].chChanName, "aaa%d", i + m_struDeviceInfo.iStartChan);
				m_struDeviceInfo.struChanInfo[i].iChanIndex = i + m_struDeviceInfo.iStartChan;
				if (IpAccessCfg.byAnalogChanEnable[i])
				{
					m_struDeviceInfo.struChanInfo[i].bEnable = TRUE;
				}
				else
				{
					m_struDeviceInfo.struChanInfo[i].bEnable = FALSE;
				}

			}
			else//clear the state of other channel
			{
				m_struDeviceInfo.struChanInfo[i].iChanIndex = -1;
				m_struDeviceInfo.struChanInfo[i].bEnable = FALSE;
				sprintf(m_struDeviceInfo.struChanInfo[i].chChanName, "");
			}
		}

		//����ͨ��
		for (i = 0; i < MAX_IP_CHANNEL; i++)
		{
			if (IpAccessCfg.struStreamMode[i].uGetStream.struChanInfo.byEnable)  //ipͨ������
			{
				m_struDeviceInfo.struChanInfo[i + MAX_ANALOG_CHANNUM].bEnable = TRUE;
				m_struDeviceInfo.struChanInfo[i + MAX_ANALOG_CHANNUM].iChanIndex = i + IpAccessCfg.dwStartDChan;
				sprintf(m_struDeviceInfo.struChanInfo[i + MAX_ANALOG_CHANNUM].chChanName, "IP Camera %d", i + 1);
			}
			else
			{
				m_struDeviceInfo.struChanInfo[i + MAX_ANALOG_CHANNUM].bEnable = FALSE;
				m_struDeviceInfo.struChanInfo[i + MAX_ANALOG_CHANNUM].iChanIndex = -1;
			}
		}


	}

}

/*************************************************
������:    	CreateDeviceTree
��������:	����ͨ����
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::CreateDeviceTree()
{
	m_hDevItem = m_ctrlTreeChan.InsertItem("Dev");
	m_ctrlTreeChan.SetItemData(m_hDevItem, DEVICETYPE * 1000);
	for (int i = 0; i < MAX_CHANNUM_V30; i++)
	{
		if (m_struDeviceInfo.struChanInfo[i].bEnable)  //ͨ����Ч������ͨ����
		{
			HTREEITEM ChanItem = m_ctrlTreeChan.InsertItem(m_struDeviceInfo.struChanInfo[i].chChanName, m_hDevItem);
			m_ctrlTreeChan.SetItemData(ChanItem, CHANNELTYPE * 1000 + i);   //Data��Ӧͨ���������е�����
		}
	}
	m_ctrlTreeChan.Expand(m_hDevItem, TVE_EXPAND);
}

/*************************************************
������:    	OnDblclkTreeChan
��������:	˫��ͨ����������ѡ��ͨ��
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnDblclkTreeChan(NMHDR* pNMHDR, LRESULT* pResult)
{
	HTREEITEM hSelected = m_ctrlTreeChan.GetSelectedItem();
	//δѡ��
	if (NULL == hSelected)
		return;
	DWORD itemData = m_ctrlTreeChan.GetItemData(hSelected);
	HTREEITEM hParent = NULL;
	int itype = itemData / 1000;    //
	int iIndex = itemData % 1000;

	switch (itype)
	{
	case DEVICETYPE:
		m_iCurChanIndex = -1;
		break;
	case CHANNELTYPE:
		m_iCurChanIndex = iIndex;
		TRACE("select chan: %d\n", iIndex);
		//    DbPlayChannel(iIndex);
		StartPlay(m_iCurChanIndex);
		OnSelchangeComboPreset();
		break;
	default:
		break;

	}
	*pResult = 0;
}

/*************************************************
������:    	DbPlayChannel
��������:	˫������
�������:   ChanIndex-ͨ����
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::DbPlayChannel(int ChanIndex)
{

	 	if(!m_bIsPlaying)  //Play
	 	{
	 		StartPlay(ChanIndex);
	 	}
	 	else                //Stop,play
	 	{
	         StopPlay();
	 		StartPlay(ChanIndex);
	 
	 	}
}

/*************************************************
������:    	StartPlay
��������:	��ʼһ·����
�������:   ChanIndex-ͨ����
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::StartPlay(int iChanIndex)
{
	cRPD = this;
	if (!m_bIsPlaying&&iChanIndex == 0)
	{
		NET_DVR_CLIENTINFO ClientInfo;
//		ClientInfo.hPlayWnd = GetDlgItem(IDC_STATIC_PLAY)->m_hWnd;
		ClientInfo.hPlayWnd = NULL;
		ClientInfo.lChannel = m_iCurChanIndex + 1;
		ClientInfo.lLinkMode = 0;
		ClientInfo.sMultiCastIP = NULL;
		TRACE("Channel number:%d\n", ClientInfo.lChannel);

		m_lPlayHandle = NET_DVR_RealPlay_V30(m_struDeviceInfo.lLoginID, &ClientInfo, NULL, NULL, TRUE);

		NET_DVR_SetRealDataCallBack(m_lPlayHandle, fRealDataCallBackIPC, 1);

	
		//ʹ�ö�ʱ����ʮ��
		SetTimer(1, 15, 0);

		if (-1 == m_lPlayHandle)
		{
			DWORD err = NET_DVR_GetLastError();
			CString m_csErr;
			m_csErr.Format("IPCamera���ų����������%d", err);
			MessageBox(m_csErr);
		}

		m_bIsPlaying = TRUE;
	}

	if (!m_bIsPlaying1&&iChanIndex == 1)
	{
		//	TRACE("sssssssssssssss\n");
		NET_DVR_CLIENTINFO ClientInfo;
		//ClientInfo.hPlayWnd = hPlayWnd1;
		//ClientInfo.hPlayWnd = GetDlgItem(IDC_STATIC_PLAY2)->m_hWnd;
		ClientInfo.hPlayWnd = NULL;
		ClientInfo.lChannel = m_iCurChanIndex + 2;
		ClientInfo.lLinkMode = 0;
		ClientInfo.sMultiCastIP = NULL;
		TRACE("Channel number:%d\n", ClientInfo.lChannel);

	
		m_lPlayHandle1 = NET_DVR_RealPlay_V30(m_struDeviceInfo1.lLoginID, &ClientInfo, NULL, NULL, TRUE);

		NET_DVR_SetRealDataCallBack(m_lPlayHandle1, fRealDataCallBack, 2);

		if (-1 == m_lPlayHandle1)
		{
			DWORD err = NET_DVR_GetLastError();
			CString m_csErr;
			m_csErr.Format("fishEye���ų����������%d", err);
			MessageBox(m_csErr);
		}

		m_bIsPlaying1 = TRUE;
	}

}

/*************************************************
������:    	StopPlay
��������:	ֹͣ����
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::StopPlay()
{
	if (m_lPlayHandle != -1)
	{
		if (m_bIsRecording)  //����¼����ֹͣ
		{
			StopRecord();
		}
		NET_DVR_StopRealPlay(m_lPlayHandle);
		m_lPlayHandle = -1;
		m_bIsPlaying = FALSE;
		GetDlgItem(IDC_STATIC_PLAY)->Invalidate();
	}
}

/*************************************************
������:    	OnSelchangedTreeChan
��������:	��ȡѡ�е�ͨ����
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnSelchangedTreeChan(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	HTREEITEM hSelected = m_ctrlTreeChan.GetSelectedItem();
	//δѡ��
	if (NULL == hSelected)
		return;
	DWORD itemData = m_ctrlTreeChan.GetItemData(hSelected);
	HTREEITEM hParent = NULL;
	int itype = itemData / 1000;    //
	int iIndex = itemData % 1000;

	switch (itype)
	{
	case DEVICETYPE:
		m_iCurChanIndex = -1;
		break;
	case CHANNELTYPE:
		m_iCurChanIndex = iIndex;
		OnSelchangeComboPreset();
		TRACE("select chan index: %d\n", iIndex);
		break;
	default:
		break;

	}
	*pResult = 0;
}

/*************************************************
������:    	GetCurChanIndex
��������:	��ȡ��ǰѡ�е�ͨ����
�������:
�������:
����ֵ:		��ǰѡ�е�ͨ����
**************************************************/
int CRealPlayDlg::GetCurChanIndex()
{
	return m_iCurChanIndex;
}

/*************************************************
������:    	GetPlayHandle
��������:	��ȡ���ž��
�������:
�������:
����ֵ:		���ž��
**************************************************/
LONG CRealPlayDlg::GetPlayHandle()
{
	return m_lPlayHandle;
}

/*************************************************
������:    	GetPTZSpeed
��������:	��ȡ��̨�ٶ�
�������:
�������:
����ֵ:		��̨�ٶ�
**************************************************/
int CRealPlayDlg::GetPTZSpeed()
{
	return (m_comboPTZSpeed.GetCurSel());
}

/*************************************************
������:    	OnButtonRecord
��������:	��ʼ/ֹͣ¼�� ��ť
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonRecord()
{
	if (m_lPlayHandle == -1)
	{
		MessageBox("����ѡ��һ��ͨ������");
		return;
	}
	if (!m_bIsRecording)
	{
		StartRecord();
	}
	else
	{
		StopRecord();
	}

}

/*************************************************
������:    	StartRecord
��������:	��ʼ¼��
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::StartRecord()
{
	char RecName[256] = { 0 };

	CTime CurTime = CTime::GetCurrentTime();;
	sprintf(RecName, "%04d%02d%02d%02d%02d%02d_ch%02d.mp4", CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
		CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), m_struDeviceInfo.struChanInfo[GetCurChanIndex()].iChanIndex);

	if (!NET_DVR_SaveRealData(m_lPlayHandle, RecName))
	{
		MessageBox("����¼��ʧ��");
		return;
	}
	m_bIsRecording = TRUE;
	GetDlgItem(IDC_BUTTON_RECORD)->SetWindowText("ֹͣ¼��");
}

/*************************************************
������:    	StopRecord
��������:	ֹͣ¼��
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::StopRecord()
{
	if (!NET_DVR_StopSaveRealData(m_lPlayHandle))
	{
		MessageBox("ֹͣ¼��ʧ��");
		return;
	}
	m_bIsRecording = FALSE;
	GetDlgItem(IDC_BUTTON_RECORD)->SetWindowText("¼��");
}
/*************************************************
������:    	OnSelchangeComboPreset
��������:	Ԥ�õ�combobox���ı䰴ť״̬
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnSelchangeComboPreset()
{
	int iIndex = m_comboPreset.GetCurSel();

	if (m_struDeviceInfo.struChanInfo[m_iCurChanIndex].struDecodercfg.bySetPreset[iIndex])
	{
		GetDlgItem(IDC_BUTTON_PRESET_GOTO)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_PRESET_DEL)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_PRESET_GOTO)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_PRESET_DEL)->EnableWindow(FALSE);
	}
}
//IPCץͼ����
void CRealPlayDlg::OnSelchangeComboPicType()
{
	int iSel = m_coPicType.GetCurSel();
	if (0 == iSel)  //jpg
	{
		m_coJpgSize.EnableWindow(TRUE);
		m_coJpgQuality.EnableWindow(TRUE);
	}
	else if (1 == iSel)          //bmp
	{
		m_coJpgSize.EnableWindow(FALSE);
		m_coJpgQuality.EnableWindow(FALSE);
	}

}
/*************************************************
������:    	OnButtonCapture
��������:	ץͼ
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonCapture()
{
	if (m_lPlayHandle == -1)
	{
		MessageBox("����ѡ��һ��ͨ������");
		return;
	}
	UpdateData(TRUE);

	char PicName[256] = { 0 };

	int iPicType = m_coPicType.GetCurSel();
	if (1 == iPicType)  //bmp
	{
		CTime CurTime = CTime::GetCurrentTime();;
		sprintf(PicName, "%04d%02d%02d%02d%02d%02d_ch%02d.bmp", CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
			CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), m_struDeviceInfo.struChanInfo[GetCurChanIndex()].iChanIndex);

		if (NET_DVR_CapturePicture(m_lPlayHandle, PicName))
		{
			MessageBox("ץͼ�ɹ�!");
		}
	}
	else if (0 == iPicType)  //jgp
	{
		CTime CurTime = CTime::GetCurrentTime();;
		sprintf(PicName, "%04d%02d%02d%02d%02d%02d_ch%02d.jpg", CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(), \
			CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), m_struDeviceInfo.struChanInfo[GetCurChanIndex()].iChanIndex);

		//�齨jpg�ṹ
		NET_DVR_JPEGPARA JpgPara = { 0 };
		JpgPara.wPicSize = (WORD)m_coJpgSize.GetCurSel();
		JpgPara.wPicQuality = (WORD)m_coJpgQuality.GetCurSel();

		LONG iCurChan = m_struDeviceInfo.struChanInfo[GetCurChanIndex()].iChanIndex;

		if (NET_DVR_CaptureJPEGPicture(m_struDeviceInfo.lLoginID, iCurChan, &JpgPara, PicName))
		{
			MessageBox("ץͼ�ɹ�");
		}
	}

	return;
}

/*************************************************
������:    	OnButtonPlay
��������:	����ѡ��ͨ��
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonPlay()
{

	if (!m_bIsPlaying1 || !m_bIsPlaying)
	{

		m_iCurChanIndex = 0;
		StartPlay(m_iCurChanIndex);

		StartPlay(m_iCurChanIndex + 1);

		GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText("ֹͣ����");
	}
	else
	{
		NET_DVR_StopRealPlay(m_lPlayHandle);
		NET_DVR_StopRealPlay(m_lPlayHandle1);
		m_lPlayHandle = -1;
		m_lPlayHandle1 = -1;
		m_bIsPlaying = FALSE;
		m_bIsPlaying1 = FALSE;
		GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText("����");
	}

}


/*************************************************
������:    	GetDecoderCfg
��������:	��ȡ��̨��������Ϣ
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::GetDecoderCfg()
{
	NET_DVR_DECODERCFG_V30 DecoderCfg;
	DWORD  dwReturned;
	BOOL bRet;


	//��ȡͨ����������Ϣ
	for (int i = 0; i < MAX_CHANNUM_V30; i++)
	{
		if (m_struDeviceInfo.struChanInfo[i].bEnable)
		{
			memset(&DecoderCfg, 0, sizeof(NET_DVR_DECODERCFG_V30));
			bRet = NET_DVR_GetDVRConfig(m_struDeviceInfo.lLoginID, NET_DVR_GET_DECODERCFG_V30, \
				m_struDeviceInfo.struChanInfo[i].iChanIndex, &DecoderCfg, sizeof(NET_DVR_DECODERCFG_V30), &dwReturned);
			if (!bRet)
			{
				TRACE("Get DecderCfg failed,Chan:%d\n", m_struDeviceInfo.struChanInfo[i].iChanIndex);
				continue;
			}

			memcpy(&m_struDeviceInfo.struChanInfo[i].struDecodercfg, &DecoderCfg, sizeof(NET_DVR_DECODERCFG_V30));
		}

	}

}

/*************************************************
������:    	InitDecoderReferCtrl
��������:	��ʼ����̨������ؿؼ�
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::InitDecoderReferCtrl()
{
	int i;
	CString tmp;
	//����Ԥ�õ�
	for (i = 0; i < MAX_PRESET_V30; i++)
	{
		tmp.Format("%d", i + 1);     //i+1
		m_comboPreset.AddString(tmp);
	}
	m_comboPreset.SetCurSel(0);

	GetDlgItem(IDC_BUTTON_PRESET_GOTO)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PRESET_DEL)->EnableWindow(FALSE);

	//Ѳ���켣
	for (i = 0; i < MAX_CRUISE_SEQ; i++)
	{
		tmp.Format("%d", i + 1);     //i+1
		m_comboSeq.AddString(tmp);
	}
	m_comboSeq.SetCurSel(0);


}

/*************************************************
������:    	OnButtonPresetGoto
��������:	����Ԥ�õ�
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonPresetGoto()
{
	int iPreset = m_comboPreset.GetCurSel() + 1;    //+1
	if (m_lPlayHandle >= 0)
	{
		if (!NET_DVR_PTZPreset(m_lPlayHandle, GOTO_PRESET, iPreset))
		{
			MessageBox("����Ԥ�õ�ʧ��");
			return;
		}
	}
	else
	{
		if (!NET_DVR_PTZPreset_Other(m_struDeviceInfo.lLoginID, m_struDeviceInfo.struChanInfo[m_iCurChanIndex].iChanIndex, \
			GOTO_PRESET, iPreset))
		{
			MessageBox("����Ԥ�õ�ʧ��");
			return;
		}

	}
}

/*************************************************
������:    	OnButtonPresetSet
��������:	����Ԥ�õ�
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonPresetSet()
{
	int iPreset = m_comboPreset.GetCurSel() + 1;    //+1
	if (m_lPlayHandle >= 0)
	{
		if (!NET_DVR_PTZPreset(m_lPlayHandle, SET_PRESET, iPreset))
		{
			MessageBox("����Ԥ�õ�ʧ��");
			return;
		}
	}
	else
	{
		if (!NET_DVR_PTZPreset_Other(m_struDeviceInfo.lLoginID, m_struDeviceInfo.struChanInfo[m_iCurChanIndex].iChanIndex, \
			SET_PRESET, iPreset))
		{
			MessageBox("����Ԥ�õ�ʧ��");
			return;
		}

	}

	//��ӵ�Ԥ�õ���Ϣ
	m_struDeviceInfo.struChanInfo[m_iCurChanIndex].struDecodercfg.bySetPreset[iPreset - 1] = TRUE;
	//���°�ť״̬
	OnSelchangeComboPreset();

}


/*************************************************
������:    	OnButtonPresetDel
��������:	ɾ��Ԥ�õ�
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonPresetDel()
{
	int iPreset = m_comboPreset.GetCurSel() + 1;    //+1
	if (m_lPlayHandle >= 0)
	{
		if (!NET_DVR_PTZPreset(m_lPlayHandle, CLE_PRESET, iPreset))
		{
			MessageBox("ɾ��Ԥ�õ�ʧ��");
			return;
		}
	}
	else
	{
		if (!NET_DVR_PTZPreset_Other(m_struDeviceInfo.lLoginID, m_struDeviceInfo.struChanInfo[m_iCurChanIndex].iChanIndex, \
			CLE_PRESET, iPreset))
		{
			MessageBox("ɾ��Ԥ�õ�ʧ��");
			return;
		}

	}

	//��ӵ�Ԥ�õ���Ϣ
	m_struDeviceInfo.struChanInfo[m_iCurChanIndex].struDecodercfg.bySetPreset[iPreset - 1] = FALSE;
	//���°�ť״̬
	OnSelchangeComboPreset();

}

/*************************************************
������:    	OnButtonSeqGoto
��������:	����/ֹͣ Ѳ��
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonSeqGoto()
{
	int iSeq = m_comboSeq.GetCurSel() + 1;    //+1
	if (!m_bIsOnCruise)
	{
		if (m_lPlayHandle >= 0)
		{
			if (!NET_DVR_PTZCruise(m_lPlayHandle, RUN_SEQ, iSeq, 0, 0))
			{
				MessageBox("����Ѳ��ʧ��");
				return;
			}
		}
		else
		{
			if (!NET_DVR_PTZCruise_Other(m_struDeviceInfo.lLoginID, m_struDeviceInfo.struChanInfo[m_iCurChanIndex].iChanIndex, \
				RUN_SEQ, iSeq, 0, 0))
			{
				MessageBox("����Ѳ��ʧ��");
				return;
			}

		}
		m_bIsOnCruise = TRUE;
		GetDlgItem(IDC_BUTTON_SEQ_GOTO)->SetWindowText("ֹͣ");
	}
	else
	{
		if (m_lPlayHandle >= 0)
		{
			if (!NET_DVR_PTZCruise(m_lPlayHandle, STOP_SEQ, iSeq, 0, 0))
			{
				MessageBox("ֹͣѲ��ʧ��");
				return;
			}
		}
		else
		{
			if (!NET_DVR_PTZCruise_Other(m_struDeviceInfo.lLoginID, m_struDeviceInfo.struChanInfo[m_iCurChanIndex].iChanIndex, \
				STOP_SEQ, iSeq, 0, 0))
			{
				MessageBox("ֹͣѲ��ʧ��");
				return;
			}

		}
		m_bIsOnCruise = FALSE;
		GetDlgItem(IDC_BUTTON_SEQ_GOTO)->SetWindowText("����");
	}

}

/*************************************************
������:    	OnButtonSeqSet
��������:	����Ѳ��·��
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonSeqSet()
{
	CDlgPTZCruise Dlg;
	Dlg.DoModal();

}

/*************************************************
������:    	OnButtonTrackRun
��������:	��ʼ���й켣
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonTrackRun()
{
	if (!m_bTrackRun)
	{
		if (m_lPlayHandle >= 0)
		{
			if (!NET_DVR_PTZTrack(m_lPlayHandle, RUN_CRUISE))
			{
				MessageBox("���й켣ʧ��");
			}
		}
		else
		{
			if (!NET_DVR_PTZTrack(m_struDeviceInfo.lLoginID, m_struDeviceInfo.struChanInfo[m_iCurChanIndex].iChanIndex))
			{
				MessageBox("���й켣ʧ��");
			}
		}
		m_bTrackRun = TRUE;
		GetDlgItem(IDC_BUTTON_TRACK_RUN)->SetWindowText("ֹͣ");
	}
	else
	{
		//��㷢��һ����̨��������ֹͣ����
		if (m_lPlayHandle >= 0)
		{
			if (!NET_DVR_PTZControl(m_lPlayHandle, TILT_UP, 1))
			{
				MessageBox("ֹͣ�켣ʧ��");
			}
		}
		else
		{
			if (!NET_DVR_PTZControl_Other(m_struDeviceInfo.lLoginID, m_struDeviceInfo.struChanInfo[m_iCurChanIndex].iChanIndex, TILT_UP, 1))
			{
				MessageBox("ֹͣ�켣ʧ��");
			}
		}
		m_bTrackRun = FALSE;
		GetDlgItem(IDC_BUTTON_TRACK_RUN)->SetWindowText("����");
	}



}

/*************************************************
������:    	OnButtonTrackStart
��������:	��ʼ��¼�켣
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonTrackStart()
{

	if (m_lPlayHandle >= 0)
	{
		if (!NET_DVR_PTZTrack(m_lPlayHandle, STA_MEM_CRUISE))
		{
			MessageBox("��ʼ��¼�켣ʧ��");
			return;
		}
	}
	else
	{
		if (!NET_DVR_PTZTrack_Other(m_struDeviceInfo.lLoginID, m_struDeviceInfo.struChanInfo[m_iCurChanIndex].iChanIndex, STA_MEM_CRUISE))
		{
			MessageBox("��ʼ��¼�켣ʧ��");
			return;
		}
	}


}

/*************************************************
������:    	OnButtonTrackStop
��������:	ֹͣ��¼�켣
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnButtonTrackStop()
{
	if (m_lPlayHandle >= 0)
	{
		if (!NET_DVR_PTZTrack(m_lPlayHandle, STO_MEM_CRUISE))
		{
			MessageBox("ֹͣʧ��");
			return;
		}
	}
	else
	{
		if (!NET_DVR_PTZTrack_Other(m_struDeviceInfo.lLoginID, m_struDeviceInfo.struChanInfo[m_iCurChanIndex].iChanIndex, STO_MEM_CRUISE))
		{
			MessageBox("ֹͣʧ��");
			return;
		}
	}
}

/*************************************************
������:    	OnBtnPtzAuto
��������:	��̨��ʼ/ֹͣ�����Զ�ɨ��
�������:
�������:
����ֵ:
**************************************************/
void CRealPlayDlg::OnBtnPtzAuto()
{
	int iSpeed = GetPTZSpeed();
	if (m_lPlayHandle >= 0)
	{
		if (!m_bAutoOn)
		{
			if (iSpeed >= 1)
			{
				NET_DVR_PTZControlWithSpeed(m_lPlayHandle, PAN_AUTO, 0, iSpeed);
			}
			else
			{
				NET_DVR_PTZControl(m_lPlayHandle, PAN_AUTO, 0);
			}
			GetDlgItem(IDC_BTN_PTZ_AUTO)->SetWindowText("ֹͣ");
			m_bAutoOn = TRUE;
		}
		else
		{
			if (iSpeed >= 1)
			{
				NET_DVR_PTZControlWithSpeed(m_lPlayHandle, PAN_AUTO, 1, iSpeed);
			}
			else
			{
				NET_DVR_PTZControl(m_lPlayHandle, PAN_AUTO, 1);
			}
			GetDlgItem(IDC_BTN_PTZ_AUTO)->SetWindowText("�Զ�");
			m_bAutoOn = FALSE;
		}
	}

}

void CRealPlayDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (m_bIsLogin)
	{
		StopPlay();
		NET_DVR_Logout_V30(m_struDeviceInfo.lLoginID);
	}
	cvReleaseVideoWriter(&writer);


	cvDestroyAllWindows();

	CDialog::OnClose();
}

void CRealPlayDlg::OnIpnFieldchangedIpaddressDev(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;
}

//��ͷ��λ
void CRealPlayDlg::OnBnClickedButton1()
{
	NET_DVR_PTZPOS lpinBuffer;
	lpinBuffer.wAction = 1;//��ȡʱ���ֶ���Ч
	lpinBuffer.wPanPos = 0x120;
	lpinBuffer.wTiltPos = 0x634;//��ֱ����;
	lpinBuffer.wZoomPos = 0;//�䱶����;

	DWORD size = sizeof(lpinBuffer);

	bool xxx = NET_DVR_SetDVRConfig(m_struDeviceInfo.lLoginID, NET_DVR_SET_PTZPOS, 1, &lpinBuffer, size);//�������λ��������Ϣ

	if (!xxx)
	{
		DWORD	iLastErr = NET_DVR_GetLastError();
		CString str;
		str.Format(_T("NET_DVR_Login_V30 failed, error code= %d", iLastErr));
		MessageBox(str);
	}

}

//��ʾ�����������ͼ��
afx_msg LRESULT CRealPlayDlg::OnShowEndImage(WPARAM wParam, LPARAM lParam)
{

	// 	int end = clock();
	// 	TRACE("***************============   %d\n", start - end);
	// 	start = end;
	IplImage *img1 = (IplImage *)lParam;

	//����Ϊ��Ƶ�ļ�
 	if (RecordFish_flag == TRUE)
 	{
		if (writer == NULL)
		{
			char RecName[256] = { 0 };
			CTime CurTime = CTime::GetCurrentTime();
			CString RectName;
			RectName.Format(_T("D:\\Video  avi\\%04d%02d%02d-%02d%02d%02d.avi"), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay(),
				CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond());
			int fps = 3;
			writer = cvCreateVideoWriter(RectName, CV_FOURCC('X', 'V', 'I', 'D'), fps, cvSize(1280, 720));
		}
		cvWriteFrame(writer, img1);
	}
	//��ʾת�������Ƶ
	if (img1 != NULL)
	{	
		IplImage *desc = cvCreateImage(cvSize(FishWidth, FishHeight), img1->depth, img1->nChannels);
		cvResize(img1, desc, CV_INTER_CUBIC);
		CImage myImage;
		myImage.CopyOf(desc);
		CRect rect;
		CWnd *pWnd = GetDlgItem(IDC_STATIC_PLAY2);
			CDC *pDC = pWnd->GetDC();
		pWnd->GetClientRect(&rect);
		pDC->SetStretchBltMode(STRETCH_HALFTONE);
		myImage.DrawToHDC(pDC->m_hDC, rect);
		ReleaseDC(pDC);
		myImage.Destroy();
		cvReleaseImage(&desc);

	
		//�������ָ���
        CClientDC dc(this);
		CRect Rect;
		GetDlgItem(IDC_STATIC_PLAY2)->GetWindowRect(Rect);
		ScreenToClient(Rect);
		CPen pen(PS_SOLID, 1, RGB(0, 255, 0)); ////����һ����������󣬹���ʱ���û�������
		dc.SelectObject(&pen);
		CPoint m_Start_Line_x((Rect.right+Rect.left)/2,Rect.top);
		CPoint m_End_Line_x((Rect.right + Rect.left) / 2, Rect.bottom);
		dc.MoveTo(m_Start_Line_x);
		dc.LineTo(m_End_Line_x);
		CPoint m_Start_Line_y(Rect.left, (Rect.bottom + Rect.top) / 2);
		CPoint m_End_Line_y(Rect.right, (Rect.bottom + Rect.top) / 2);
		dc.MoveTo(m_Start_Line_y);
		dc.LineTo(m_End_Line_y);
		pen.DeleteObject();

// 		//�����ο򻭵�ͼ����
		if (DrawRange_flag == TRUE)
		{			
			CPen pen(0, 1, RGB(255, 0, 0));
			CPen *oldPen = dc.SelectObject(&pen);
			dc.SelectStockObject(NULL_BRUSH);

			//CPoint CompPoint; 
			//CompPoint.x = IPWidth*(m_OldPoint1.y - m_startPoint1.y) / IPHeight + m_startPoint1.x;
			//CompPoint.y = m_OldPoint1.y;
			////���ƻ���Ĵ�С
			//CRect Rect;
			//GetDlgItem(IDC_STATIC_PLAY2)->GetWindowRect(Rect);
			//ScreenToClient(Rect);
			//if (CompPoint.x>Rect.right)
			//{
			//	m_startPoint1.x = m_startPoint1.x - (CompPoint.x - Rect.right);
			//	CompPoint.x = Rect.right;	
			//	
			//}
			//else if (CompPoint.x < Rect.left)
			//{
			//	m_startPoint1.x = m_startPoint1.x + (Rect.left - CompPoint.x);
			//	CompPoint.x = Rect.left;				
			//}		
			dc.Rectangle(CRect(m_startPoint1, m_OldPoint1));
			dc.SelectObject(oldPen);
			oldPen->DeleteObject();
			pen.DeleteObject();
			oldPen = NULL;
		}
		UpdateData(TRUE);
		pWnd = NULL;
		pDC = NULL;
	}
		img1 = NULL;
		

	return 0;
}
//�ؼ�����Ӧ����
void CRealPlayDlg::ReSize(void)
{
	float fsp[2];
	POINT Newp; //��ȡ���ڶԻ���Ĵ�С  
	double New_x;
	double New_y;
	CRect recta;
	GetClientRect(&recta);     //ȡ�ͻ�����С    
	Newp.x = recta.right - recta.left;
	Newp.y = recta.bottom - recta.top;
	fsp[0] = (float)Newp.x / old.x;
	fsp[1] = (float)Newp.y / old.y;
	CRect Rect;
	int woc;
	int i;
	CPoint OldTLPoint, TLPoint; //���Ͻ�  
	CPoint OldBRPoint, BRPoint; //���½�  
	HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //�г����пؼ�  
	//ȡ����ʼʱ�ؼ���λ��
	if (flag_First == false)
	{
		i = 0;
		while (hwndChild)
		{
			woc = ::GetDlgCtrlID(hwndChild);//ȡ��ID  
			GetDlgItem(woc)->GetWindowRect(Rect);
			ScreenToClient(Rect);
			OldPoint[i++] = Rect.TopLeft();
			OldPoint[i++] = Rect.BottomRight();
			hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
		}
		flag_First = true;
		old = Newp;
	}
	//����Ӧ�ؼ���λ��
	else{
		i = 0;
		while (hwndChild)
		{
			woc = ::GetDlgCtrlID(hwndChild);//ȡ��ID  
			OldTLPoint = Rect.TopLeft();
			TLPoint.x = long((OldPoint[i].x*fsp[0]));
			TLPoint.y = long((OldPoint[i++].y * fsp[1]));
			OldBRPoint = Rect.BottomRight();
			BRPoint.x = long((OldPoint[i].x *fsp[0]));
			BRPoint.y = long((OldPoint[i++].y *fsp[1]));
			Rect.SetRect(TLPoint, BRPoint);
			GetDlgItem(woc)->MoveWindow(Rect, TRUE);
			hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
		}
	}
	Invalidate(TRUE);
}
//�ؼ�����Ӧ
void CRealPlayDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	//if (AUTOSIZE == TRUE)
	//{    
	//	//�ж����  ��С��  �Ƿ�ı�ؼ�
	//	if (!this->IsIconic() && !this->IsZoomed())
	//	{
			ReSize();
			
	/*	}    
	}*/
	
	// TODO:  �ڴ˴������Ϣ����������
}
//����¼��
void CRealPlayDlg::OnBnClickedButton2()
{
	if (ButtonRecordFish_flag == FALSE)
	{
		GetDlgItem(IDC_BUTTON2)->SetWindowText("ֹͣ¼��");
		
	RecordFish_flag = TRUE;
	ButtonRecordFish_flag = TRUE;
	}
	else
	{
		GetDlgItem(IDC_BUTTON2)->SetWindowText("¼��");
		RecordFish_flag = FALSE;
		
		//ֹͣ¼��
		writer = NULL;
		ButtonRecordFish_flag = FALSE;
	}
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}

void CRealPlayDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (m_struDeviceInfo.lLoginID == 0)
	{
		CRect Rect;
		GetDlgItem(IDC_STATIC_PLAY2)->GetWindowRect(Rect);
		ScreenToClient(Rect);
		if (point.x >= Rect.left&&point.x <= Rect.right&&point.y >= Rect.top&&point.y <= Rect.bottom)
		{
			//����ԭ���ľ���
			CClientDC dc(this);
			UpdateWindow();
			//��ȡ��ʼ����
			m_startPoint1 = point;
			m_OldPoint1 = point;
			x1 = point.x - Rect.left;
			y11 = point.y - Rect.top;
			m_startRect1 = TRUE;
			DrawRange_flag = FALSE;
		}
	}


	CDialog::OnLButtonDown(nFlags, point);
}

void CRealPlayDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ

	if (m_struDeviceInfo.lLoginID == 0)
	{
		CRect Rect;
		GetDlgItem(IDC_STATIC_PLAY2)->GetWindowRect(Rect);
		ScreenToClient(Rect);
		if (point.x >= Rect.left&&point.x <= Rect.right&&point.y >= Rect.top&&point.y <= Rect.bottom)
		{
			a1 = x1;
			b1 = y11;
			c1 = point.x - Rect.left;
			d1 = point.y - Rect.top;
			TRACE("  =====     a=%d  b=%d  c=%d d=%d   ��%d  �ߣ�%d\n ", a1, b1, c1, d1,c1-a1,d1-b1);
			//��ʾ�������������
			CRealPlayDlg *pDlg1 = (CRealPlayDlg*)AfxGetMainWnd();
			CString xxxxx;
			xxxxx.Format(_T("��ʼ���꣺  %d��%d    �������꣺  %d��%d"), a1, b1, c1, d1);
			pDlg1->GetDlgItem(IDC_EDIT2_point)->SetWindowTextA(xxxxx);
			m_OldPoint1 = point;
			m_startRect1 = FALSE;

			DrawRange_flag = TRUE;

			//ȡ�����ʱ��ͷ�ƶ�
 			if (a1 != c1&&b1 != d1)
 			{
					IpcameraMove();
			}

		}
	}
	CDialog::OnLButtonUp(nFlags, point);
}

//���� 
void CRealPlayDlg::OnMouseMove(UINT nFlags, CPoint point)
{

	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CRect Rect;
	GetDlgItem(IDC_STATIC_PLAY2)->GetWindowRect(Rect);
	//��Դ��ڴ�С
	ScreenToClient(Rect);
	if (point.x >= Rect.left&&point.x <= Rect.right&&point.y >= Rect.top&&point.y <= Rect.bottom)
	{
		if (TRUE == m_startRect1)   //�����Ƿ��е����ж��Ƿ���Ի�����
		{
			CClientDC dc(this);        //ȡ�ÿͻ����豸������

			dc.SetROP2(R2_NOTXORPEN);	 //���û�ͼɫΪ��Ļ��תɫ
			CPen pen(0, 1, RGB(234, 23, 53));
			CBrush *pBrush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
			dc.SelectObject(&pen);
			dc.SelectStockObject(NULL_BRUSH);
			dc.Rectangle(CRect(m_startPoint1, point));//����ԭ�о���
			dc.Rectangle(CRect(m_startPoint1, point)); //���µľ���
			dc.SelectObject(pBrush);
			pen.DeleteObject();
		}
	}
	CDialog::OnMouseMove(nFlags, point);
}
// ��ͷ�ƶ�
void CRealPlayDlg::IpcameraMove()
{
	//�������
	CountPoint();

	NET_DVR_PTZPOS lpinBuffer;
	lpinBuffer.wAction = 1;//��ȡʱ���ֶ���Ч
	lpinBuffer.wPanPos = k1;//ˮƽ����  k1      0--13824   16���Ƶ�360
	lpinBuffer.wTiltPos = w1;//��ֱ����;  w1    0--2304    16���Ƶ�90
	lpinBuffer.wZoomPos = z1;//�䱶����; z1     16--512     16���Ƶ�

//	TRACE("  =====     k=%f  w=%f  z=%f \n ", k1, w1, z1);

	DWORD size = sizeof(lpinBuffer);

	bool xxx = NET_DVR_SetDVRConfig(m_struDeviceInfo.lLoginID, NET_DVR_SET_PTZPOS, 1, &lpinBuffer, size);//�������λ��������Ϣ

	if (!xxx)
	{
		DWORD	iLastErr = NET_DVR_GetLastError();
		CString str;
		str.Format(_T("NET_DVR_Login_V30 failed, error code= %d", iLastErr));
		MessageBox(str);
	}

}
//�������
void CRealPlayDlg::CountPoint()
{
	o1 = sqrt(pow((double)(a1 - c1), 2) + pow((double)(b1 - d1), 2));  //���ߵĶԽ��߳���
	p1 = sqrt(pow((double)FishWidth, 2) + pow((double)FishHeight, 2));  //�ؼ��ĶԽ��߳���

	CPoint point((a1 + c1) / 2, (b1 + d1) / 2);
	k1 = ComputeAngle(point)+15;  // ����ƫ�ƽǶ�15

//	TRACE("*******    %f", k1);
	char buffer2[20];
	k1 = k1*10;  //ˮƽ����
	sprintf_s(buffer2, "%f", k1);
	k1 = strtoul(buffer2, NULL, 16);  //��ʮ����ת��Ϊʮ������

	//��ֱ����
	CPoint CentPoint(FishWidth / 2, FishHeight / 2);
	double length = PointLegth(point, CentPoint);
	double ffish = FishWidth / (2 * tan((double)60 / 180 * PI));//�궨60��
 	w1 = atan(length / ffish) * 180 / PI+21;     //����ƫ�ƽǶ�25
	w1 = w1 * 10;
	char buffer3[20];
	sprintf_s(buffer3, "%f", w1);
	w1 = strtoul(buffer3, NULL, 16);
	w1 = 2202 - w1;

	//�䱶����

	double zoomAg;
	if (a1-FishWidth/2<0&&c1-FishWidth/2>0)
	{
		zoomAg = abs(atan(abs((c1 - (FishWidth / 2))) / ffish) + atan(abs((a1 - (FishWidth / 2))) / ffish));
	}
	else
	{		
		zoomAg = abs(atan(abs((c1 - (FishWidth / 2))) / ffish) - atan(abs((a1 - (FishWidth / 2))) / ffish));
	}
	double angleIP = tan((double)zoomAg / 180 * PI / 2);
	double IPCameraf = 190 / 2 * angleIP;

//	z1 = (double)190 / (double)200 * (double)(190-c1 + a1) - 80;

	z1 = 200 - (c1 - a1)-80;

	char buffer5[20];
	sprintf_s(buffer5, "%f", z1);
	z1 = strtoul(buffer5, NULL, 16);

}
//����ˮƽ�Ƕ�
double CRealPlayDlg::ComputeAngle(CPoint nowpoint)
{
	CPoint CentPoint(FishWidth / 2, FishHeight / 2);

	//б�߳���
	double length = PointLegth(nowpoint, CentPoint);
	//�Ա߱�б�� sin
	double hudu;
	double ag;

	if ((CentPoint.x - nowpoint.x) <= 0 && (CentPoint.y - nowpoint.y) >= 0)
	{

		hudu = asin(abs(CentPoint.y - nowpoint.y) / length);
		ag = hudu * 180 / PI;
		ag = 90 - ag;
	}
	else if ((CentPoint.x - nowpoint.x) <= 0 && (CentPoint.y - nowpoint.y) <= 0)
	{

		hudu = asin(abs(CentPoint.y - nowpoint.y) / length);
		ag = hudu * 180 / PI;
		ag = 90 + ag;
	}
	else if ((CentPoint.x - nowpoint.x) >= 0 && (CentPoint.y - nowpoint.y) <= 0)
	{

		hudu = asin(abs(CentPoint.y - nowpoint.y) / length);
		ag = hudu * 180 / PI;
		ag = 270- ag;
	}

	else if ((CentPoint.x - nowpoint.x) >= 0 && (CentPoint.y - nowpoint.y) >= 0)
	{

		hudu = asin(abs(CentPoint.y - nowpoint.y) / length);
		ag = hudu * 180 / PI;
		ag = 270 + ag;
	}

 	return ag;
}

double CRealPlayDlg::PointLegth(CPoint pa, CPoint pb)
{
	return sqrt(pow((double)(pa.x - pb.x), 2) + pow((double)(pa.y - pb.y), 2));
}

void CRealPlayDlg::OnBnClickedButton3()
{
	NET_DVR_PTZPOS lpinBuffer;
  	DWORD size = sizeof(lpinBuffer);
	DWORD dwReturnLen;
	bool xxx = NET_DVR_GetDVRConfig(m_struDeviceInfo.lLoginID, NET_DVR_GET_PTZPOS, 1, &lpinBuffer, size,&dwReturnLen);
	if (!xxx)
	{
		DWORD	iLastErr = NET_DVR_GetLastError();
		CString str;
		str.Format(_T("NET_DVR_Login_V30 failed, error code= %d", iLastErr));
		MessageBox(str);
	}
	else
	{
		
		TRACE("λ�ò���    ˮƽ��ʮ����%0x  ʮ��%d      ��ֱ��ʮ����%0x ʮ��%d  �䱶��ʮ����%0x  ʮ��%d\n", lpinBuffer.wPanPos, lpinBuffer.wPanPos, lpinBuffer.wTiltPos, lpinBuffer.wTiltPos, lpinBuffer.wZoomPos, lpinBuffer.wZoomPos);
		
	}
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}
//��ip��ͷ�ϻ�ʮ��
void CRealPlayDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO:  �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CClientDC dc(this);
	CRect Rect;
	GetDlgItem(IDC_STATIC_PLAY)->GetWindowRect(Rect);
	ScreenToClient(Rect);
	int Wlenth = (Rect.right - Rect.left) / 10*4;
	int Hlenth = (Rect.bottom - Rect.top) / 10 * 4;
	CPen pen(PS_SOLID, 1, RGB(0, 255, 0)); ////����һ����������󣬹���ʱ���û�������
	dc.SelectObject(&pen);
	//������
	CPoint m_Start_Line_x((Rect.right + Rect.left) / 2, Rect.top + Hlenth);
	CPoint m_End_Line_x((Rect.right + Rect.left) / 2, Rect.bottom - Hlenth);
	dc.MoveTo(m_Start_Line_x);
	dc.LineTo(m_End_Line_x);
	//������
	CPoint m_Start_Line_y(Rect.left + Wlenth, (Rect.bottom + Rect.top) / 2);
	CPoint m_End_Line_y(Rect.right - Wlenth, (Rect.bottom + Rect.top) / 2);
	dc.MoveTo(m_Start_Line_y);
	dc.LineTo(m_End_Line_y);
	pen.DeleteObject();
	CDialog::OnTimer(nIDEvent);
}

//IPC��ʾ׷��Ŀ��
afx_msg LRESULT CRealPlayDlg::OnTestShowipc(WPARAM wParam, LPARAM lParam)
{
	IplImage *img1 = (IplImage *)lParam;

	//��ʾ��Ƶ
	if (img1 != NULL)
	{
		IplImage *desc = cvCreateImage(cvSize(FishWidth, FishHeight), img1->depth, img1->nChannels);
		cvResize(img1, desc, CV_INTER_CUBIC);
		CImage myImage;
		myImage.CopyOf(desc);
		CRect rect;
		CWnd *pWnd = GetDlgItem(IDC_STATIC_PLAY);
		CDC *pDC = pWnd->GetDC();
		pWnd->GetClientRect(&rect);
		pDC->SetStretchBltMode(STRETCH_HALFTONE);
		myImage.DrawToHDC(pDC->m_hDC, rect);
		ReleaseDC(pDC);
		myImage.Destroy();
		cvReleaseImage(&desc);
		pWnd = NULL;
		pDC = NULL;
	}
	img1 = NULL;
	return 0;
}

//�������
void TestOutline(IplImage* src, IplImage* dst)
{
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;
	//cvThreshold(src, src, 120, 255, CV_THRESH_BINARY);   // ��ֵ��    CV_THRESH_BINARY ��ɫȡ��
	// ��ȡ����  
	int contour_num = cvFindContours(src, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);


	cvZero(dst);        // �������  
	CvSeq *_contour = contour;
	double maxarea = 0;
	double minarea = 20;
	int m = 0;
	for (; contour != 0; contour = contour->h_next)
	{

		double tmparea = fabs(cvContourArea(contour));
		if (tmparea < minarea)
		{
			cvSeqRemove(contour, 0); // ɾ�����С���趨ֵ������  
			continue;
		}
		//CvRect aRect = cvBoundingRect(contour, 0);
		//if ((aRect.width / aRect.height)<1)
		//{
		//	cvSeqRemove(contour, 0); //ɾ����߱���С���趨ֵ������  
		//	continue;
		//}
		if (tmparea > maxarea)
		{
			maxarea = tmparea;
		}
		m++;
		// ����һ��ɫ��ֵ  
		CvScalar color = CV_RGB(0,255,0);

		//max_level �������������ȼ�������ȼ�Ϊ0�����Ƶ��������������Ϊ1��������������������ͬ�ļ���������  
		//���ֵΪ2�����е�����������ȼ�Ϊ2����������ͬ�����������е�һ���������������  
		//���ֵΪ����������������ͬ�������������������ֱ������Ϊabs(max_level)-1��������  
		cvDrawContours(dst, contour, color, color, -1, 1, 8);   //�����ⲿ���ڲ�������  
	}

	storage = NULL;
	contour = NULL;
	_contour = NULL;
}
