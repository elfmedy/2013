#ifndef _VIDEO_TRACK_STRUCT_H_
#define _VIDEO_TRACK_STRUCT_H_

#include <vector>

//��Ƶ�Ĳ���
struct VideoPara
{
	int    m_BOUNDING_BOX_MIN;			//ͼ���п�Ĵ�С����
	int    m_BOUNDING_BOX_MAX;			//ͼ���п�Ĵ�С����
	double m_BOUNDING_BOX_RATEMIN;		//�������Сֵ(����һ���ȿ��,ֻ��width/height)
	double m_BOUNDING_BOX_RATEMAX;		//��������ֵ
	int	   m_ISDEBUG;					//�ǲ���������debugģʽ��
};


//�켣ƥ��Ĳ���
struct VideoAnalysisPara
{
	int m_DISTANCE_INTER_FRAME_MAX;	//����ƥ��֮���������С�ľ���
	int m_TRACK_NUM_THRESHOLD;		//ƥ��켣ʱ�켣�м�����ʧ��֡
	int m_CLEARUP_TRACK_POINT_MIN;	//�켣�����ٵ���Ч�����Ŀ
};


//֡�е������ṹ��
typedef struct objectBox_struct
{
	int				x_pos;			//box���½ǵ�x����
	int				y_pos;			//box���½ǵ�y����
	int				box_width;		//box�Ŀ��
	int				box_height;		//box�ĸ߶�
	unsigned char	*data_org;		//box��ԭʼͼƬ
	unsigned char	*data_mask;		//box��ǰ��
	long			frame_count;	//�ڼ�֡
}ObjectBox;


//�켣�Ľṹ��
typedef struct trackLine_struct
{
	int lostCount;					//��ʧ����
	double speedX;					//x�����ٶ�
	double speedY;					//y�����ٶ�
	double size;					//��С
	double pro;						//����� width / height
	unsigned char *firstFrame;		//�˹켣��һ֡����Ƶ(������ʾ�������)
	std::vector<ObjectBox> line;	//�켣��
}TrackLine;

//�켣����Ĺ۲���
class TrackObserver
{
public:
	virtual void OneLineComplete(const TrackLine & trackLine) = 0;
	virtual ~TrackObserver(){}
protected:
};

#endif