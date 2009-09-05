#include "LocalDataBase.h"

LocalDataBase::LocalDataBase() {
    wchar_t currpath[MAX_PATH];
    if(File::GetCurrentPath(currpath)){
        C::wsprintf(db_path,L"%s\\sms.db",currpath);
    }else{
        C::wsprintf(db_path,DEFAULT_DB);
    }
    bTempTableCreated = false;
    bconnected = false;
}

LocalDataBase::~LocalDataBase() {
    disconnectDatabase();
}

bool LocalDataBase::connect(){
    if(bconnected) return bconnected;

    bconnected = connectDatabase(db_path);
    return bconnected;
}
bool LocalDataBase::checkpwd(wchar_t* pwd,int len){
    bool nRet = false;
    if(!bconnected) connect();
    if(!bconnected) return nRet;

    if(pwd && len != 0){
        decrypt(pwd,len);
    }
    C::wsprintf(sqlcmdw,L"select count(*) from sqlite_master");
    int rc;
    if ((rc = sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail)) == SQLITE_OK) {
        nRet = (sqlite3_step(pStmt) == SQLITE_ROW);
    }
    sqlite3_finalize(pStmt);
    if(!nRet){
        disconnect();
    }
    return nRet;
}
bool LocalDataBase::decrypt(wchar_t* pwd, int len){
    char* temp = new char[len*2+1];
    int bytecnt = 0;
    wchar_t *p = pwd;
    char* b_pwd = temp;
    for(int i = 0; i < len; i++){
        wchar_t w = *p++;
        if(w&0xff){
            *b_pwd++ = w&0xff;
            bytecnt++;
        }
        if((w>>8)&0xff){
            *b_pwd++ = (w>>8)&0xff;
            bytecnt++;
        }
    }
    *b_pwd = '\0';
    bool ret = decrytpDatabase(temp,bytecnt);
    delete temp;
    return ret;
}

bool LocalDataBase::encrypt(wchar_t* pwd, int len){
    char* temp = new char[len*2+1];
    int bytecnt = 0;
    wchar_t *p = pwd;
    char* b_pwd = temp;
    for(int i = 0; i < len; i++){
        wchar_t w = *p++;
        if(w&0xff){
            *b_pwd++ = w&0xff;
            bytecnt++;
        }
        if((w>>8)&0xff){
            *b_pwd++ = (w>>8)&0xff;
            bytecnt++;
        }
    }
    *b_pwd = '\0';
    bool ret =  setDatabasePassword(temp,bytecnt);
    delete temp;
    return ret;
}


bool LocalDataBase::connectDatabase(const wchar_t * dbfile) {
    const wchar_t* f = dbfile;
    if (f == NULL) {
        f = DEFAULT_DB;
    }
    return (sqlite3_open16(f, &db) == SQLITE_OK);
}

bool LocalDataBase::disconnectDatabase() {
    bconnected = !(sqlite3_close(db) == SQLITE_OK);
    return !bconnected;
}

void LocalDataBase::createDefaultDatabase() {
    //what ever create db
    //UNIQUE solved duplication problem
    //create account table
    C::wsprintf(sqlcmdw, CREATE_CONTACT_TBL, TABLE_CONTACT);

    if (sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail) == SQLITE_OK) {
        sqlite3_step(pStmt);
    }
    sqlite3_finalize(pStmt);

    C::wsprintf(sqlcmdw, CREATE_SMS_TBL, TABLE_SMS);
    if (sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail) == SQLITE_OK) {
        sqlite3_step(pStmt);
    }
    sqlite3_finalize(pStmt);
    return;
}

bool LocalDataBase::decrytpDatabase(const char* pwd,int len){
    return (sqlite3_key(db,pwd,len) == SQLITE_OK);
}

bool LocalDataBase::setDatabasePassword(const char* pwd,int len){
    return (sqlite3_rekey(db,pwd,len) == SQLITE_OK);
}

//////////////////////////////////////
//sms操作
bool LocalDataBase::GetSmsCount_v1(UINT &received, UINT &sent){
    bool nRet = false;
    UINT total = 0;
    received = 0;
    sent = 0;

    C::wsprintf(sqlcmdw,L"select count(*) from %s",TABLE_SMS_OLD);
    if (sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail) == SQLITE_OK) {
        if (sqlite3_step(pStmt) == SQLITE_ROW){
            total = sqlite3_column_int(pStmt, 0);
            nRet = true;
        }
    }
    sqlite3_finalize(pStmt);

    if(total != 0){
        C::wsprintf(sqlcmdw,L"select sum(SendReceive) from %s",
            TABLE_SMS);
        if (sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail) == SQLITE_OK) {
            if (sqlite3_step(pStmt) == SQLITE_ROW){
                sent = sqlite3_column_int(pStmt, 0);
            }
        }
        sqlite3_finalize(pStmt);
        received = total - sent;
    }
    return nRet;
}

bool LocalDataBase::GetSmsCount(UINT &received, UINT &sent){
    bool nRet = false;
    UINT total = 0;
    received = 0;
    sent = 0;

    C::wsprintf(sqlcmdw,L"select count(*) from %s",TABLE_SMS);
    if (sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail) == SQLITE_OK) {
        if (sqlite3_step(pStmt) == SQLITE_ROW){
            total = sqlite3_column_int(pStmt, 0);
            nRet = true;
        }
    }
    sqlite3_finalize(pStmt);

    if(total != 0){
        C::wsprintf(sqlcmdw,L"select sum(SendReceive) from %s",
            TABLE_SMS);
        if (sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail) == SQLITE_OK) {
            if (sqlite3_step(pStmt) == SQLITE_ROW){
                sent = sqlite3_column_int(pStmt, 0);
            }
        }
        sqlite3_finalize(pStmt);
        received = total - sent;
    }
    return nRet;
}

