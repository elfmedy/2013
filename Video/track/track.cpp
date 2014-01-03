#include "track.h"

using std::vector;
using std::iterator;
using namespace std;


//默认参数
VideoAnalysisPara VIDEOANALYSIS_DEFAULT_PARA = {
	100,		//最小面积
	10000,		//最大面积10000
	0.2,		//长宽比
	5,			//长宽比
	20,				//!!!这个参数的设置非常重要(最大是12，包括12)
	0.5,			//容许丢失几秒钟 frame = fps * n，将时间换算成可以丢失的帧
	15				//最少的有效点的数目
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
	m_trackFrameThreshold = 42;		//后面初始化的时候会覆盖
}

VideoAnalysis::~VideoAnalysis()
{
	//清除观察者的指针，至于内存的话，这里不清除，留给申请的地方自己去清除
	m_observer.clear();
	m_observer.swap(vector<TrackObserver *>());
	//单帧的这个应该是空的
	m_boxInSingleFrame.swap(vector<ObjectBox>());
	//最后既未删除也没有来得及成为完全路径的(先判断是不是null)
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
	//trackLine应该是空的
	m_trackLineVector.clear();
	m_trackLineVector.swap(vector<TrackLine>());
	//背景算法
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

	m_pDlbkObj = new Cdlbk(dlbkPara);		//建立背景对象
	m_pTmpFrameImg  = cvCreateImage(cvSize(m_videoWidth, m_videoHeight), IPL_DEPTH_8U, 1);
	m_pBackground   = cvCreateImage(cvSize(m_videoWidth, m_videoHeight), IPL_DEPTH_8U, 3);

	return 0;
}

//第一个函数只是将一帧中的box全部推入m_boxInSingleFrame
void VideoAnalysis::Process(const IplImage *pCaptureFrame)
{
	m_frameCount++;

	//前150帧全部用来更新背景
	if (m_frameCount < 150)
	{
		m_pDlbkObj->Process(m_videoWidth, m_videoHeight, (unsigned char *)(pCaptureFrame->imageData));
		memcpy(m_pBackground->imageData, m_pDlbkObj->GetBack(), m_videoWidth * m_videoHeight * 3);
		return;
	}
	m_pDlbkObj->Process(m_videoWidth, m_videoHeight, (unsigned char *)(pCaptureFrame->imageData));

	//将符合要求的box推入boxInSingleFrame向量中
	memcpy(m_pTmpFrameImg->imageData, m_pDlbkObj->GetFore(), m_videoWidth * m_videoHeight);		//前景。!!!注意宽要为4的倍数，不然出错
	memcpy(m_pBackground->imageData, m_pDlbkObj->GetBack(), m_videoWidth * m_videoHeight * 3);	//背景。
	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contour = 0;														//和storage共内存，所以不需要delete
	cvFindContours(m_pTmpFrameImg, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0)); 
	while (contour)
	{
		double tmpArea = fabs(cvContourArea(contour));
		if (tmpArea > m_BOUNDING_BOX_MIN && tmpArea <= m_BOUNDING_BOX_MAX)	//大小大于最小下限小于最大上限
		{
			CvRect tmpRect = cvBoundingRect(contour, 0);
			//长宽比大于最小上限小于最大上限
			if (((float)tmpRect.width / tmpRect.height) > m_BOUNDING_BOX_RATEMIN && ((float)tmpRect.width / tmpRect.height) < m_BOUNDING_BOX_RATEMAX)
			{
				ObjectBox tmpBox = {tmpRect.x, tmpRect.y, tmpRect.width, tmpRect.height, NULL, NULL, m_frameCount};
				tmpBox.data_org =  new unsigned char[tmpRect.width * tmpRect.height * 3];				//!!!显示出来的时候要调为4的倍数
				tmpBox.data_mask = new unsigned char[tmpRect.width * tmpRect.height * 1];
				//复制原始前景和处理过的前景的数据
				for(int y = 0; y < tmpBox.box_height; y++)
				{
					int tmpStart = 3 * (tmpBox.y_pos + y) * m_videoWidth + 3 * tmpBox.x_pos;
					memcpy(tmpBox.data_org + y*3*tmpBox.box_width, (unsigned char *)(pCaptureFrame->imageData + tmpStart), tmpBox.box_width * 3);	
					int tmpStart2 = (tmpBox.y_pos + y) * m_videoWidth + tmpBox.x_pos;
					memcpy(tmpBox.data_mask + y*tmpBox.box_width, (unsigned char *)(m_pDlbkObj->GetFore() + tmpStart2), tmpBox.box_width);							
				}	
				m_boxInSingleFrame.push_back(tmpBox);		//最后将一帧图像中符合要求的框推入m_boxInSingleFrame	
			}
		}
		contour = contour->h_next;
	}
	cvReleaseMemStorage(&storage);

	ProcessTrackLine(pCaptureFrame);		//一帧也没有也要算lost
	//对路径进行处理提取出完整路径，去掉没用的
	ProcessClearUpTrackLine();
}

