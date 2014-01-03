#include "track.h"

using std::vector;
using std::iterator;
using namespace std;


//Ĭ�ϲ���
VideoAnalysisPara VIDEOANALYSIS_DEFAULT_PARA = {
	100,		//��С���
	10000,		//������10000
	0.2,		//�����
	5,			//�����
	20,				//!!!������������÷ǳ���Ҫ(�����12������12)
	0.5,			//����ʧ������ frame = fps * n����ʱ�任��ɿ��Զ�ʧ��֡
	15				//���ٵ���Ч�����Ŀ
};


VideoAnalysis::VideoAnalysis(VideoAnalysisPara &para)
{
	//para
	m_BOUNDING_BOX_MIN			= para.m_BOUNDING_BOX_MIN;
	m_BOUNDING_BOX_MAX			= para.m_BOUNDING_BOX_MAX;
	m_BOUNDING_BOX_RATEMIN		= para.m_BOUNDING_BOX_RATEMIN;
	m_BOUNDING_BOX_RATEMAX		= para.m_BOUNDING_BOX_RATEMAX;
	m_DISTANCE_INTER_FRAME_MAX  = para.m_DISTANCE_INTER_FRAME_MAX;
	m_TRACK_TIME_THRESHOLD		= para.m_TRACK_TIME_THRESHOLD;									
	m_CLEARUP_TRACK_POINT_MIN	= para.m_CLEARUP_TRACK_POINT_MIN;
	//others
	m_frameCount = 0;
	m_videoWidth = 0;
	m_videoHeight = 0;
	m_videoFps = 0;
	m_pDlbkObj = NULL;
	m_pTmpFrameImg = NULL;
	m_pBackground = NULL;
	m_trackFrameThreshold = 42;		//�����ʼ����ʱ��Ḳ��
}

VideoAnalysis::~VideoAnalysis()
{
	//����۲��ߵ�ָ�룬�����ڴ�Ļ������ﲻ�������������ĵط��Լ�ȥ���
	m_observer.clear();
	m_observer.swap(vector<TrackObserver *>());
	//��֡�����Ӧ���ǿյ�
	m_boxInSingleFrame.swap(vector<ObjectBox>());
	//����δɾ��Ҳû�����ü���Ϊ��ȫ·����(���ж��ǲ���null)
	for (vector<TrackLine>::iterator iter = m_trackLineVector.begin(); iter != m_trackLineVector.end(); iter++)
	{
		if ((*iter).firstFrame != NULL)
			delete [] ((*iter).firstFrame);
		for (vector<ObjectBox>::iterator iter2 = (*iter).line.begin(); iter2 != (*iter).line.end(); iter2++)
		{
			if ((*iter2).data_org != NULL)
				delete [] ((*iter2).data_org);
			if ((*iter2).data_mask != NULL)
				delete [] ((*iter2).data_mask);
		}
	}
	//trackLineӦ���ǿյ�
	m_trackLineVector.clear();
	m_trackLineVector.swap(vector<TrackLine>());
	//�����㷨
	cvReleaseImage(&m_pBackground);
	cvReleaseImage(&m_pTmpFrameImg);
	delete m_pDlbkObj;
}

void VideoAnalysis::Release()
{
	delete this;
}

int VideoAnalysis::Init(int width, int height, int fps, DlbkPara &dlbkPara/* , VideoAnalysisPara &para*/)
{
	m_frameCount  = 0;
	m_videoWidth = width;
	m_videoHeight = height;
	m_videoFps = fps;

	m_trackFrameThreshold = (int)(m_videoFps * m_TRACK_TIME_THRESHOLD);

	m_pDlbkObj = new Cdlbk(dlbkPara);		//������������
	m_pTmpFrameImg  = cvCreateImage(cvSize(m_videoWidth, m_videoHeight), IPL_DEPTH_8U, 1);
	m_pBackground   = cvCreateImage(cvSize(m_videoWidth, m_videoHeight), IPL_DEPTH_8U, 3);

	return 0;
}

