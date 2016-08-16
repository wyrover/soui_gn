// residbuilder.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
//
#include <Shlwapi.h>
#include "tinyxml/tinyxml.h"
#include "XGetopt.h"
#include <map>
#include <string>
#include <vector>
#include <tchar.h>

#pragma comment (lib,"Shlwapi.lib")


class PrectectDir
{
public:
	PrectectDir()
	{
		uiroot_path = L"";

		wchar_t szCurrentDir[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, szCurrentDir, sizeof(szCurrentDir));

		std::wstring strExe = szCurrentDir;
		int nPos1 = strExe.rfind(L"\\");
		strExe = strExe.substr(0, nPos1);
		 

		wcscpy(m_szCurrentDir, strExe.c_str());
		wprintf(L"\n PrectectDir Current Dir %s\n", m_szCurrentDir);
	}

	~PrectectDir()
	{
		//wprintf(L"\n PrectectDir Reset Current Dir %s\n", m_szCurrentDir);
		//::SetCurrentDirectoryW(m_szCurrentDir);
	}

	std::wstring getWorkDir()
	{
		return std::wstring(m_szCurrentDir);
	}

	void setUIWorkDir(const std::wstring &path)
	{
		uiroot_path = path;
	}

	std::wstring getUIWorkDir()
	{
		return uiroot_path;
	}
private:
	wchar_t m_szCurrentDir[MAX_PATH + 1];
	std::wstring uiroot_path;
};

PrectectDir obj;

std::string toNarrowString(const wchar_t* pStr, int len, UINT CodePage = CP_ACP)
{
	std::string buf;

	if (pStr == NULL)
	{
		assert(NULL);
		return buf;
	}

	if (len < 0 && len != -1)
	{
		assert(NULL);
		return buf;
	}

	// figure out how many narrow characters we are going to get 
	int nChars = WideCharToMultiByte(CodePage, 0,
		pStr, len, NULL, 0, NULL, NULL);
	if (len == -1)
		--nChars;
	if (nChars == 0)
		return "";

	// convert the wide string to a narrow string
	// nb: slightly naughty to write directly into the string like this
	buf.resize(nChars);
	WideCharToMultiByte(CodePage, 0, pStr, len,
		const_cast<char*>(buf.c_str()), nChars, NULL, NULL);

	return buf;
}


std::wstring toWideString(const char* str, size_t len, unsigned int code_page=CP_ACP)
{
	std::wstring buf;
	int rc_chars = MultiByteToWideChar(code_page,
		0,
		str,
		len,
		NULL,
		0);

	if (len == -1)
		--rc_chars;
	if (rc_chars == 0)
		return buf;

	buf.resize(rc_chars);
	MultiByteToWideChar(code_page,
		0,
		str,
		len,
		const_cast<wchar_t*>(buf.c_str()),
		rc_chars);

	return buf;
} 

const wchar_t  RB_HEADER_RC[]=
L"/*<------------------------------------------------------------------------------------------------->*/\n"\
L"/*¸ÃÎÄ¼þÓÉuiresbuilderÉú³É£¬Çë²»ÒªÊÖ¶¯ÐÞ¸Ä*/\n"\
L"/*<------------------------------------------------------------------------------------------------->*/\n"
L"#define DEFINE_UIRES(name, type, file_path)\\\n"
L"    name type file_path\n\n";

const wchar_t  RB_HEADER_ID[]=
L"/*<------------------------------------------------------------------------------------------------->*/\n"\
L"/*¸ÃÎÄ¼þÓÉuiresbuilderÉú³É£¬Çë²»ÒªÊÖ¶¯ÐÞ¸Ä*/\n"\
L"/*<------------------------------------------------------------------------------------------------->*/\n";

const wchar_t ROBJ_DEF[] =
L"#define ROBJ_IN_CPP \\\n"
L"namespace SOUI \\\n"
L"{\\\n"
L"    const _R R;\\\n"
L"    const _UIRES UIRES;\\\n"
L"}\n";

struct IDMAPRECORD
{
	WCHAR szType[100];
	WCHAR szName[200];
	WCHAR szPath[MAX_PATH];
};

//½âÎöÎª²¼¾ÖµÄÎÄ¼þÀàÐÍ
const wchar_t KXML_LAYOUT[]= L"layout";
const wchar_t KXML_SMENU[]= L"smenu";
const wchar_t KXML_SMENUEX[]= L"smenuex";
//È«¾Ö×ÊÔ´¶¨Òå
const wchar_t KXML_UIDEF[] = L"uidef";

