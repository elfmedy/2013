#include "FaceItem.h"

// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
//��ı�׼�������� DLL �е������ļ��������������϶���� MULTIVIEWFACEDETECTORCLASSDLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
//�κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ 
// MULTIVIEWFACEDETECTORCLASSDLL_API ������Ϊ�ǴӴ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef MULTIVIEWFACEDETECTORCLASSDLL_EXPORTS
#define MULTIVIEWFACEDETECTORCLASSDLL_API __declspec(dllexport)
#else
#define MULTIVIEWFACEDETECTORCLASSDLL_API __declspec(dllimport)
#endif

// �����Ǵ� Multiview Face Detector Class DLL.dll ������
class MULTIVIEWFACEDETECTORCLASSDLL_API CMultiviewFaceDetectorClassDLL {
public:
	CMultiviewFaceDetectorClassDLL(void);
	// TODO: �ڴ�������ķ�����
};

extern MULTIVIEWFACEDETECTORCLASSDLL_API int nMultiviewFaceDetectorClassDLL;

MULTIVIEWFACEDETECTORCLASSDLL_API int fnMultiviewFaceDetectorClassDLL(void);

class CFaceDetector;

class FaceDetectorClass
{
private:
	CFaceDetector *mpFaceDetector;
	SFaceItems mFaces;
public:
	MULTIVIEWFACEDETECTORCLASSDLL_API FaceDetectorClass();
	MULTIVIEWFACEDETECTORCLASSDLL_API ~FaceDetectorClass();
	MULTIVIEWFACEDETECTORCLASSDLL_API int Detect(const void *pImage, SFaceItem *pFaces, int MaxNumFace);
};