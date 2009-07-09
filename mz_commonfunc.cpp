#include "mz_commonfunc.h"
using namespace std;



void MZ_CommonC::newstrcpy(wchar_t** pdst,const wchar_t* src){
	if(*pdst) delete *pdst;
	wchar_t* newdst = new wchar_t[lstrlen(src) + 1];
	lstrcpy(newdst,src);
	*pdst = newdst;
}