//将一帧中的所有物体框和轨迹做匹配
void VideoAnalysis::ProcessTrackLine(const IplImage *pCaptureFrame)
{
	vector<ObjectBox> copyBoxInCurFrame(m_boxInSingleFrame);

	//对于每条路径而言：此帧中有匹配的box那么就加入box到临时路径中，如果没有，则路径的lost+1
	for (vector<TrackLine>::iterator iterTrackLine = m_trackLineVector.begin(); iterTrackLine != m_trackLineVector.end(); iterTrackLine++)
	{	
		vector< pair< vector<ObjectBox>::iterator, double> > matchVector;		//匹配中的迭代器都被压入
		for (vector<ObjectBox>::iterator iterBoxInCurFrame = m_boxInSingleFrame.begin(); iterBoxInCurFrame != m_boxInSingleFrame.end(); iterBoxInCurFrame++)
		{
			//小于距离阈值的都放入vector
			double distance = CalDistance(*iterTrackLine, *iterBoxInCurFrame);
			if (distance < m_DISTANCE_INTER_FRAME_MAX)
			{
				matchVector.push_back(pair<vector<ObjectBox>::iterator, double>(iterBoxInCurFrame, distance));
			}
		}
		//判断所有初步匹配中的(在matchVector中)，然后更像相应的参数
		if (static_cast<int>(matchVector.size()) > 0)
		{
			UpdateBestScoreLinePara(*iterTrackLine, matchVector, m_boxInSingleFrame, copyBoxInCurFrame);
		}
		//没匹配上就将路径的lost加1
		else
		{
			(*iterTrackLine).lostCount++;
		}
	}

	//对于帧中的box而言：将当前帧的所有box和每条路径进行匹配，匹配中了就加入到路径中，没匹配中就建立新的路径
	//建立新的路径
	for (vector<ObjectBox>::iterator iterBox = copyBoxInCurFrame.begin(); iterBox != copyBoxInCurFrame.end(); iterBox++)
	{
		if ((*iterBox).box_width != 0)	//如果已经匹配上了的话，会把它的width置为0，所以这里是判断不为0的就建立新的路径
		{	
			//深度复制
			ObjectBox tmpBox(*iterBox);
			tmpBox.data_org  = new unsigned char[tmpBox.box_width * tmpBox.box_height * 3];
			memcpy(tmpBox.data_org, (*iterBox).data_org, tmpBox.box_width * tmpBox.box_height * 3);
			tmpBox.data_mask = new unsigned char[tmpBox.box_width * tmpBox.box_height];
			memcpy(tmpBox.data_mask, (*iterBox).data_mask, tmpBox.box_width * tmpBox.box_height);
			unsigned char *tmpImg = new unsigned char[m_videoHeight*m_videoWidth*3];	//起始帧的画面
			memcpy(tmpImg, pCaptureFrame->imageData, m_videoHeight*m_videoWidth*3);
			//加入路径  lost  vx vy           size                                    pro                         img                line
			TrackLine tmp = {0, 0, 0, tmpBox.box_height*tmpBox.box_width, 1.0*tmpBox.box_width/tmpBox.box_height, tmpImg, vector<ObjectBox>(1, tmpBox)};
			m_trackLineVector.push_back(tmp);
		}
	}

	//清空当前帧中的box,包括申请的空间
	DeleteSingleFrameBoxData(m_boxInSingleFrame);
}


//得到完整的路径
void VideoAnalysis::ProcessClearUpTrackLine()
{
	for (vector<TrackLine>::iterator iterTrackLine = m_trackLineVector.begin(); iterTrackLine != m_trackLineVector.end(); )
	{
		TrackLine singleLine = *iterTrackLine;
		//有效点数目大于一定值并且已经lost了一段时间
		if (static_cast<int>(singleLine.line.size()) > m_CLEARUP_TRACK_POINT_MIN && singleLine.lostCount >= m_trackFrameThreshold)
		{
			//这里还可以加其他的条件判断是不是一条完整的轨迹
			{
				//调用所有回调函数
				for (vector<TrackObserver *>::iterator observerIter = m_observer.begin(); observerIter != m_observer.end(); observerIter++)
				{
					(*observerIter)->OneLineComplete(singleLine);
				}
				//删除数据(first_frame, data_org, data_mask)
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
		//lost了一定时间，并且有效点数目又不够，清除这条路径，不加入到完整路径中去
		else
		{
			delete [] (singleLine.firstFrame);	//删除第一帧的画面
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
	//估计的位置，中心位置为原点
	double esPosX = lastBoxInLine.x_pos + lastBoxInLine.box_width / 2 + trackLine.speedX * (trackLine.lostCount + 1);
	double esPosY = lastBoxInLine.y_pos +lastBoxInLine.box_height / 2 + trackLine.speedY * (trackLine.lostCount + 1);
	//计算误差
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
	double minScore = 420;	//42*10，随便的一个大数
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
	
	//只要不是极端恶劣就算匹配上了，然后根据相应的分数来决定更新权重的大小
	if (minScore < 84 + 1)	//64+16+4
	{
		//double updateRate = (1.8 / minScore + 0.1) * (0.5 / static_cast<int>(trackLine.line.size()) + 0.5);	//更新权重，分数越高，line中的数目越低，更新权重越小
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

