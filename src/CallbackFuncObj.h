
//callbackfuncobj.h
#ifndef __CALLBACK_FUNCTION_BOJ_H__
#define __CALLBACK_FUNCTION_BOJ_H__


#include <stdio.h>
#include <iostream>

template<typename pCallbackFunc>
class TCallbackFuncObj
{
public:
	TCallbackFuncObj();
	TCallbackFuncObj(pCallbackFunc pCF, void* pCParam);
	~TCallbackFuncObj();
	
	inline void Set(pCallbackFunc pCF, void* pCParam);
	inline pCallbackFunc GetCallbackFunc();
	inline void* GetCallbackParam();

private:
	TCallbackFuncObj(const TCallbackFuncObj& crs);
	TCallbackFuncObj& operator=(const TCallbackFuncObj& crs );
	
private:
	pCallbackFunc	m_pCF;
	void* 			m_pCParam;
};





#endif   //__CALLBACK_FUNCTION_BOJ_H__