//×Ô¶¯±àºÅ¿ªÊ¼ID
const int KStartID = 0x00010000; 

//»ñµÃÎÄ¼þµÄ×îºóÐÞ¸ÄÊ±¼ä
__int64 GetLastWriteTime(LPCSTR pszFileName)
{
	__int64 tmFile=-1;
	WIN32_FIND_DATAA findFileData;
	HANDLE hFind = FindFirstFileA(pszFileName, &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		tmFile= *(__int64*)&findFileData.ftLastWriteTime;
		FindClose(hFind);
	}
	return tmFile;
}

//»ñµÃÎÄ¼þµÄ×îºóÐÞ¸ÄÊ±¼ä
__int64 GetLastWriteTime(LPCWSTR pszFileName)
{
    __int64 tmFile=0;
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW(pszFileName, &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        tmFile= *(__int64*)&findFileData.ftLastWriteTime;
        FindClose(hFind);
    }
    return tmFile;
}

//½«µ¥·´Ð±¿¸×ª»»³ÉË«·´Ð±¿¸
std::wstring BuildPath(LPCWSTR pszPath)
{
	LPCWSTR p=pszPath;
	WCHAR szBuf[MAX_PATH*2]={0};
	WCHAR *p2=szBuf;
	while(*p)
	{
		if(*p==L'\\')
		{
			if(*(p+1)!=L'\\')
			{//µ¥Ð±¿¸
				p2[0]=p2[1]=L'\\';
				p++;
				p2+=2;
			}else
			{//ÒÑ¾­ÊÇË«Ð±¿¸
				p2[0]=p2[1]=L'\\';
				p+=2;
				p2+=2;
			}
		}else
		{
			*p2=*p;
			p++;
			p2++;
		}
	}
	*p2=0;
	return std::wstring(szBuf);
}

#define STAMP_FORMAT	L"//stamp:0000000000000000\r\n"
#define STAMP_FORMAT2	L"//stamp:%08x%08x\r\n"

#pragma pack(push,1)

class FILEHEAD
{
public:
	//char szBom[2];
	WCHAR szHeadLine[ARRAYSIZE(STAMP_FORMAT)];

	FILEHEAD(__int64 ts=0)
	{
		//szBom[0] = 0xFF;
		//szBom[1] = 0xFE;
		swprintf(szHeadLine,STAMP_FORMAT2,(ULONG)((ts>>32)&0xffffffff),(ULONG)(ts&0xffffffff));		
	}
	static __int64 ExactTimeStamp(LPCTSTR pszFile)
	{
		__int64 ts=0;
		FILE *f = _tfopen(pszFile, _T("rb"));
		if(f)
		{
			FILEHEAD head;
			fread(&head,sizeof(FILEHEAD),1,f);
			DWORD dHi=0,dLow=0;
			if(wcsncmp(head.szHeadLine,STAMP_FORMAT2,8)==0)
			{
				swscanf(head.szHeadLine,STAMP_FORMAT2,&dHi,&dLow);
				ts=((__int64)dHi)<<32|dLow;
			}
			fclose(f);
		}
		return ts;
	}
};
#pragma  pack(pop)

void WriteFile(__int64 tmIdx, const std::string &strRes, const std::wstring &strOut, BOOL bWithHead = FALSE)
{
	//__int64 tmIdx=GetLastWriteTime(strIndexFile.c_str());
	__int64 tmSave=FILEHEAD::ExactTimeStamp(strRes.c_str());
	//write output string to target res file
	if(tmIdx!=tmSave)
	{

		FILE * f=_tfopen(strRes.c_str(),_T("wb"));
		if(f)
		{
			FILEHEAD tmStamp(tmIdx);
			fwrite(&tmStamp,sizeof(FILEHEAD)-sizeof(WCHAR),1,f);//Ð´UTF16ÎÄ¼þÍ·¼°Ê±¼ä¡£-sizeof(WCHAR)ÓÃÀ´È¥³ýstamp×îºóÒ»¸ö\0
			if (bWithHead)
				fwrite(RB_HEADER_RC,sizeof(WCHAR),wcslen(RB_HEADER_RC),f);
			fwrite(strOut.c_str(),sizeof(WCHAR),strOut.length(),f);
			fclose(f);
			printf("build %s succeed!\n",strRes.c_str());
		}
		else
		{
			printf("build %s failed,open file error!\n", strRes.c_str());
		}
	}else
	{
		printf("%s no need to update\n",strRes.c_str());
	}
}

