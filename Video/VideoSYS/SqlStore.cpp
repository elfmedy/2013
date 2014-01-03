#include "SqlStore.h"
#include "windows.h"
#include <fstream>
#include "string.h"
#include <string>
#include <iostream>

using namespace std;

struct SqlStorePara SQLSTORE_DEFAULT_PARA = {
	1,						//is new to start

	0,						//cameraId
	0,						//year
	0,						//month
	0,						//day
	0,						//hour
	0,						//minute
	0,						//second
	"overallImage\\",		//'overallImage' path
	"sqlPara\\",			//'sqlPara' path. actually, store the sql iterator 
	"pictureFeat\\"			//'pictureFeat' path
};


int SqlStore::Init(struct SqlStorePara *para)
{
	//parameter init
	m_isNewStart = para->m_isNewStart;
	m_cameraId   = para->m_cameraID;

	struct tm mStartTime;
	mStartTime.tm_year = para->m_startYear - 1900;
	mStartTime.tm_mon  = para->m_startMonth - 1;
	mStartTime.tm_mday = para->m_startDay;
	mStartTime.tm_hour = para->m_startHour;
	mStartTime.tm_min  = para->m_startMinute;
	mStartTime.tm_sec  = para->m_startSecond;
	m_startTime = mktime(&mStartTime);

	strcpy(m_overviewImagePath, para->m_overviewImagePath);
	strcpy(m_sqlParaPath, para->m_sqlParaPath);
	strcpy(m_pictureFeatPath, para->m_pictureFeatPath);

	//itit the 'pictureFeat' file
	if (InitPictureFeat() != 0)
	{
		printf("SqlStore::Init() error, InitPictureFeat()\n");
		return -1;
	}

	//init the sql ini file which store the sql [id]
	if (InitSql() != 0)
	{
		printf("SqlStore::Init() error, InitSql()\n");
		return -1;
	}

	return 0;
}


int SqlStore::InitPictureFeat()
{
	//create 10 hash files. each hash file have 100,003 hashvalue, each hashvalue can store 100 'id'
	//10,003 is a prime number
	for (int i = 0; i < 10; i++)
	{
		char tmpStr[256];
		sprintf(tmpStr, "%sfeat_%03d", m_pictureFeatPath, i + 1);
		fstream fin(tmpStr, ios_base::in | ios_base::binary);
		bool isOpen = fin.is_open();
		fin.close();
		if (!isOpen || m_isNewStart)
		{
			fstream fout(tmpStr, ios_base::out | ios_base::binary);
			struct PictureFeatStruct tmp;
			tmp.count = 0;
			for (int i = 0; i < 100003; i++)
			{
				fout.write((char *)&tmp, sizeof(struct PictureFeatStruct));
			}
			fout.close();
		}
		
	}
	return 0;
}


//use file to store the sql database's [id]. different video with continuous [id] num.
//TODO: this can be improve. you can store this id in a sql table
int SqlStore::InitSql()
{
	//init the sqlPara file, also init curSqlData here
	string path(m_sqlParaPath);
	path = path + "sqlid";
	fstream fin(path.c_str(), ios_base::in | ios_base::binary);
	if (!fin.is_open() || m_isNewStart)
	{
		fin.close();
		fstream fout(path.c_str(), ios_base::out | ios_base::binary);
		int idCount   = 1;
		curSqlData.id = 1;
		fout.write((char *)&idCount, sizeof(int));
		fout.close();
	}
	else
	{
		fin.read((char *)&(curSqlData.id), sizeof(int));
		fin.close();
	}
	curSqlData.cameraID = m_cameraId;

	//create sql table, and create 'stored procedure'
	try
	{
		otl_connect::otl_initialize();
		ConnectToDataBase();
		if (m_isNewStart)	//if set newStart, first del old table and stored procedure, then new them
		{
			otl_cursor::direct_exec(m_db, "drop procedure my_proc", otl_exception::disabled);
			otl_cursor::direct_exec(m_db, "drop table video", otl_exception::disabled);	
			otl_cursor::direct_exec(m_db, 
				"create table video("
				"id int primary key not null, "
				"cameraid int, "
				"starttime datetime, "
				"endtime datetime, "
				"objtype int, "
				"basiccolor int)"
			);
			otl_cursor::direct_exec(m_db,
				"create procedure my_proc "
				" @id int, "
				" @cameraid int, "
				" @starttime datetime, "
				" @endtime datetime, "
				" @objtype int, "
				" @basiccolor int "
				"as "
				" insert into video ("
				" id, "
				" cameraid, "
				" starttime, "
				" endtime, "
				" objtype, "
				" basiccolor "
				") values ("
				" @id, "
				" @cameraid, "
				" @starttime, "
				" @endtime, "
				" @objtype, "
				" @basiccolor)"
			);  // create stored procedure
		}
	}
	catch(otl_exception& p)
	{
		cout << p.msg << endl; 
		cout << p.stm_text << endl; 
		cout << p.sqlstate << endl; 
		cout << p.var_info << endl;
		m_db.logoff();
		printf("we have some error in init sql. getchar() to exit\n");
		getchar();
		return -1;
	}

	m_db.logoff();

	ConnectToDataBase();	//after ini better logoff then login to write data
	return 0;
}