//��һ������ֻ�ǽ�һ֡�е�boxȫ������m_boxInSingleFrame
void VideoAnalysis::Process(const IplImage *pCaptureFrame)
{
	m_frameCount++;

	//ǰ150֡ȫ���������±���
	if (m_frameCount < 150)
	{
		m_pDlbkObj->Process(m_videoWidth, m_videoHeight, (unsigned char *)(pCaptureFrame->imageData));
		memcpy(m_pBackground->imageData, m_pDlbkObj->GetBack(), m_videoWidth * m_videoHeight * 3);
		return;
	}
	m_pDlbkObj->Process(m_videoWidth, m_videoHeight, (unsigned char *)(pCaptureFrame->imageData));

	//������Ҫ���box����boxInSingleFrame������
	memcpy(m_pTmpFrameImg->imageData, m_pDlbkObj->GetFore(), m_videoWidth * m_videoHeight);		//ǰ����!!!ע���ҪΪ4�ı�������Ȼ����
	memcpy(m_pBackground->imageData, m_pDlbkObj->GetBack(), m_videoWidth * m_videoHeight * 3);	//������
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;														//��storage���ڴ棬���Բ���Ҫdelete
	cvFindContours(m_pTmpFrameImg, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0)); 
	while (contour)
	{
		double tmpArea = fabs(cvContourArea(contour));
		if (tmpArea > m_BOUNDING_BOX_MIN && tmpArea <= m_BOUNDING_BOX_MAX)	//��С������С����С���������
		{
			CvRect tmpRect = cvBoundingRect(contour, 0);
			//����ȴ�����С����С���������
			if (((float)tmpRect.width / tmpRect.height) > m_BOUNDING_BOX_RATEMIN && ((float)tmpRect.width / tmpRect.height) < m_BOUNDING_BOX_RATEMAX)
			{
				ObjectBox tmpBox = {tmpRect.x, tmpRect.y, tmpRect.width, tmpRect.height, NULL, NULL, m_frameCount};
				tmpBox.data_org =  new unsigned char[tmpRect.width * tmpRect.height * 3];				//!!!��ʾ������ʱ��Ҫ��Ϊ4�ı���
				tmpBox.data_mask = new unsigned char[tmpRect.width * tmpRect.height * 1];
				//����ԭʼǰ���ʹ������ǰ��������
				for(int y = 0; y < tmpBox.box_height; y++)
				{
					int tmpStart = 3 * (tmpBox.y_pos + y) * m_videoWidth + 3 * tmpBox.x_pos;
					memcpy(tmpBox.data_org + y*3*tmpBox.box_width, (unsigned char *)(pCaptureFrame->imageData + tmpStart), tmpBox.box_width * 3);	
					int tmpStart2 = (tmpBox.y_pos + y) * m_videoWidth + tmpBox.x_pos;
					memcpy(tmpBox.data_mask + y*tmpBox.box_width, (unsigned char *)(m_pDlbkObj->GetFore() + tmpStart2), tmpBox.box_width);							
				}	
				m_boxInSingleFrame.push_back(tmpBox);		//���һ֡ͼ���з���Ҫ��Ŀ�����m_boxInSingleFrame	
			}
		}
		contour = contour->h_next;
	}
	cvReleaseMemStorage(&storage);

	ProcessTrackLine(pCaptureFrame);		//һ֡Ҳû��ҲҪ��lost
	//��·�����д�����ȡ������·����ȥ��û�õ�
	ProcessClearUpTrackLine();
}