//C++¹Ø¼ü×Ö
wchar_t* szCppKey[] =
{
    L"__asm",
    L"__assume",
    L"__based",
    L"__cdecl",
    L"__declspec",
    L"__event",
    L"__except",
    L"__except",
    L"__fastcall",
    L"__finally",
    L"__finally",
    L"__forceinline",
    L"__if_exists",
    L"__if_not_exists",
    L"__inline",
    L"__int16",
    L"__int32",
    L"__int64",
    L"__int8",
    L"__interface",
    L"__leave",
    L"__m128",
    L"__m128d",
    L"__m128i",
    L"__m64",
    L"__multiple_inheritance",
    L"__noop",
    L"__single_inheritance",
    L"__stdcall",
    L"__super",
    L"__try",
    L"__try",
    L"__uuidof",
    L"__w64",
    L"__wchar_t",
    L"abstract",
    L"array",
    L"bool",
    L"break",
    L"case",
    L"catch",
    L"char",
    L"class",
    L"const",
    L"const_cast",
    L"continue",
    L"default",
    L"delegate",
    L"delete",
    L"dllexport",
    L"dllimport",
    L"do",
    L"double",
    L"dynamic_cast",
    L"else",
    L"enum",
    L"event",
    L"explicit",
    L"extern",
    L"false",
    L"finally",
    L"float",
    L"for",
    L"friend",
    L"friend_as",
    L"goto",
    L"if",
    L"initonly",
    L"inline",
    L"int",
    L"interface",
    L"interior_ptr",
    L"literal",
    L"long",
    L"mutable",
    L"namespace",
    L"new",
    L"nullptr",
    L"operator",
    L"private",
    L"protected",
    L"public",
    L"register",
    L"reinterpret_cast",
    L"return",
    L"safecast",
    L"selectany",
    L"short",
    L"signed",
    L"sizeof",
    L"static",
    L"static_cast",
    L"struct",
    L"switch",
    L"template",
    L"this",
    L"throw",
    L"true",
    L"try",
    L"typedef",
    L"typeid",
    L"typename",
    L"union",
    L"unsigned",
    L"virtual",
    L"void",
    L"volatile",
    L"wchar_t",
    L"while"
};

//ÐèÒª×ªÒåµÄÎÞÐ§×Ö·û,ÒÔÏÂ·ûºÅ¶¼×Ô¶¯×ª»»³É"_"
const wchar_t g_CharEscape[] =L".+-*/~\'\"^%#!@?;|{[]}=";
void EscapeChar(wchar_t &c)
{
    const wchar_t *p = g_CharEscape;
    while(*p)
    {
        if( *p == c)
        {
            c = L'_';
            break;
        }
        p++;
    }
}

int wcscmp2(const void * p1,const void* p2)
{
    const wchar_t *psz1 = (const wchar_t *) p1;
    const wchar_t *psz2 = *(const wchar_t **) p2;
    return wcscmp(psz1,psz2);
}

void MakeNameValid(const wchar_t * pszName,wchar_t * pszOut)
{
    const wchar_t * p1 = pszName;
    wchar_t * p2 = pszOut;
    
    //Êý×Ö¿ªÍ·£¬Ç°Ãæ¼ÓÉÏnameÇ°×º
    if(*p1 >= L'0' && *p1 <= L'9')
    {
        wcscpy(p2,L"name_");
        p2+=5;
    }
    
    //×ªÒåpszNameÖÐµÄ²»ÄÜÓÃÓÚnameµÄ×Ö·û
    while(*p1)
    {
        *p2 = *p1;
        EscapeChar(*p2);
        ++p1;
        ++p2;
    }
    
    *p2=0;
    
    //·ÀÖ¹nameÊÇC++¹Ø¼ü×Ö
    void *pFind = bsearch(pszOut,szCppKey,ARRAYSIZE(szCppKey),sizeof(wchar_t*),wcscmp2);
    if(pFind)
    {
        wcscpy(p2,L"_cpp");
    }
}

