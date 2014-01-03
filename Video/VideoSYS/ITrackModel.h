/*
	this file is a interface for our applaction
	you can use your own track model to replace it and you must define 2 function
	1. Init
	2. Process

	we also have a interface for observer, you must define 1 function
	1.update

	track moel have observer, you can use it in Process, let it to store or something
	or, you can just don't use it and write your own Process function
*/

#ifndef _I_TRACK_MODLE_H_
#define _I_TRACK_MODLE_H_

#include <vector>

//track model
class ITrackModel
{
public:
	virtual int Init(void *para) = 0;
	virtual void Process(unsigned char *pInput, const unsigned char *pMask, const unsigned char *pBack) = 0;
	virtual void EndProcess() {}
	void Attach(class IObserver *obs) { views.push_back(obs); }
	void Notify(void *para = NULL);
	virtual ~ITrackModel() { views.clear(); }
private:
	std::vector<class IObserver *> views;
};

//observer
class IObserver
{
public:
	IObserver(ITrackModel *mod) { model = mod; model->Attach(this);}
	virtual ~IObserver() {}
	virtual void Update(void *para) = 0;
private:
	ITrackModel *model;
};

#endif
