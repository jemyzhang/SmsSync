#pragma once
#include "sqlite3\sqlite3.h"
#include "mz_commonfunc.h"
using namespace MZ_CommonFunc;

using namespace std;

#ifdef _DEBUG
#define DEFAULT_DB		L"Program Files\\M8SmsBackup\\sms.db"
#else
#define DEFAULT_DB		L"\\Disk\\Programs\\M8SmsBackup\\sms.db"
#endif

#define TABLE_CONTACT	L"contacts_v1"
#define TABLE_SMS	L"sms_v1"
#define TABLE_TEMP	L"#sms#"

#define CREATE_CONTACT_TBL L"CREATE TABLE IF NOT EXISTS '%s' (PhoneNumber text primary key,Name text NOT NULL,Label numeric NOT NULL)"
#define CREATE_SMS_TBL L"CREATE TABLE IF NOT EXISTS  '%s' (PN text not null, PhoneNumber text not null, Content text not null, TimeStamps datetime not null, SendReceive numeric not null)"
#define CREATE_TEMP_TBL L"create temp table if not exists '%s' (name text,content text,timestamps datetime,sendreceive numeric)"

#define INSERT_CONTACT L"INSERT INTO '%s' (PhoneNumber,Name,Label) values('%s','%s',%d)"
#define INSERT_SMS L"INSERT INTO '%s' (PN,PhoneNumber,Content,TimeStamps,SendReceive) values('%s','%s','%s','%04d-%02d-%02d %02d:%02d:%02d',%d)"

#define CONTACT_EQU_CONDITION   L"where PhoneNumber='%s' and Name='%s' and Label=%d"
#define COUNT_SELECT_CONTACT  L"select count(*) from '%s' "CONTACT_EQU_CONDITION
#define DELETE_SELECT_CONTACT  L"delete from '%s' "CONTACT_EQU_CONDITION
#define UPDATE_SELECT_CONTACT	L"update '%s' set Name='%s',Label=%d where PhoneNumber='%s'"

#define SMS_EQU_CONDITION   L"where PhoneNumber='%s' and Content='%s' and TimeStamps='%04d-%02d-%02d %02d:%02d:%02d' and SendReceive=%d"
#define COUNT_SELECT_SMS  L"select count(*) from '%s' "SMS_EQU_CONDITION
#define DELETE_SELECT_SMS  L"delete from '%s' "SMS_EQU_CONDITION

        /* 按照姓名检索sms时
//创建临时关联数据库
create temp table if not exists exec (name text,telnumber text, content text,timestamps datetime,sendreceive numeric)
//插入临时关联数据库，以姓名替代号码，当联系人中存在该号码时
insert into exec (name,telnumber,content,timestamps,sendreceive) 
        select contacts_v1.Name as name, sms_v1.PN, sms_v1.content as content ,sms_v1.timestamps as timestamps,sms_v1.sendreceive as sendreceive
                from contacts_v1,sms_v1 where (contacts_v1.PhoneNumber =  sms_v1.PN)
//插入临时关联数据库，以号码作为姓名，当联系人中不存在该号码时
insert into exec (name,telnumber,content,timestamps,sendreceive)
    select PN as name,PN, content,timestamps,sendreceive 
                from sms_v1 where (select count(*) from contacts_v1 where contacts_v1.PhoneNumber ==  sms_v1.PN)=0
//sms联系人列表
select distinct name from exec
*/

        /*日期列表生成方法
//ref:http://www.sqlite.org/lang_datefunc.html
日期
select distinct date(timestamps) from exec
//年
select distinct strftime('%Y',timestamps) from exec
//月
select distinct strftime('%m',timestamps) from exec where strftime('%Y',timestamps)='2009'
//日
select distinct strftime('%d',timestamps) from exec where strftime('%Y%m',timestamps)='200912'
*/

        typedef list<LPWSTR> TelNumbers_t,*TelNumbers_ptr;
typedef enum TelLabelType{
    MOBILETEL	=	0,
    WORKTEL		=	1,
    HOMETEL		=	2,
    HOMETEL2	=	3,
}TelLabel_t;

typedef struct ContactData{
    LPWSTR	Name;
    TelNumbers_t	MobileTels;
    TelNumbers_t    WorkTels;
    TelNumbers_t    HomeTels;
    TelNumbers_t    HomeTel2s;
    void Reset(){    //清空，以便下次使用
        if(Name){
            delete [] Name;
            Name = NULL;
        }
        TelNumbers_t::iterator i;
        if(MobileTels.size()){
            for(i = MobileTels.begin(); i != MobileTels.end(); i ++){
                LPWSTR strNum = *i;
                if(strNum) delete [] strNum;
            }
            MobileTels.clear();
        }
        if(WorkTels.size()){
            for(i = WorkTels.begin(); i != WorkTels.end(); i ++){
                LPWSTR strNum = *i;
                if(strNum) delete [] strNum;
            }
            WorkTels.clear();
        }
        if(HomeTels.size()){
            for(i = HomeTels.begin(); i != HomeTels.end(); i ++){
                LPWSTR strNum = *i;
                if(strNum) delete [] strNum;
            }
            HomeTels.clear();
        }
        if(HomeTel2s.size()){
            for(i = HomeTel2s.begin(); i != HomeTel2s.end(); i ++){
                LPWSTR strNum = *i;
                if(strNum) delete [] strNum;
            }
            HomeTel2s.clear();
        }
    }
    ContactData(){
        Name = NULL;
    }
    ~ContactData(){
        Reset();
    }
}ContactData_t, *ContactData_ptr;

