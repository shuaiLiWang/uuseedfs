/*
 *  project: 
 *                ͨ��ģ�� ( ��STL���� *.ini �ļ� )
 *                  
 *  description:
 *
 *                  ͨ��CHandle_IniFile�����ini �ļ���ʵ����ӡ�ɾ�����޸�
 *
 *          ( the end of this file have one sample,
 *                    welcom to use... )
 *
 *
 *       file:ZLB_Handle_File.h
 *
 *  author: @ zlb
 *  
 *  time:2005-07-25  
 *
 *
 *  
 --*/
 
 
 
 
#ifndef ZLB_HANDLE_FILE__
#define ZLB_HANDLE_FILE__
 
#include <map>
#include <string>
#include <string.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include <functional>
#include <fstream>
#include <iostream>
#include <iterator>
using namespace std;
 
typedef map<string, string, less<string> > strMap;
typedef strMap::iterator strMapIt;
 
const char*const MIDDLESTRING = "_____***_______";
 
struct analyzeini{
       string strsect;
       strMap *pmap;
       analyzeini(strMap & strmap):pmap(&strmap){}       
       void operator()( const string & strini)
       {
              size_t first =strini.find('[');
              size_t last = strini.rfind(']');
              if( first != string::npos && last != string::npos && first != last+1)
              {
                     strsect = strini.substr(first+1,last-first-1);
                     return ;
              }
              if(strsect.empty())
                     return ;
              if((first=strini.find('='))== string::npos)
                     return ;
              string strtmp1= strini.substr(0,first);
              string strtmp2=strini.substr(first+1, string::npos);
              first= strtmp1.find_first_not_of(" \t");
              last = strtmp1.find_last_not_of(" \t");
              if(first == string::npos || last == string::npos)
                     return ;
              string strkey = strtmp1.substr(first, last-first+1);
              first = strtmp2.find_first_not_of(" \t");
              if(((last = strtmp2.find("\t#", first )) != -1) ||
            ((last = strtmp2.find(" #", first )) != -1) ||
            ((last = strtmp2.find("\t//", first )) != -1)||
            ((last = strtmp2.find(" //", first )) != -1))
              {
            strtmp2 = strtmp2.substr(0, last-first);
              }
              last = strtmp2.find_last_not_of(" \t");
              if(first == string::npos || last == string::npos)
                     return ;
              string value = strtmp2.substr(first, last-first+1);
              string mapkey = strsect + MIDDLESTRING;
              mapkey += strkey;
              (*pmap)[mapkey]=value;
              return ;
       }
};
 
class CIniFile
{
public:
    CIniFile( );
    ~CIniFile( );
    bool open(const char* pinipath);

/* 
 *    ��ȡ��Ӧ��ֵ
 *    description:
 *           psect = ���� 
 *           pkey  = �μ���
 *  return = value
 *
*/
    string read(const char*psect, const char*pkey);
/* 
 *
 *  �޸����� (rootkey)
 *  description:
 *      sItself    = Ҫ�޸ĵ�������
 *           sNewItself = Ŀ��������
 *   return = true(�ɹ�) or false(ʧ��)
 *
*/ 
       bool change_rootkey(char *sItself,char *sNewItself);

/* 
 *    �޸Ĵμ� subkey 
 *    description:
 *      sRootkey    = Ҫ�޸ĵĴμ�������������
 *           sItSubkey   = Ҫ�޸ĵĴμ���
 *           sNewSubkey  = Ŀ��μ���
 *   return = true(�ɹ�) or false(ʧ��)
 *
*/
       bool change_subkey(char *sRootkey, char *sItSubkey, char *sNewSubkey);
/* 
 *    �޸ļ�ֵ 
 *    description:
 *      sRootkey    = Ҫ�޸ĵĴμ�������������
 *           sSubkey     = Ҫ�޸ĵĴμ���
 *           sValue      = �μ���Ӧ��ֵ
 *   return = true(�ɹ�) or false(ʧ��)
 *
*/
       bool change_keyvalue(char *sRootkey, char *sSubkey, char *sValue);
/* 
 *    ɾ����ֵ 
 *    description:
 *      sRootkey    = Ҫɾ����������
 *           sSubkey     = Ҫɾ���Ĵμ��� (���Ϊ�վ�ɾ�������������µ����дμ�)
 *    
 *   return = true(�ɹ�) or false(ʧ��)
 *
*/
       bool del_key(char *sRootkey,
                             char *sSubkey="");
/* 
 *    ���Ӽ�ֵ
 *    description:
 *      1��sRootkey    = Ҫ���ӵ�������
 *           2��sSubkey     = Ҫ���ӵĴμ��� 
 *           3��sKeyValue   = Ҫ���ӵĴμ�ֵ
 *
 *    (** ���Ϊ1 2 ͬʱΪ�վ�ֻ�������������ͬʱ��Ϊ�վ�����ȷ���ļ� **)
 *
 *   return = true(�ɹ�) or false(ʧ��)
 *
*/
       bool add_key(char *sRootkey,
                             char *sSubkey="",
                             char *sKeyValue="");
/* 
 *    save ini file 
 *
 *    �ṩ������ƣ����漰���޸ġ�ɾ�������Ӳ���ʱ��
 *    ��������д���ļ���ֻ�Ǹ�д�ڴ棬�����ôκ���
 *    ʱд���ļ�������
 *
*/
       bool flush();

protected:
    /* �� ini �ļ�*/
       bool do_open(const char* pinipath);

private:
       /* file path */
       string s_file_path;
       /* �����ֵ��Ӧ  key = value */
       strMap    c_inimap;
       /* ���������ļ���Ϣ */
       vector<string> vt_ini;
};
 
