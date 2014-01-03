#include "ITrackModel.h"

void ITrackModel::Notify(void *para)
{
	for (int i = 0; i < (int)(views.size()); i++) 
		views[i]->Update(para);
}