#define SAFE_RELEASE_LPWSTR(s) { \
if((s)) {\
         delete [] (s); \
         (s) = NULL; \
           }\
}

typedef enum SmsSendReceive{
    SMS_RECEIVE	=	0,
    SMS_SEND	=	1,
}SmsSendReceive_t;

//used for sms display list
typedef struct SmsSimpleData{
    LPWSTR ContactName;
    LPWSTR MobileNumber;
    BOOL	SendReceiveFlag;
    LPWSTR	TimeStamp;
    LPWSTR	Content;
    BOOL	SelectionFlag;
    void Reset(){
        SAFE_RELEASE_LPWSTR(ContactName);
        SAFE_RELEASE_LPWSTR(MobileNumber);
        SAFE_RELEASE_LPWSTR(Content);
        SAFE_RELEASE_LPWSTR(TimeStamp);
        SelectionFlag = false;
        SendReceiveFlag = false;
    }
    SmsSimpleData(){
        ContactName = NULL;
        MobileNumber = NULL;
        SendReceiveFlag = false;
        Content = NULL;
        TimeStamp = NULL;
        SelectionFlag = false;
    }
    ~SmsSimpleData(){
        Reset();
    }
}SmsSimpleData_t, *SmsSimpleData_ptr;

typedef struct SmsViewListKey{
    LPWSTR key;			//列表醒目名称
    UINT	nReceive;	//发送条数
    UINT	nSend;		//接收条数
    SmsViewListKey(){
        key = NULL;
        nReceive = 0;
        nSend = 0;
    }
    ~SmsViewListKey(){
        Reset();
    }
    void Reset(){
        SAFE_RELEASE_LPWSTR(key);
        nReceive = 0;
        nSend = 0;
    }
}SmsViewListKey_t,*SmsViewListKey_ptr;

class LocalDataBase {
public:
    LocalDataBase();
    ~LocalDataBase();
public:
    bool connect();
    bool disconnect() { return disconnectDatabase(); }
    bool decrypt(wchar_t* pwd,int len);
    bool encrypt(wchar_t* pwd,int len);
    //check if pwd is correct, db will disconnected automatically after this operation
    bool checkpwd(wchar_t* pwd,int len);
public:
    //sms相关操作
    //获取短信数量
    bool GetSmsCount(UINT &received, UINT &sent);
    //获取短信条目
    bool GetSms(UINT idx,SmsSimpleData_ptr);
public:
    bool CreateTempSmsTable();	//建立内联表格
private:
    bool bTempTableCreated;
private:
    //sqlite operations
    sqlite3* db;
    sqlite3_stmt* pStmt;
    const void* pzTail;
    wchar_t sqlcmdw[512];
    wchar_t db_path[MAX_PATH];
    bool bconnected;
protected:
    bool connectDatabase(const wchar_t*);
    bool disconnectDatabase();
    void createDefaultDatabase();
    bool decrytpDatabase(const char* pwd,int len);	//true: successful
    bool setDatabasePassword(const char* pwd,int len);	//true: successful
public:
    int beginTrans(){
        bool rc = false;
        C::wsprintf(sqlcmdw,L"begin transaction;");
        if (sqlite3_prepare16(db, sqlcmdw, -1, &pStmt, &pzTail) == SQLITE_OK) {
            sqlite3_step(pStmt);
        }
        sqlite3_finalize(pStmt);
        return rc;
    }
    int commitTrans(){
        bool rc = false;
        C::wsprintf(sqlcmdw,L"commit transaction;");
        if (sqlite3_prepare16(db, sqlcmdw, -1, &pStmt, &pzTail) == SQLITE_OK) {
            sqlite3_step(pStmt);
        }
        sqlite3_finalize(pStmt);
        return rc;
    }
    bool reorgDatebase(){
        bool rc = false;
        C::wsprintf(sqlcmdw,L"VACUUM");
        if (sqlite3_prepare16(db, sqlcmdw, -1, &pStmt, &pzTail) == SQLITE_OK) {
            sqlite3_step(pStmt);
        }
        sqlite3_finalize(pStmt);
        return rc;
    }
};

