/*
	this file is about store data

	we have 4 files part(3/4 is file system, 1/4 is sqlserver)
	1. path to store the overallimage(a image data a file, /overallImage/000000001 ,etc.)
	2. image feat(each file struct PictureFeatStruct[100,003], /pictureFeat/feat_001, etc.)
	3. sql id(store the sqlserver id serials, can be improved)
	4. sqlserver(store struct SqlStoreData, val TLV4.0 C++ templete)

	read & write file can be faster, i didn't read and write a file in the same time, you can modify source code to do this
	use file's in & out mode

	2012.07.17
*/

#ifndef _SQL_STORE_H_
#define _SQL_STORE_H_

#include "time.h"
#include <bitset>
#include <vector>

#define OTL_ODBC_MSSQL_2005		//use odbc
#include "otlv4.h"				//XXX:OTL SQL2005 ODBC . set correct


//parament
struct SqlStorePara
{
	int m_isNewStart;			//if set to 1, the whole data store(include sql database) will set empty.

	int m_cameraID;
	int m_startYear;
	int m_startMonth;
	int m_startDay;
	int m_startHour;
	int m_startMinute;
	int m_startSecond;

	char m_overviewImagePath[256];
	char m_sqlParaPath[256];
	char m_pictureFeatPath[256];
};


//default parament
extern struct SqlStorePara SQLSTORE_DEFAULT_PARA;


//sql data struct
struct SqlStoreData
{
	int id;

	//basic
	int cameraID;
	otl_datetime startTime;
	otl_datetime endTime;
	int objType;

	//personal
	int basicColor;
};


//pictureFeat file struct. 1 file = struct PictureFeatStruct[100]
struct PictureFeatStruct
{
	int count;
	unsigned char originalArr[14];
	int idArr[100];
};


//sql database class. all the function relate to sql store
class SqlStore
{
public:
	int Init(struct SqlStorePara *para);
	int Store();
	int EndStore();
public:
	int InitPictureFeat();
	int InitSql();
public:
	void SetTime(int startMs, int videoFps, int startFC, int endFC);
	void SetOverallImage(unsigned char *data, int width, int height);
	void SetBasicColor(unsigned int color);
	void SetPictureFeat(unsigned char *feat, int featArrSize);
private:
	void ConnectToDataBase();
	void Tm2DateTime(struct tm &oTime, otl_datetime &dTime);
	void SavPictureFeat(int hashValue, unsigned char *feat, int featArrSize);
private:
	otl_connect m_db;
	struct SqlStoreData curSqlData;		//data to be store to sqlserver.several observer change this data.
private:
	int m_isNewStart;
	int m_cameraId;
	time_t m_startTime;
	char m_overviewImagePath[256];
	char m_sqlParaPath[256];
	char m_pictureFeatPath[256];
};

#endif