////////////////////////////////////////////////////////////////////
bool LocalDataBase::CreateTempSmsTable(){
    if(bTempTableCreated) return bTempTableCreated;

    C::wsprintf(sqlcmdw,L"create temp table if not exists '%s' "
        L"(name text,telnumber text, content text,timestamps datetime,sendreceive numeric, year text, month text, day text)",TABLE_TEMP);
    if (sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail) == SQLITE_OK) {
        if (sqlite3_step(pStmt) == SQLITE_DONE){
            bTempTableCreated = true;
        }
    }
    sqlite3_finalize(pStmt);
    if(!bTempTableCreated) return bTempTableCreated;

    C::wsprintf(sqlcmdw,L"insert into '%s' (name,telnumber,content,timestamps,sendreceive,year,month,day) "
        L"select contacts_v1.Name, sms_v1.PhoneNumber, sms_v1.content,sms_v1.timestamps,sms_v1.sendreceive,"
        L"strftime('%%Y',sms_v1.timestamps),strftime('%%m',sms_v1.timestamps),strftime('%%d',sms_v1.timestamps)"
        L"from contacts_v1,sms_v1 where (contacts_v1.PhoneNumber =  sms_v1.PN)",TABLE_TEMP);
    if (sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail) == SQLITE_OK) {
        sqlite3_step(pStmt);
    }
    sqlite3_finalize(pStmt);

    C::wsprintf(sqlcmdw,L"insert into '%s' (name,telnumber,content,timestamps,sendreceive,year,month,day) "
        L"select PN as name,PhoneNumber, content,timestamps,sendreceive,"
        L"strftime('%%Y',timestamps),strftime('%%m',timestamps),strftime('%%d',timestamps)"
        L"from sms_v1 where PN not in (select PhoneNumber from contacts_v1)",TABLE_TEMP);
    if (sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail) == SQLITE_OK) {
        sqlite3_step(pStmt);
    }
    sqlite3_finalize(pStmt);
    return bTempTableCreated;
}

bool LocalDataBase::GetSms(UINT idx,SmsSimpleData_ptr psms){
    bool nRet = false;

    if(psms == NULL) return nRet;

    C::wsprintf(sqlcmdw,L"select name,phonenumber,content,strftime('%%Y-%%m-%%d %%H:%%M:%%S',timestamps),sendreceive from '%s' "
                    L"where rowid=%d",
                    TABLE_SMS,idx+1);
    int nRc = 0;
    if ((nRc = sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail)) == SQLITE_OK) {
        if (sqlite3_step(pStmt) == SQLITE_ROW){
            C::newstrcpy(&psms->ContactName,(LPWSTR) sqlite3_column_text16(pStmt, 0));
            C::newstrcpy(&psms->MobileNumber,(LPWSTR) sqlite3_column_text16(pStmt, 1));
            C::newstrcpy(&psms->Content,(LPWSTR) sqlite3_column_text16(pStmt, 2));
            C::newstrcpy(&psms->TimeStamp,(LPWSTR) sqlite3_column_text16(pStmt, 3));
            psms->SendReceiveFlag = (BOOL)sqlite3_column_int(pStmt, 4);
            nRet = true;
        }
    }
    return nRet;
}

bool LocalDataBase::GetSms_v1(UINT idx,SmsSimpleData_ptr psms){
    bool nRet = false;

    if(psms == NULL) return nRet;

    if(!bTempTableCreated){
        if(!CreateTempSmsTable()){  //创建不成功
            return nRet;
        }
    }

    C::wsprintf(sqlcmdw,L"select name,telnumber,content,strftime('%%Y-%%m-%%d %%H:%%M:%%S',timestamps),sendreceive from '%s' "
                    L"where rowid=%d",
                    TABLE_TEMP,idx+1);
    int nRc = 0;
    if ((nRc = sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail)) == SQLITE_OK) {
        if (sqlite3_step(pStmt) == SQLITE_ROW){
            C::newstrcpy(&psms->ContactName,(LPWSTR) sqlite3_column_text16(pStmt, 0));
            C::newstrcpy(&psms->MobileNumber,(LPWSTR) sqlite3_column_text16(pStmt, 1));
            C::newstrcpy(&psms->Content,(LPWSTR) sqlite3_column_text16(pStmt, 2));
            C::newstrcpy(&psms->TimeStamp,(LPWSTR) sqlite3_column_text16(pStmt, 3));
            psms->SendReceiveFlag = (BOOL)sqlite3_column_int(pStmt, 4);
            nRet = true;
        }
    }
    return nRet;
}

bool LocalDataBase::oldTableExists(){
    bool nRet = false;
    bool c = false;	//如果为连接数据库，则打开数据后执行关闭，恢复初始状态
    if(!bconnected) {
        connect();
        c = true;
    }
    if(!bconnected) return nRet;

    C::wsprintf(sqlcmdw,L"select count(*) from sqlite_master where type='table' and name = '%s'",TABLE_SMS_OLD);
    if (sqlite3_prepare16(db,sqlcmdw,-1,&pStmt,&pzTail) == SQLITE_OK) {
        if(sqlite3_step(pStmt) == SQLITE_ROW){
            nRet = (sqlite3_column_int(pStmt, 0) != 0);
        }
    }
    sqlite3_finalize(pStmt);
    if(c){
        disconnect();
    }
    return nRet;
}
