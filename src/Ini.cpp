#include "Ini.h"


CIniFile::CIniFile( )
{

}

CIniFile::~CIniFile( )
{
	c_inimap.clear();
	vt_ini.clear();
}

bool CIniFile::open(const char* pinipath)
{
	return do_open(pinipath);
}

string CIniFile::read(const char*psect, const char*pkey)
{
	string mapkey = psect;
	mapkey += MIDDLESTRING;
	mapkey += pkey;
	strMapIt it = c_inimap.find(mapkey);
	if(it == c_inimap.end())
		return "";
	else
		return it->second;
}

bool CIniFile::change_rootkey(char *sItself,char *sNewItself)
{
	if(!strcmp(sItself,"") || !strcmp(sNewItself,""))
	{
		return false;
	}
	string sIt = "[";
	sIt+=sItself;
	sIt+="]"; 
	for(std::vector<string>::iterator iter = vt_ini.begin();
		iter != vt_ini.end();iter++)
	{
		if(!(*iter).compare(sIt))
		{
			//** �ı��ļ�vector
			sIt = "[";
			sIt+=sNewItself;
			sIt+="]"; 
			*iter = sIt;
			//** �ı�map
			c_inimap.clear();
			for_each(vt_ini.begin(), vt_ini.end(), analyzeini(c_inimap));
			return true;
		}
	}
	return false;
}

bool CIniFile::change_subkey(char *sRootkey, char *sItSubkey, char *sNewSubkey)
{
	if(!strcmp(sRootkey,"") || !strcmp(sItSubkey,"") || !strcmp(sNewSubkey,""))
	{
		return false;
	}
	string sRoot = "[";
	sRoot+=sRootkey;
	sRoot+="]"; 
	int i = 0;
	for(std::vector<string>::iterator iter = vt_ini.begin();
		iter != vt_ini.end();iter++)
	{
		if(!(*iter).compare(sRoot))
		{
			//** �ı��ļ�vector
			i=1;
			continue;
		}
		if(i)
		{
			if(0 == (*iter).find("["))
			{
				/* û�ҵ�,�Ѿ�����һ�� root */
				return false;
			}
			if(0 == (*iter).find(sItSubkey))
			{
				/* �Ѿ��ҵ� */
				string save = (*iter).substr(strlen(sItSubkey),(*iter).length());
				*iter  = sNewSubkey;
				*iter +=save;
				//** �ı�map
				c_inimap.clear();
				for_each(vt_ini.begin(), vt_ini.end(), analyzeini(c_inimap));
				return true;
			}
		}
	}
	return false;
}

bool CIniFile::change_keyvalue(char *sRootkey, char *sSubkey, char *sValue)
{
	if(!strcmp(sRootkey,"") || !strcmp(sSubkey,""))
	{
		return false;
	}
	string sRoot = "[";
	sRoot+=sRootkey;
	sRoot+="]"; 
	int i = 0;
	for(std::vector<string>::iterator iter = vt_ini.begin();
		iter != vt_ini.end();iter++)
	{
		if(!(*iter).compare(sRoot))
		{
			//** �ı��ļ�vector
			i=1;
			continue;
		}
		if(i)
		{
			if(0 == (*iter).find("["))
			{
				/* û�ҵ�,�Ѿ�����һ�� root */
				return false;
			}
			if(0 == (*iter).find(sSubkey))
			{
				/* �Ѿ��ҵ� */
				string save = (*iter).substr(0,strlen(sSubkey)+1);
				*iter  = save;
				*iter +=sValue;
				//** �ı�map
				c_inimap.clear();
				for_each(vt_ini.begin(), vt_ini.end(), analyzeini(c_inimap));
				return true;
			}
		}
	}
	return false;
}