//��һ֡�е����������͹켣��ƥ��
void VideoAnalysis::ProcessTrackLine(const IplImage *pCaptureFrame)
{
	vector<ObjectBox> copyBoxInCurFrame(m_boxInSingleFrame);

	//����ÿ��·�����ԣ���֡����ƥ���box��ô�ͼ���box����ʱ·���У����û�У���·����lost+1
	for (vector<TrackLine>::iterator iterTrackLine = m_trackLineVector.begin(); iterTrackLine != m_trackLineVector.end(); iterTrackLine++)
	{	
		vector< pair< vector<ObjectBox>::iterator, double> > matchVector;		//ƥ���еĵ���������ѹ��
		for (vector<ObjectBox>::iterator iterBoxInCurFrame = m_boxInSingleFrame.begin(); iterBoxInCurFrame != m_boxInSingleFrame.end(); iterBoxInCurFrame++)
		{
			//С�ھ�����ֵ�Ķ�����vector
			double distance = CalDistance(*iterTrackLine, *iterBoxInCurFrame);
			if (distance < m_DISTANCE_INTER_FRAME_MAX)
			{
				matchVector.push_back(pair<vector<ObjectBox>::iterator, double>(iterBoxInCurFrame, distance));
			}
		}
		//�ж����г���ƥ���е�(��matchVector��)��Ȼ�������Ӧ�Ĳ���
		if (static_cast<int>(matchVector.size()) > 0)
		{
			UpdateBestScoreLinePara(*iterTrackLine, matchVector, m_boxInSingleFrame, copyBoxInCurFrame);
		}
		//ûƥ���Ͼͽ�·����lost��1
		else
		{
			(*iterTrackLine).lostCount++;
		}
	}

	//����֡�е�box���ԣ�����ǰ֡������box��ÿ��·������ƥ�䣬ƥ�����˾ͼ��뵽·���У�ûƥ���оͽ����µ�·��
	//�����µ�·��
	for (vector<ObjectBox>::iterator iterBox = copyBoxInCurFrame.begin(); iterBox != copyBoxInCurFrame.end(); iterBox++)
	{
		if ((*iterBox).box_width != 0)	//����Ѿ�ƥ�����˵Ļ����������width��Ϊ0�������������жϲ�Ϊ0�ľͽ����µ�·��
		{	
			//��ȸ���
			ObjectBox tmpBox(*iterBox);
			tmpBox.data_org  = new unsigned char[tmpBox.box_width * tmpBox.box_height * 3];
			memcpy(tmpBox.data_org, (*iterBox).data_org, tmpBox.box_width * tmpBox.box_height * 3);
			tmpBox.data_mask = new unsigned char[tmpBox.box_width * tmpBox.box_height];
			memcpy(tmpBox.data_mask, (*iterBox).data_mask, tmpBox.box_width * tmpBox.box_height);
			unsigned char *tmpImg = new unsigned char[m_videoHeight*m_videoWidth*3];	//��ʼ֡�Ļ���
			memcpy(tmpImg, pCaptureFrame->imageData, m_videoHeight*m_videoWidth*3);
			//����·��  lost  vx vy           size                                    pro                         img                line
			TrackLine tmp = {0, 0, 0, tmpBox.box_height*tmpBox.box_width, 1.0*tmpBox.box_width/tmpBox.box_height, tmpImg, vector<ObjectBox>(1, tmpBox)};
			m_trackLineVector.push_back(tmp);
		}
	}

	//��յ�ǰ֡�е�box,��������Ŀռ�
	DeleteSingleFrameBoxData(m_boxInSingleFrame);
}