#endif
 
 
/*
 *
 * sample:
                 #include "ZLB_Handle_File.h"
 
                    CHandle_IniFile ini;
 
                     //-- �ڱ����½�һ��Test.ini���ļ�
 
                    if(!ini.open(".\\Test.ini"))
                           return -1;
 
                    //+++++++++++++++++++++++++++++++++++++++++
                    ini.add_key("RootKeyName");
                    ini.add_key("RootKeyName","SubKeyName_1","Test_1");
                    //-------------you can do up like down-----
                    //-- ini.add_key("RootKeyName","SubKeyName_1","Test_1");
                    //-----------------------------------------
 
 
            ini.add_key("RootKeyName","SubKeyName_2","Test_2");
                    string strvalue = ini.read("RootKeyName","SubKeyName_1");
                    if(strvalue.empty())
                           return -1;
                    else
                           cout<<"value="<<strvalue<<endl;
                    if(ini.del_key("RootKeyName","SubKeyName_2"))
                    {
                           cout<<"you del one (SubKeyName_2) !!!"<<endl;
                           ini.add_key("RootKeyName","SubKeyName_2","Test_2");
                    }else
                           cout<<" del one failed !!! please check your ini file..."<<endl;
                    if(ini.del_key("RootKeyName"))
                    {
                           cout<<"you del all under the RootKeyName (SubKeyName_1 and SubKeyName_2) !!!"
                                  <<endl;
                            ini.add_key("RootKeyName","SubKeyName_1","Test_1");
                    }else
                           cout<<" del all (RootKeyName) failed !!! please check your ini file..."
                                <<endl;
                     if(ini.change_rootkey("RootKeyName","RootKeyNewName"))
                     {
                            cout<<"you change one rootkey success !!!"
                                  <<endl;
                     }else
                     {
                            cout<<"you change one rootkey failed !!!please check your ini file..."
                                  <<endl;
                     }
                    if(ini.change_subkey("RootKeyNewName","SubKeyName_1","SubKeyNewName_1"))
                     {
                            cout<<"you change one subkey success !!!"
                                  <<endl;
                     }else
                     {
                            cout<<"you change one subkey failed !!!please check your ini file..."
                                  <<endl;
                     }
                     if(ini.change_keyvalue("RootKeyNewName","SubKeyNewName_1","TestNew_1"))
                     {
                            cout<<"you change one keyvalue success !!!"
                                  <<endl;
                     }else
                     {
                            cout<<"you change one keyvalue failed !!!please check your ini file..."
                                  <<endl;
                     }       
                     ini.flush();
--*/