int SqlStore::Store()
{
	otl_stream o(1,				// buffer size should be equal to 1 in case of stored procedure call
		"{call my_proc("
		" :id<int,in>, "
		" :cameraid<int,in>, "
		" :starttime<timestamp,in>, "
		" :endtime<timestamp,in>, "
		" :objtype<int,in>, "
		" :basiccolor<int,in> "
		")}",
		m_db
	);
	o.set_commit(0);			// set stream auto-commit off since the stream does not generate transaction

	//HACK:it's better first write 'next id' to file. since sudden shut down would result the id in file is big than in sqlserver, we still can write correct
	//contrary, next sql 'insert' will fail
	int tmpNextId = curSqlData.id + 1;
	string sqlPath(m_sqlParaPath);
	sqlPath = sqlPath + "sqlid";
	fstream fout(sqlPath.c_str(), ios_base::out | ios_base::app | ios_base::binary);
	fout.write((char *)&tmpNextId, sizeof(int));
	fout.close();

	o << curSqlData.id 
	  << curSqlData.cameraID 
	  << curSqlData.startTime 
	  << curSqlData.endTime 
	  << curSqlData.objType 
	  << curSqlData.basicColor;
	m_db.commit();				//XXX:official site didn't have this, but it's important,through,in sudden exit, data can't be writed into sqlserver

	curSqlData.id += 1;

	return 0;
}


int SqlStore::EndStore()
{
	m_db.logoff();
	return 0;
}


inline void SqlStore::ConnectToDataBase()
{
	m_db.rlogon("UID=sa;PWD=1598741;DSN=video");
}


void SqlStore::Tm2DateTime(struct tm &oTime, otl_datetime &dTime)
{
	dTime.year   = oTime.tm_year + 1900;
	dTime.month  = oTime.tm_mon + 1;
	dTime.day    = oTime.tm_mday;
	dTime.hour   = oTime.tm_hour;
	dTime.minute = oTime.tm_min;
	dTime.second = oTime.tm_sec;
	dTime.frac_precision = 0;
	dTime.fraction		 = 0;
}


void SqlStore::SetTime(int startMs, int videoFps, int startFC, int endFC)
{
	time_t tmpTime1 = m_startTime + startMs / 1000;
	time_t tmpTime2 = tmpTime1 + (endFC - startFC) / videoFps;
	struct tm startTm, endTm;
	localtime_s(&startTm, &tmpTime1);
	localtime_s(&endTm, &tmpTime2);
	Tm2DateTime(startTm, curSqlData.startTime);
	Tm2DateTime(endTm, curSqlData.endTime);
}


//store the img data to file. just the Iplimage's imageData. width is widthstep
void SqlStore::SetOverallImage(unsigned char *data, int width, int height)
{
	string path(m_overviewImagePath);
	char tmpStr[256];
	sprintf(tmpStr, "%s%09d", path.c_str(), curSqlData.id);
	fstream fout(tmpStr, ios_base::out | ios_base::binary);
	fout.write((char *)&width, sizeof(int));
	fout.write((char *)&height, sizeof(int));
	fout.write((char *)data, width * height * 3);
	fout.close();
}


void SqlStore::SetBasicColor(unsigned int color)
{
	curSqlData.basicColor = color;
}


