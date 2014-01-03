
#include "ifacebox.h"
#include "FaceDetector.h"


/*
--------------------------------------------------------------------------------
FaceDll类 继承自IfaceDll
--------------------------------------------------------------------------------
*/
class FaceDll : public IFaceDll
{
public:
	FaceDll();
	~FaceDll();
	void Detect(IplImage * srcImage, SFaceItems &faceItems);
	void Release();
private:
	CFaceDetector *m_pFaceDetector;		//detector对象指针
};

FaceDll::FaceDll()
{
	m_pFaceDetector = new CFaceDetector();
}

FaceDll::~FaceDll()
{
	if (m_pFaceDetector != NULL)
	{
		delete m_pFaceDetector;
		m_pFaceDetector = NULL;
	}
}

void FaceDll::Detect(IplImage *srcImage, SFaceItems &faceItems)
{
	m_pFaceDetector->Detect(srcImage, faceItems);
	//if(mFaces.size() > MaxNumFace)
	//{
	//	std::cout << "mFaces.size() > MaxNumFace" << std::endl;
	//	exit(-1);
	//}
	//memcpy(pFaces, &mFaces[0], sizeof(SFaceItem) * mFaces.size());
	//return mFaces.size();	
}

void FaceDll::Release()
{
	delete this;
}


/*
--------------------------------------------------------------------------------
dll导出的函数
--------------------------------------------------------------------------------
*/
IFaceDll * _stdcall GetFaceDll()
{
	return new FaceDll();
}

void _stdcall ReleaseFaceDll(IFaceDll *obj)
{
	obj->Release();
}