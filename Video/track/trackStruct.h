#ifndef _VIDEO_TRACK_STRUCT_H_
#define _VIDEO_TRACK_STRUCT_H_

#include <vector>

//视频的参数
struct VideoPara
{
	int    m_BOUNDING_BOX_MIN;			//图像中框的大小下限
	int    m_BOUNDING_BOX_MAX;			//图像中框的大小上限
	double m_BOUNDING_BOX_RATEMIN;		//长宽比最小值(长不一定比宽大,只是width/height)
	double m_BOUNDING_BOX_RATEMAX;		//长宽比最大值
	int	   m_ISDEBUG;					//是不是运行在debug模式下
};


//轨迹匹配的参数
struct VideoAnalysisPara
{
	int m_DISTANCE_INTER_FRAME_MAX;	//两次匹配之间允许的最小的距离
	int m_TRACK_NUM_THRESHOLD;		//匹配轨迹时轨迹中间容许丢失几帧
	int m_CLEARUP_TRACK_POINT_MIN;	//轨迹中最少的有效点的数目
};


//帧中的物体框结构体
typedef struct objectBox_struct
{
	int				x_pos;			//box左下角的x坐标
	int				y_pos;			//box左下角的y坐标
	int				box_width;		//box的宽度
	int				box_height;		//box的高度
	unsigned char	*data_org;		//box的原始图片
	unsigned char	*data_mask;		//box的前景
	long			frame_count;	//第几帧
}ObjectBox;


//轨迹的结构体
typedef struct trackLine_struct
{
	int lostCount;					//丢失计数
	double speedX;					//x方向速度
	double speedY;					//y方向速度
	double size;					//大小
	double pro;						//长宽比 width / height
	unsigned char *firstFrame;		//此轨迹第一帧的视频(用来显示搜索结果)
	std::vector<ObjectBox> line;	//轨迹框
}TrackLine;

//轨迹处理的观察者
class TrackObserver
{
public:
	virtual void OneLineComplete(const TrackLine & trackLine) = 0;
	virtual ~TrackObserver(){}
protected:
};

#endif