bool CIniFile::del_key(char *sRootkey, char *sSubkey)
{
	if(!strcmp(sRootkey,""))
	{
		return false;
	}
	string sRoot = "[";
	sRoot+=sRootkey;
	sRoot+="]"; 
	if(!strcmp(sSubkey,""))
	{
		//** del root key
		int last_boud = 0;
		int pad_size = 0;
		std::vector<string>::iterator at_begin,at_end = vt_ini.end();
		for(std::vector<string>::iterator iter = vt_ini.begin();
			iter != vt_ini.end();iter++)
		{
			if(!(*iter).compare(sRoot))
			{
				//** �ı��ļ�vector
				last_boud = 1;
				pad_size++;
				at_begin = iter;
				continue;
			}
			if(last_boud)
			{
				/* �Ѿ�����һ�� root */
				if(0 == (*iter).find("["))
				{
					at_end = iter;
					break;
				}
				pad_size++;
			}

		}
		if(!last_boud)
		{
			return false;/* û�ҵ�*/
		}
		/* �滻 */
		// if(at_end !=(std::vector<string>::iterator )0)
		if(at_end !=vt_ini.end())
		{
			for(std::vector<string>::iterator pIter = at_begin;
				at_end != vt_ini.end();pIter++,at_end++)
			{
				*pIter = *at_end;
			}
		}
		while(pad_size)
		{
			vt_ini.pop_back();
			pad_size--;
		}
	}else
		//** del sub key
	{
		int last_boud = 0;
		for(std::vector<string>::iterator iter = vt_ini.begin();
			iter != vt_ini.end();iter++)
		{
			if(!(*iter).compare(sRoot))
			{
				//** �ı��ļ�vector
				last_boud = 1;
				continue;
			}
			if(last_boud)
			{
				if(0 == (*iter).find("["))
				{
					/* û�ҵ�,�Ѿ�����һ�� root */
					return false;
				}
				if(0 == (*iter).find(sSubkey))
				{
					/* �Ѿ��ҵ� */
					if((iter+1) == vt_ini.end())
					{
						vt_ini.pop_back();
						break;
					}
					for(std::vector<string>::iterator it = iter+1;
						it != vt_ini.end();it++)
					{
						/* del one ��ǰ��λ*/
						*(it-1) = *it;
					}
					vt_ini.pop_back();
					break;
				}

			}
		}
		if(!last_boud)
			return false;
	}
	//** �ı�map
	c_inimap.clear();
	for_each(vt_ini.begin(), vt_ini.end(), analyzeini(c_inimap));
	return true;
}

bool CIniFile::add_key(char *sRootkey, char *sSubkey, char *sKeyValue)
{
	if(!strcmp(sRootkey,""))
	{
		return false;
	}
	string sRoot = "[";
	sRoot+=sRootkey;
	sRoot+="]"; 
	bool isEnd = false;
	for(std::vector<string>::iterator iter = vt_ini.begin();
		iter != vt_ini.end();iter++)
	{
		if(!(*iter).compare(sRoot))
		{
			iter++;
			//** �ı��ļ�vector
			if(!strcmp(sSubkey,""))
			{
				return true;
			}else
			{
				if(!strcmp(sKeyValue,""))
				{
					return false;
				}
				//** �����Ҵμ�
				while(iter != vt_ini.end())
				{
					if(0 == (*iter).find("["))
					{
						/* û�ҵ�,�Ѿ�����һ�� root 
						���
						*/
						string push_node = *(vt_ini.end()-1);
						string s_node =*iter;
						std::vector<string>::iterator p_back ;
						for(p_back = vt_ini.end()-1;
							p_back != iter;p_back--)
						{
							*p_back  = *(p_back-1);
						}
						*p_back = sSubkey;
						*p_back += "=";
						*p_back += sKeyValue;
						vt_ini.push_back(push_node);
						//** �ı�map
						c_inimap.clear();
						for_each(vt_ini.begin(), vt_ini.end(), analyzeini(c_inimap));
						return true;

					}else
					{
						/*
						�ҵ����滻
						*/
						if(0 == (*iter).find(sSubkey))
						{
							string save = (*iter).substr(0,strlen(sSubkey)+1);
							*iter  = save;
							*iter +=sKeyValue;

							//** �ı�map
							c_inimap.clear();
							for_each(vt_ini.begin(), vt_ini.end(), analyzeini(c_inimap));
							return true;
						}
					}
					iter++;
				}
				/* û�ҵ� */      
				isEnd = true;
				break;
			}
		}
	}
	/* û�ҵ����� */
	if(strcmp(sSubkey,"") && strcmp(sKeyValue,""))
	{
		if(!isEnd)
		{
			vt_ini.push_back(sRoot);
		}
		string st = sSubkey;
		st += "=";
		st += sKeyValue;
		vt_ini.push_back(st);

	}else if(!strcmp(sSubkey,"") && !strcmp(sKeyValue,""))
	{     
		vt_ini.push_back(sRoot);
	}else
	{
		return false;
	}
	//** �ı�map
	c_inimap.clear();
	for_each(vt_ini.begin(), vt_ini.end(), analyzeini(c_inimap));
	return true;
}

bool CIniFile::flush()
{
	ofstream out(s_file_path.c_str());
	if(!out.is_open())
		return false;
	copy(vt_ini.begin(),vt_ini.end()-1,ostream_iterator<string>(out,"\n"));
	copy(vt_ini.end()-1,vt_ini.end(),ostream_iterator<string>(out,""));
	out.close();
	return true;
}

bool CIniFile::do_open(const char* pinipath)
{
	s_file_path = pinipath;
	ifstream fin(pinipath);
	if(!fin.is_open())
		return false;

	vt_ini.clear();
	while(!fin.eof())
	{
		string inbuf;
		getline(fin, inbuf,'\n');
		if(inbuf.empty())
			continue;
		vt_ini.push_back(inbuf);
	}
	fin.close();
	if(vt_ini.empty())
		return true;

	c_inimap.clear();
	for_each(vt_ini.begin(), vt_ini.end(), analyzeini(c_inimap));
	return true;        
}