void MakeNameValid(const char * pszName,wchar_t * pszOut)
{
    wchar_t szNameW[300];
    MultiByteToWideChar(CP_UTF8,0,pszName,-1,szNameW,300);
    MakeNameValid(szNameW,pszOut);
}

void ParseLayout(TiXmlElement *xmlNode,std::map<std::wstring,int> &vecName2ID,int & nStartId)
{
    if(!xmlNode) return;
    
    const char * pszAttrName = xmlNode->Attribute("name");
    if(pszAttrName)
    {//ÓÐnameÊôÐÔ²Å½âÎöid
        wchar_t szName[100];
        int nID = nStartId;
        MultiByteToWideChar(CP_ACP,0,pszAttrName,-1,szName,100);
        
        if(vecName2ID.find(szName) == vecName2ID.end())
        {
            const char *pszID = xmlNode->Attribute("id");
            if(!pszID)
            {
                nStartId++;
            }else if(strnicmp(pszID,"ID",2) == 0)
            {//IDÎªÃüÃûID£¬ÈçIDOK£¬IDCANCEL
                if(stricmp(pszID,"IDOK")==0)
                    nID = IDOK;
                if(stricmp(pszID,"IDCANCEL")==0)
                    nID = IDCANCEL;
                if(stricmp(pszID,"IDABORT")==0)
                    nID = IDABORT;
                if(stricmp(pszID,"IDRETRY")==0)
                    nID = IDRETRY;
                if(stricmp(pszID,"IDIGNORE")==0)
                    nID = IDIGNORE;
                if(stricmp(pszID,"IDYES")==0)
                    nID = IDYES;
                if(stricmp(pszID,"IDNO")==0)
                    nID = IDNO;
                if(stricmp(pszID,"IDCLOSE")==0)
                    nID = IDCLOSE;
                if(stricmp(pszID,"IDHELP")==0)
                    nID = IDHELP;
                if(stricmp(pszID,"IDTRYAGAIN")==0)
                    nID = IDTRYAGAIN;
                if(stricmp(pszID,"IDCONTINUE")==0)
                    nID = IDCONTINUE;
            }else
            {
                nID = atoi(pszID);
            }
            vecName2ID[szName] = nID;
        }
    }
    TiXmlElement *pChild = xmlNode->FirstChildElement();
    while(pChild)
    {
        ParseLayout(pChild,vecName2ID,nStartId);
        pChild=pChild->NextSiblingElement();
    }
}

void ParseLayoutFile(const wchar_t * pszFileName,std::map<std::wstring,int> &mapName2ID,int & nStartId)
{
	std::wstring sFileName = pszFileName;

    TiXmlDocument xmlLayout;
	FILE *f = _wfopen(sFileName.c_str(),L"rb");
    if(!f)
	{
		//::MessageBoxW(NULL,sFileName.c_str(),L"aaa  shit file not found! ",0);
		return;
	}
    
    if(xmlLayout.LoadFile(f))
    {
        TiXmlElement *pXmlNode = xmlLayout.RootElement();
        //±ÜÃâ½âÎöµ½skin½áµã
        if(stricmp(pXmlNode->Value(),"soui") == 0)
            ParseLayout(pXmlNode->FirstChildElement("root"),mapName2ID,nStartId);
        else if(stricmp(pXmlNode->Value(),"include") == 0 
            || stricmp(pXmlNode->Value(),"menu") == 0   //smenu
            || stricmp(pXmlNode->Value(),"menuRoot") == 0 //smenuex
            )
            ParseLayout(pXmlNode,mapName2ID,nStartId);
    }else
    {
        wprintf(L"!!!err: Load Layout XML Failed! file name: %s\n",pszFileName);
    }
    fclose(f);
}

