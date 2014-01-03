/*
	this file is a interface for our applaction
	you can use your own back model to replace it and you must define 3 function
	1. Init
	2. Process
	3. GetBack
	4. GetMask
	detils below. function EndProcess is not necessary
*/

#ifndef _I_BACK_MODLE_
#define _I_BACK_MODLE_

//back model interface
class IBackModel
{
public:
    virtual int Init(void *para) = 0;
    virtual void Process(unsigned char *pInput) = 0;
	virtual void EndProcess() {}
    virtual const unsigned char *GetBack() = 0;
    virtual const unsigned char *GetMask() = 0;
	virtual ~IBackModel() {}
};

#endif