//get the hash value to the feat, then store the sql id in the 'pictureFeat' file
//100,003 hash array, 14 length feat
void SqlStore::SetPictureFeat(unsigned char *feat, int featArrSize)
{
	//cal the hash value to feat array
	unsigned int seed = 131;	//31 131 1313 131313 etc..  (bkdr hash seeds)
	unsigned int hashValue = 0;
	for (int i = 0; i < featArrSize; i++)
	{
		hashValue = hashValue * seed + feat[i];
	}
	hashValue = (hashValue & 0x7FFFFFFF) % 100003;	

	string featPath(m_pictureFeatPath);
	featPath = featPath + "feat_001";
	fstream firstFeatFile(featPath.c_str(), ios_base::in | ios_base::binary);
	
	//confirm the hash value. if conflict, change it and try again.
	struct PictureFeatStruct tmp;
	int loopCount = 0;
	while (loopCount < 10)	//HACK:if loopCount is big than 10, exit
	{
		firstFeatFile.seekg(hashValue * sizeof(struct PictureFeatStruct));
		firstFeatFile.read((char *)(&tmp),sizeof(struct PictureFeatStruct));
		if ( tmp.count == 0 )
		{
			break;
		}
		else
		{
			int equalFlag = 1;
			for (int k = 0; k < featArrSize; k++)
			{
				if ( (tmp.originalArr)[k] != feat[k] )
				{
					equalFlag = 0;
					break;
				}
			}
			if (equalFlag == 1)
			{
				break;
			}
			else
			{
				hashValue = (hashValue + 1) % 100003;
			}
		}
		loopCount++;
	}
	firstFeatFile.close();

	//id -> feat file[hash code]
	SavPictureFeat(hashValue, feat, featArrSize);
}


void SqlStore::SavPictureFeat(int hashValue, unsigned char *feat, int featArrSize)
{
	struct PictureFeatStruct firstTmpStruct;
	string featPath(m_pictureFeatPath);
	featPath = featPath + "feat_001";
	
	//read first hash file, hash value struct -> firstTmpStruct
	fstream firstFeatFile(featPath.c_str(), ios_base::in | ios_base::binary);	
	firstFeatFile.seekg(hashValue * sizeof(struct PictureFeatStruct));
	firstFeatFile.read((char *)(&firstTmpStruct),sizeof(struct PictureFeatStruct));
	firstFeatFile.close();

	//firstTmpStruct -> first feat file
	int sumCount = firstTmpStruct.count;
	int fileNum  = sumCount / 100 + 1;
	int idOffset = sumCount % 100;
	if (fileNum >= 1000)
	{
		printf("SavPictureFeat error, we have 1000 files. getchar() to continue.\n");
		getchar();
		return;
	}
	firstTmpStruct.count += 1;
	if (sumCount == 0)
	{
		for (int i = 0; i < featArrSize; i++) 
		{
			firstTmpStruct.originalArr[i] = feat[i];
		}
	}
	if (fileNum == 1)
	{
		firstTmpStruct.idArr[idOffset] = curSqlData.id;
	}
	fstream firstFileOut(featPath.c_str(), ios_base::in | ios_base::out | ios_base::binary);	//XXX:original is ios_base::out | ios_base::app | ios_base::binary
	firstFileOut.seekp(hashValue * sizeof(struct PictureFeatStruct));							//seek can only reach the ending of file
	firstFileOut.write((char *)&firstTmpStruct, sizeof(struct PictureFeatStruct));				//so, when you want to modify certain line , it must be in & out mode
	firstFileOut.close();																	

	//write other feat file. first read to struct, then write 
	if (fileNum > 1)
	{
		struct PictureFeatStruct tmp;

		//hash -> struct
		char tmpStr[256];
		sprintf(tmpStr, "%sfeat_%03d", m_pictureFeatPath, fileNum);
		fstream fin(tmpStr, ios_base::in | ios_base::binary);
		if (!fin.is_open())
		{
			fstream fout(tmpStr, ios_base::out | ios_base::binary);
			for (int i = 0; i < 100003; i++)
			{
				fout.write((char *)&tmp, sizeof(struct PictureFeatStruct));
			}
			fout.close();
		}
		else
		{
			fin.seekg(hashValue * sizeof(struct PictureFeatStruct));
			fin.read((char *)&tmp, sizeof(struct PictureFeatStruct));
		}
		fin.close();

		//struct -> hash. count and originalArr is invid
		fstream fout(tmpStr, ios_base::in | ios_base::out | ios_base::binary);	//XXX:see above
		tmp.idArr[idOffset] = curSqlData.id;
		fout.seekp(hashValue * sizeof(struct PictureFeatStruct));
		fout.write((char *)&tmp, sizeof(struct PictureFeatStruct));
		fout.close();
	}
}