//´ÓUIDefÖÐ½âÎöString,Color Table
__int64 ParseUIDefFile(std::map<std::string, std::string> &mapFiles, const wchar_t * pszFileName, std::map<std::string, int> &mapString, std::map<std::string, int> &mapColor)
{
    TiXmlDocument xmlUidef;
    __int64 tmStamp  = 0;

	//std::wstring sFileName = obj.getUIWorkDir() +L"\\"+ pszFileName;
	//std::wstring sFileName =  pszFileName;

	std::wstring sFileName = obj.getUIWorkDir() + L"\\" + pszFileName;
  
	wprintf(L"xmlUidef.LoadFile %s  \n", sFileName.c_str());


	if(xmlUidef.LoadFile(sFileName.c_str()))
    {
        TiXmlElement *pXmlNode = xmlUidef.RootElement();
        if(stricmp(pXmlNode->Value(),"uidef")==0)
        {
            //½âÎö string table
            TiXmlElement *pXmlString = pXmlNode->FirstChildElement("string");
            if(pXmlString)
            {
                TiXmlDocument docString;
                if(pXmlString->Attribute("src")!=NULL)
                {
					std::map<std::string, std::string>::iterator it = mapFiles.find(pXmlString->Attribute("src"));
                    if(it == mapFiles.end())
                    {
                        printf("error: can't find string table file that specified by src attribute");
                        pXmlString = NULL;
                    }else
                    {
                        tmStamp += GetLastWriteTime(it->second.c_str());
                        docString.LoadFile(it->second.c_str());
                        pXmlString = docString.FirstChildElement("string");
                    }
                }
                if(pXmlString)
                {
                    TiXmlElement *pStrEle = pXmlString->FirstChildElement();
                    while(pStrEle)
                    {
						std::string strName = pStrEle->Value();
                        mapString[strName] = 1;
                        pStrEle = pStrEle->NextSiblingElement();
                    }
                }
            }
            
            //½âÎö color table
            TiXmlElement *pXmlColor = pXmlNode->FirstChildElement("color");
            if(pXmlColor)
            {
                TiXmlDocument docColor;
                if(pXmlColor->Attribute("src")!=NULL)
                {
					std::map<std::string, std::string>::iterator it = mapFiles.find(pXmlColor->Attribute("src"));
                    if(it == mapFiles.end())
                    {
                        printf("error: can't find color table file that specified by src attribute");
                        pXmlColor = NULL;
                    }else
                    {
                        docColor.LoadFile(it->second.c_str());
                        tmStamp += GetLastWriteTime(it->second.c_str());
                        pXmlColor = docColor.FirstChildElement("color");
                    }
                }
                if(pXmlColor)
                {
                    TiXmlElement *pColorEle = pXmlColor->FirstChildElement();
                    while(pColorEle)
                    {
						std::string strName = pColorEle->Value();
                        mapColor[strName] = 1;
                        
                        pColorEle = pColorEle->NextSiblingElement();
                    }
                }
            }
        }
    }
	else
	{
		if (xmlUidef.Error())
		{ 
			std::string sErrorDesc = xmlUidef.ErrorDesc();
			std::wstring sError = toWideString(sErrorDesc.c_str(), sErrorDesc.length());
			
			wprintf(L"ParseUIDefFile failed %s ,%s \n", sFileName.c_str(), sError.c_str());
		}
		else
		{
			wprintf(L"ParseUIDefFile failed %s ,please use xmlspy to check it !\n", sFileName.c_str());
		}
		
	}
    
    return tmStamp;
}
std::string& trim(std::string &s)
{
	if (s.empty())
	{
		return s;
	}

	s.erase(0, s.find_first_not_of(" ")); 
	return s;
}



