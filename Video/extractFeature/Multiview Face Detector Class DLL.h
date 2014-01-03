#include "FaceItem.h"

// 下列 ifdef 块是创建使从 DLL 导出更简单的
//宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MULTIVIEWFACEDETECTORCLASSDLL_EXPORTS
// 符号编译的。在使用此 DLL 的
//任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将 
// MULTIVIEWFACEDETECTORCLASSDLL_API 函数视为是从此 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef MULTIVIEWFACEDETECTORCLASSDLL_EXPORTS
#define MULTIVIEWFACEDETECTORCLASSDLL_API __declspec(dllexport)
#else
#define MULTIVIEWFACEDETECTORCLASSDLL_API __declspec(dllimport)
#endif

// 此类是从 Multiview Face Detector Class DLL.dll 导出的
class MULTIVIEWFACEDETECTORCLASSDLL_API CMultiviewFaceDetectorClassDLL {
public:
	CMultiviewFaceDetectorClassDLL(void);
	// TODO: 在此添加您的方法。
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