//�õ�������·��
void VideoAnalysis::ProcessClearUpTrackLine()
{
	for (vector<TrackLine>::iterator iterTrackLine = m_trackLineVector.begin(); iterTrackLine != m_trackLineVector.end(); )
	{
		TrackLine singleLine = *iterTrackLine;
		//��Ч����Ŀ����һ��ֵ�����Ѿ�lost��һ��ʱ��
		if (static_cast<int>(singleLine.line.size()) > m_CLEARUP_TRACK_POINT_MIN && singleLine.lostCount >= m_trackFrameThreshold)
		{
			//���ﻹ���Լ������������ж��ǲ���һ�������Ĺ켣
			{
				//�������лص�����
				for (vector<TrackObserver *>::iterator observerIter = m_observer.begin(); observerIter != m_observer.end(); observerIter++)
				{
					(*observerIter)->OneLineComplete(singleLine);
				}
				//ɾ������(first_frame, data_org, data_mask)
				delete [] (singleLine.firstFrame);
				singleLine.firstFrame = NULL;
				for (vector<ObjectBox>::iterator iterBox = singleLine.line.begin(); iterBox != singleLine.line.end(); iterBox++)
				{
					delete [] ((*iterBox).data_org);
					(*iterBox).data_org = NULL;
					delete [] ((*iterBox).data_mask);
					(*iterBox).data_mask = NULL;
				}
				iterTrackLine = m_trackLineVector.erase(iterTrackLine);
			}
		}
		else if (singleLine.lostCount < m_trackFrameThreshold)
		{
			iterTrackLine++;
		}
		//lost��һ��ʱ�䣬������Ч����Ŀ�ֲ������������·���������뵽����·����ȥ
		else
		{
			delete [] (singleLine.firstFrame);	//ɾ����һ֡�Ļ���
			singleLine.firstFrame = NULL;
			for (vector<ObjectBox>::iterator iterBox = singleLine.line.begin(); iterBox != singleLine.line.end(); iterBox++)
			{
				delete [] ((*iterBox).data_org);
				(*iterBox).data_org = NULL;
				delete [] ((*iterBox).data_mask);
				(*iterBox).data_mask = NULL;
			}
			iterTrackLine = m_trackLineVector.erase(iterTrackLine);
		}
	}
}


double VideoAnalysis::CalDistance(TrackLine &trackLine, const ObjectBox &box)
{
	ObjectBox lastBoxInLine = *(trackLine.line.end() - 1);
	//���Ƶ�λ�ã�����λ��Ϊԭ��
	double esPosX = lastBoxInLine.x_pos + lastBoxInLine.box_width / 2 + trackLine.speedX * (trackLine.lostCount + 1);
	double esPosY = lastBoxInLine.y_pos +lastBoxInLine.box_height / 2 + trackLine.speedY * (trackLine.lostCount + 1);
	//�������
	double det2 = (box.x_pos + box.box_width / 2- esPosX) * (box.x_pos + box.box_width / 2- esPosX) + \
		(box.y_pos + box.box_height / 2 - esPosY) * (box.y_pos + box.box_height / 2 - esPosY);
	return sqrt(det2);
}