//uiresbuilder -p uires -i uires\uires.idx -r .\uires\winres.rc2 -h .\uires\resource.h idtable
int _tmain(int argc, _TCHAR* argv[])
{

	std::string strSkinPath;	//Æ¤·ôÂ·¾¶,Ïà¶ÔÓÚ³ÌÐòµÄ.rcÎÄ¼þ
	std::string strIndexFile;
	std::string strWorkingDir; //¹¤×÷Â·¾¶
	std::string strRes;		//rc2ÎÄ¼þÃû
	std::string strHeadFile; // head file
    BOOL bBuildIDMap=FALSE;  //Build ID map
	int c;

	printf("%s\n",GetCommandLineA());
	while ((c = getopt(argc, argv, _T("w:i:r:p:h:"))) != EOF || optarg!=NULL)
	{
		switch (c)
		{
		case 'w':strWorkingDir=optarg; break;
		case 'i':strIndexFile=optarg;break;
		case 'r':strRes=optarg;break;
		case 'p':strSkinPath=optarg;break;
		case 'h':strHeadFile=optarg;break;
        case EOF:
            if(_tcscmp(optarg ,_T("idtable"))==0) bBuildIDMap = TRUE;
            optind ++;
            break;
		}
	}

	if(strIndexFile.empty())
	{
		printf("not specify input file, using -i to define the input file\n");
		printf("usage: uiresbuilder -p uires -i uires\\uires.idx -r .\\uires\\winres.rc2 -h .\\uires\\resource.h idtable\n");
		printf("\tparam -w : define path of working path\n");
        printf("\tparam -i : define uires.idx path\n");
        printf("\tparam -p : define path of uires folder\n");
        printf("\tparam -r : define path of output .rc2 file\n");
        printf("\tparam -h : define path of output resource.h file\n");
        printf("\tparam idtable : define idtable is needed for resource.h. no id table for default.\n");
		return 1;
	}
	 
	printf("\tparam -w %s\n", strWorkingDir.c_str());
	printf("\tparam -i %s\n", strIndexFile.c_str());
	printf("\tparam -p %s\n", strSkinPath.c_str());
	printf("\tparam -r %s\n", strRes.c_str());
	printf("\tparam -h %s\n \n \n ", strHeadFile.c_str());

	strWorkingDir = trim(strWorkingDir);
	strIndexFile = trim(strIndexFile);
	strSkinPath = trim(strSkinPath);
	strRes = trim(strRes);
	strHeadFile = trim(strHeadFile);
	 

	printf(" \n strHeadFile: %s \n ", strHeadFile.c_str());

	std::wstring workingDir = toWideString(strWorkingDir.c_str(), strWorkingDir.length()); 
	std::wstring indexName;
 
	std::wstring sDir= obj.getWorkDir() +L"\\" +workingDir;
	::SetCurrentDirectoryW(sDir.c_str());

	obj.setUIWorkDir(sDir);

	indexName = sDir + toWideString(strIndexFile.c_str(), strIndexFile.length());

	//strRes = toNarrowString(obj.getWorkDir().c_str(), obj.getWorkDir().length()) + "\\" + strRes;		//rc2ÎÄ¼þÃû
	//strHeadFile = toNarrowString(obj.getWorkDir().c_str(), obj.getWorkDir().length()) + "\\" + strHeadFile; // head file

	wprintf(L"\n getWorkDir %s \n", obj.getWorkDir().c_str());
	
	wprintf(L"\n UIWorkDir %s \n", obj.getUIWorkDir().c_str());
	  
	printf("\n strIndexFile %s \n", strIndexFile.c_str());

	wprintf(L"\n indexName %s \n", indexName.c_str());

	//´ò¿ªuirex.idxÎÄ¼þ
	TiXmlDocument xmlIndexFile;
	if(!xmlIndexFile.LoadFile(indexName.c_str()))
	{
		if(xmlIndexFile.Error())
		{
			std::string sErrorDesc = xmlIndexFile.ErrorDesc();
			std::wstring sError = toWideString(sErrorDesc.c_str(), sErrorDesc.length());

			wprintf(L"parse input file failed %s ,%s \n", indexName.c_str(), sError.c_str());
		}
		else
		{
			wprintf(L"not error parse input file failed %s", indexName.c_str());
		}
		
		return 1;
	}
    TiXmlElement *xmlResource=xmlIndexFile.FirstChildElement("resource");
    if(!xmlResource)
        {
        printf("invalid ui index file");
        return 2;
        }

	std::vector<IDMAPRECORD> vecIdMapRecord;
	std::map<std::string, std::string>  mapFiles;
	
	std::wstring strFiles = L"\tclass _UIRES{\r\n\t\tpublic:\r\n";
	
	//load xml description of resource to vector
    TiXmlElement *pXmlType=xmlResource->FirstChildElement();
	while(pXmlType)
	{
        WCHAR szType[100],szType2[100];
        const char *pszType=pXmlType->Value();
        MultiByteToWideChar(CP_UTF8,0,pszType,-1,szType,100);
        MakeNameValid(szType,szType2);
        
        std::wstring strClassName = std::wstring(L"_")+szType2;
        std::wstring strFileType = std::wstring(L"\t\tclass ") + strClassName +L"{\r\n\t\t\tpublic:\r\n";
        std::wstring strFileTypeConstructor = std::wstring(L"\t\t\t") + strClassName + L"(){\r\n";
        std::wstring strFileTypeMember;
        
        TiXmlElement *pXmlFile=pXmlType->FirstChildElement();
        while(pXmlFile)
            {
            IDMAPRECORD rec={0};
            wcscpy(rec.szType,szType);
            const char *pszName=pXmlFile->Attribute("name");
            if(pszName)
            {
                wchar_t wszName[200];
                MultiByteToWideChar(CP_UTF8,0,pszName,-1,rec.szName,200);
                MakeNameValid(rec.szName,wszName);
                
                const char *pszPath=pXmlFile->Attribute("path");
				std::string strPath;
                if(pszPath)
                {
                    if(!strSkinPath.empty()){ strPath=strSkinPath+"\\"+pszPath;}
                    else strPath=pszPath;
                    MultiByteToWideChar(CP_UTF8,0,strPath.c_str(),strPath.length(),rec.szPath,MAX_PATH);
                }

                vecIdMapRecord.push_back(rec);
				std::string strKey = std::string(pszType) + ":" + pszName;
                mapFiles[strKey] = strPath;

                strFileTypeConstructor += std::wstring(L"\t\t\t\t") + wszName + L" = _T(\"" + szType2 + L":" + rec.szName + L"\");\r\n";
                strFileTypeMember += std::wstring(L"\t\t\tconst TCHAR * ") + wszName + L";\r\n";
            }
            pXmlFile=pXmlFile->NextSiblingElement();
		}
		
		strFileTypeConstructor += L"\t\t\t}\r\n";
		strFileType += strFileTypeConstructor + strFileTypeMember + L"\t\t}" + szType2 + L";\r\n";
		strFiles += strFileType;
		pXmlType=pXmlType->NextSiblingElement();
	}
    
    strFiles += L"\t};\r\n";
    
	if(strRes.length())
	{//±àÒë×ÊÔ´.rc2ÎÄ¼þ
		//build output string by wide char
		std::wstring strOut;
		std::vector<IDMAPRECORD>::iterator it2 = vecIdMapRecord.begin();
		while(it2!=vecIdMapRecord.end())
		{
			WCHAR szRec[2000];
			std::wstring strPath=BuildPath(it2->szPath);
			swprintf(szRec,L"DEFINE_UIRES(%s,\t%s,\t%\"%s\")\n",it2->szName,it2->szType,strPath.c_str());
			strOut+=szRec;
			it2++;
		}
        __int64 tmIdx=GetLastWriteTime(strIndexFile.c_str());
		WriteFile(tmIdx, strRes, strOut, TRUE);
	}

    //ÊäÈëname,id¶¨Òå,Ö»½âÎö×ÊÔ´ÖÐlayout×ÊÔ´µÄXML×ÊÔ´
	if (!strHeadFile.empty())
	{
		std::map<std::wstring, int> mapNameID;
		std::map<std::string, int> mapString;
		std::map<std::string, int> mapColor;


        __int64 tmResource = bBuildIDMap;
        
        int nStartID = KStartID;
		std::vector<IDMAPRECORD>::iterator it2 = vecIdMapRecord.begin();
        while(it2!=vecIdMapRecord.end())
        {
            if(wcsicmp(it2->szType,KXML_LAYOUT)==0 || wcsicmp(it2->szType,KXML_SMENU) == 0 || wcsicmp(it2->szType,KXML_SMENUEX) == 0)
            {//·¢ÏÖ²¼¾Ö»òÕß²Ëµ¥ÎÄ¼þ
                tmResource += GetLastWriteTime(it2->szPath);
                ParseLayoutFile(it2->szPath,mapNameID,nStartID);
            }else if(wcsicmp(it2->szType,KXML_UIDEF)==0)
            {//ÕÒµ½UIDEF
                tmResource += GetLastWriteTime(it2->szPath);
                tmResource += ParseUIDefFile(mapFiles,it2->szPath,mapString,mapColor);
            }
            it2 ++;
        }

		std::wstring strName, 
		        strId ,     //.id
		        strNamedID; //{name,id}Êý×é
		std::wstring strNameConstrutor;  //.nameµÄ¹¹Ôìº¯Êý
		std::wstring strNameVariables;    //.nameµÄ³ÉÔ±±äÁ¿
		
        strNamedID = L"\tconst SNamedID::NAMEDVALUE namedXmlID[]={\r\n";
        strNameConstrutor = L"_name(){\r\n";
        strId = L"\t\tclass _id{\r\n\t\tpublic:\r\n";
        
		std::map<std::wstring, int>::iterator it = mapNameID.begin();
		
		if(it == mapNameID.end())
		{
		    printf("!!!err: name-id map is empty!");
		}
		int idx = 0;
		while(it!=mapNameID.end())
		{
			WCHAR szName[200],szBuf[2000] = { 0 };
			MakeNameValid(it->first.c_str(),szName);
            
            if(!bBuildIDMap)
            {
                swprintf(szBuf,L"\t\t\t%s = L\"%s\";\r\n",szName, it->first.c_str());
                strNameConstrutor += szBuf;
            }else
            {
                swprintf(szBuf,L"\t\t\t%s = namedXmlID[%d].strName;\r\n",szName, idx);
                strNameConstrutor += szBuf;
            }
			
			swprintf(szBuf,L"\t\t const wchar_t * %s;\r\n",szName);
			strNameVariables += szBuf;
			
			swprintf(szBuf, L"\t\tconst static int %s\t=\t%d;\r\n", szName, it->second);
			strId += szBuf;
			
			swprintf(szBuf,L"\t\t{L\"%s\",%d},\r\n",it->first.c_str(),it->second);
			strNamedID += szBuf;
			it ++;
			idx ++;
		}
		strNameConstrutor += L"\t\t}\r\n";
		strName = L"\t\tclass _name{\r\n\t\tpublic:\r\n\t\t";
		strName += strNameConstrutor;
		strName += strNameVariables;
		strName += L"\t\t}name;\r\n";
		
		strId += L"\t\t}id;\r\n";
		
		if(mapNameID.size()>0)
		{
		    strNamedID = strNamedID.substr(0,strNamedID.size()-3); //È¥³ýÊý×é×îºóÒ»¸ö","
		}
		strNamedID += L"\t\t};\r\n";
		
        std::wstring strString;          //.string
        {
            int idx = 0;
            strString = L"\t\tclass _string{\r\n\t\tpublic:\r\n";
			std::map<std::string, int>::iterator it = mapString.begin();
            while(it != mapString.end())
            {
                WCHAR szName[200],szBuf[2000] = { 0 };
                MakeNameValid(it->first.c_str(),szName);
                swprintf(szBuf, L"\t\tconst static int %s\t=\t%d;\r\n", szName, idx++);
                strString += szBuf;
                it ++;
            }
            strString += L"\t\t}string;\r\n";
        }

        std::wstring strColor;           //.color
        {
            int idx = 0;
            strColor = L"\t\tclass _color{\r\n\t\tpublic:\r\n";
			std::map<std::string, int>::iterator it = mapColor.begin();
            while(it != mapColor.end())
            {
                WCHAR szName[200],szBuf[2000] = { 0 };
                MakeNameValid(it->first.c_str(),szName);
                swprintf(szBuf, L"\t\tconst static int %s\t=\t%d;\r\n", szName, idx++);
                strColor += szBuf;
                it ++;
            }
            strColor += L"\t\t}color;\r\n";
        }

		std::wstring strOut = RB_HEADER_ID;
		strOut += L"#pragma once\r\n#include <res.mgr/snamedvalue.h>\r\n";
		strOut += ROBJ_DEF;
		strOut += L"namespace SOUI\r\n{\r\n";
		strOut += strFiles;
				
		if(bBuildIDMap)
		    strOut += strNamedID;
		
		strOut += L"\tclass _R{\r\n\tpublic:\r\n";
		strOut += strName;
		if(bBuildIDMap)
        {
            strOut += L"\r\n";
            strOut += strId;
        }
        strOut += L"\r\n";
        strOut += strString;

        strOut += L"\r\n";
        strOut += strColor;

        strOut += L"\r\n\t};\r\n\r\n";
        
        strOut += L"#ifdef R_IN_CPP\r\n";
        strOut += L"\t extern const _R R;\r\n";
        strOut += L"\t extern const _UIRES UIRES;\r\n";
        strOut += L"#else\r\n";
        strOut += L"\t extern const __declspec(selectany) _R & R = _R();\r\n";
        strOut += L"\t extern const __declspec(selectany) _UIRES & UIRES = _UIRES();\r\n";
        strOut += L"#endif//R_IN_CPP\r\n";

        strOut += L"}\r\n";

		WriteFile(tmResource, strHeadFile, strOut, FALSE);
	}

	return 0;
}


