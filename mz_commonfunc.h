#ifndef _MZ_COMMON_
#define _MZ_COMMON_

#include <QFileInfo>
#include <QString>
#include <QCoreApplication>

typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef bool BOOL;

typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
typedef wchar_t *LPWSTR;

typedef enum TextEncode{
    ttcAnsi, 
    ttcUnicode, 
    ttcUnicodeBigEndian, 
    ttcUtf8
}TEXTENCODE_t;

static wchar_t const raddec[] = L"0123456789";
static wchar_t const radhex[] = L"0123456789abcdef";
static wchar_t const radHEX[] = L"0123456789ABCDEF";




class MZ_CommonC
{
public:
    static void newstrcpy(wchar_t** pdst,const wchar_t* src);
    static int convert(unsigned int val, int radix, const wchar_t *radchar,
                int width, char fminus, char padzero, wchar_t* buf){
        wchar_t cbuf[12];
        int i, j, cnt;

        i = 0;
        cnt = 0;
        do{
            cbuf[i++] = radchar[val % radix];
            val /= radix;
        }while(val != 0);

        width -= fminus;
        if(fminus && padzero){
            buf[cnt++] = '-';
        }
        for(j = i; j < width; j++){
            buf[cnt++] = padzero ? '0' : ' ';
        }
        if(fminus && !padzero){
            buf[cnt++] = '-';
        }
        while(i > 0){
            buf[cnt++] = cbuf[--i];
        }
        return cnt;
    }
    static int _wsprintf(wchar_t* dest, const wchar_t *fmt, va_list ap){
        int pos = 0, printed_char_cnt = 0, i;
        wchar_t* chptr;
        wchar_t c;
        int width;
        char padzero;
        int val;

        if(dest == NULL) return 0;

        while((c = fmt[pos++]) != '\0'){
            width = 0;
            padzero = 0;

            if(c == '%'){
                c = fmt[pos++];
                if(c == '0'){
                    padzero = 1;
                    c = fmt[pos++];
                }
                while(c >= '0' && c <= '9'){
                    width = width * 10 + c - '0';
                    c = fmt[pos++];
                }

                switch(c){
                case 'c':
                    dest[printed_char_cnt++] = va_arg(ap, int);
                    break;
                case 's':
                    chptr = (wchar_t*) va_arg(ap, unsigned char*);
                    i = 0;
                    while(chptr [i]){
                        dest[printed_char_cnt++] = chptr[i++];
                    }
                    break;
                case 'u':
                    printed_char_cnt += convert((unsigned int) va_arg(ap, unsigned int), 10,
                                                raddec, width, 0, padzero, &dest[printed_char_cnt]);
                    break;
                case 'd':
                    val = (unsigned int) va_arg(ap, unsigned int);
                    if(val >= 0){
                        printed_char_cnt += convert(val, 10,
                                                    raddec, width, 0, padzero, &dest[printed_char_cnt]);
                    }else{
                        printed_char_cnt += convert(-val, 10,
                                                    raddec, width, 1, padzero, &dest[printed_char_cnt]);
                    }
                    break;
                case 'x':
                    printed_char_cnt += convert((unsigned int) va_arg(ap, unsigned int), 16,
                                                radhex, width, 0, padzero, &dest[printed_char_cnt]);
                    break;
                case 'X':
                    printed_char_cnt += convert((unsigned int) va_arg(ap, unsigned int), 16,
                                                radHEX, width, 0, padzero, &dest[printed_char_cnt]);
                    break;
                case 'p':
                    dest[printed_char_cnt++] = '0';
                    dest[printed_char_cnt++] = 'x';
                    printed_char_cnt += convert((unsigned int) va_arg(ap, unsigned int), 16,
                                                radHEX, width, 0, padzero, &dest[printed_char_cnt]);
                    break;
                case '%':
                    dest[printed_char_cnt++] = '%';
                    break;
                default:
                    //console_printf("%%%c: missing format character\n",c);
                    break;
                }
            }else{
                dest[printed_char_cnt++] = c;
            }
        }
        dest[printed_char_cnt++] = 0;
        return printed_char_cnt;
    }
    static int wsprintf(wchar_t *pdst, const wchar_t* format,...){
        va_list   args;
        va_start(args,format);
        int nLen = _wsprintf(pdst,format,args);
        QString s = QString::fromStdWString(pdst);
        va_end(args);
        return nLen;
    }
    static int lstrlen(const wchar_t *str){
        int nRet = 0;
        if(str == NULL) return nRet;
        const wchar_t *p = str;
        while(*p++){
            nRet ++;
        }
        return nRet;
    }
    static wchar_t * lstrcpy(wchar_t *dst, const wchar_t* src){
        wchar_t *nRet = NULL;
        if(dst == NULL || src == NULL) return nRet;
        const wchar_t* psrc = src;
        wchar_t* pdst = dst;
        while(*psrc){
            *pdst++ = *psrc++;
        }
        *pdst = 0;
        return dst;
    }
};

class Mz_File {
public:
    static bool FileExists(wchar_t *pf){
        if(pf == NULL) return false;
        return QFileInfo(QString::fromStdWString(pf)).exists();
    }
    static bool GetCurrentPath(wchar_t *pf){
        if(pf == NULL) return false;
        QString debug_s = QCoreApplication::applicationDirPath();
        int nLen = QCoreApplication::applicationDirPath().toWCharArray(pf);
        pf[nLen] = 0;
        if(nLen > 0){
            return true;
        }else{
            return false;
        }
    }
};

namespace MZ_CommonFunc {
    class C : public MZ_CommonC { };
    class File : public Mz_File {};
}
#endif /*_MZ_COMMON_*/