void VideoAnalysis::UpdateBestScoreLinePara(TrackLine &trackLine, 
		const std::vector<std::pair<std::vector<ObjectBox>::iterator, double> > &iterVect,
		std::vector<ObjectBox> &originalBox, 
		std::vector<ObjectBox> &copyBox)
{
	ObjectBox lastBoxInLine = *(trackLine.line.end() - 1);
	double minScore = 420;	//42*10������һ������
	vector<pair<vector<ObjectBox>::iterator, double> >::const_iterator bestIter = iterVect.begin();

	for (vector<pair<vector<ObjectBox>::iterator, double> >::const_iterator iter = iterVect.begin(); iter != iterVect.end(); iter++)
	{
		ObjectBox singleBox   = *((*iter).first);
		double    singleDistance = (*iter).second;
		int sumScore = 0;
		//distance score
		if (singleDistance <= (m_DISTANCE_INTER_FRAME_MAX / 3.0)) sumScore += 1;
		else if (singleDistance <= (m_DISTANCE_INTER_FRAME_MAX * 2.0 / 3.0)) sumScore += 8;
		else sumScore += 64;
		//size score
		double sizeRadio = 1.0 * trackLine.size / (singleBox.box_height * singleBox.box_width);
		if (sizeRadio > 0.7 && sizeRadio < 1.5)	sumScore += 1;
		else if (sizeRadio > 0.5 && sizeRadio < 2) sumScore += 4;   //?
		else sumScore += 16;
		//pro score
		double proRadio = 1.0 * trackLine.pro / singleBox.box_width * singleBox.box_height;
		if (proRadio > 0.7 && proRadio < 1.5)	sumScore += 1;
		else if (proRadio > 0.5 && proRadio < 2) sumScore += 2;   //?
		else sumScore += 4;

		if (sumScore < minScore)
		{
			minScore = sumScore;
			bestIter = iter;
		}
	}
	
	//ֻҪ���Ǽ��˶��Ӿ���ƥ�����ˣ�Ȼ�������Ӧ�ķ�������������Ȩ�صĴ�С
	if (minScore < 84 + 1)	//64+16+4
	{
		//double updateRate = (1.8 / minScore + 0.1) * (0.5 / static_cast<int>(trackLine.line.size()) + 0.5);	//����Ȩ�أ�����Խ�ߣ�line�е���ĿԽ�ͣ�����Ȩ��ԽС
		double updateRate = 0.7;
		ObjectBox bestBox = *((*bestIter).first);

		trackLine.lostCount = 0;
		trackLine.size = trackLine.size * (1 - updateRate) + updateRate * bestBox.box_width * bestBox.box_height;
		trackLine.pro  = trackLine.pro  * (1 - updateRate) + updateRate * bestBox.box_width / bestBox.box_height;
		double speedX  = (bestBox.x_pos + bestBox.box_width / 2 - lastBoxInLine.x_pos - lastBoxInLine.box_width / 2) / (trackLine.lostCount + 1);
		double speedY  = (bestBox.y_pos + bestBox.box_height / 2 - lastBoxInLine.y_pos - lastBoxInLine.box_height / 2) / (trackLine.lostCount + 1);
		trackLine.speedX = trackLine.speedX * (1 - updateRate) + speedX * updateRate;
		trackLine.speedY = trackLine.speedY * (1 - updateRate) + speedY * updateRate;

		ObjectBox tmpBox(*((*bestIter).first));
		tmpBox.data_org  = new unsigned char[tmpBox.box_width * tmpBox.box_height * 3];
		memcpy(tmpBox.data_org, bestBox.data_org, tmpBox.box_width * tmpBox.box_height * 3);
		tmpBox.data_mask = new unsigned char[tmpBox.box_width * tmpBox.box_height];
		memcpy(tmpBox.data_mask, bestBox.data_mask, tmpBox.box_width * tmpBox.box_height);
		trackLine.line.push_back(tmpBox);
		
		(*(copyBox.begin() + ((*bestIter).first - m_boxInSingleFrame.begin()))).box_width = 0;
		//return (*matchedIter).first;
	}
	else
	{
		trackLine.lostCount++;
		//return endIter;
	}
}


void VideoAnalysis::DeleteSingleFrameBoxData(vector<ObjectBox> &singleFrameBox)
{
	for (vector<ObjectBox>::iterator iterBoxInCurFrame = singleFrameBox.begin(); iterBoxInCurFrame != singleFrameBox.end(); iterBoxInCurFrame++)
	{
		delete [] (*iterBoxInCurFrame).data_org;
		(*iterBoxInCurFrame).data_org = NULL;
		delete [] (*iterBoxInCurFrame).data_mask;
		(*iterBoxInCurFrame).data_mask = NULL;
	}
	singleFrameBox.clear();
}


void VideoAnalysis::AttachObserver(TrackObserver *observer)
{
	m_observer.push_back(observer);
}


void VideoAnalysis::DetachObserver(TrackObserver *observer)
{
	for (vector<TrackObserver *>::iterator iter = m_observer.begin(); iter != m_observer.end(); iter++)
	{
		if ((*iter) == observer)
		{
			iter = m_observer.erase(iter);
		}
	}
}

