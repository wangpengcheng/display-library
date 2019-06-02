// Adl2_Sample.cpp : Defines the entry point for the console application.
//

///
///  Copyright (c) 2012 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file Adl2_Sample.cpp
/// \brief Sample  application that demonstrates usage of ADL2 APIs in the application that contains multiple uncoordinated ADL transactions.
///
/// ADL2 APIs have been added to ADL to allow creation of the applications that contain multiple clients that call ADL2 APIs in uncoordinated manner without risking 
/// to destroy each other�s context. You can use legacy ADL and newer ADL2 APIs in the same application. The only limitation is that no more than one client that uses 
/// ADL APIs can exist at any given time in the same process. There is no limit to number of clients that use ADL2 APIs. This sample demonstrates how to author application 
/// that contains multiple ADL2 transactions and single legacy ADL transaction. 
/// Author: Ilia Blank

#if defined (LINUX)
#include "../../include/adl_sdk.h"
#include "../../include/customer/oem_structures.h"
#include <dlfcn.h>	//dyopen, dlsym, dlclose
#include <stdlib.h>	
#include <string.h>	//memeset
#include <unistd.h>	//sleep

#else
//windows 相关头文件
#include <windows.h>
#include <tchar.h>
//adl 相关二进制头文件对齐
#include "..\..\include\adl_sdk.h"
#endif
// 一般的io接口
#include <stdio.h>

// Definitions of the used function pointers. Add more if you use other ADL APIs. Note that that sample will use mixture of legacy ADL and ADL2 APIs.
//预先确定需要使用的函数指针，方便在dll中加载到函数。

typedef int (*ADL_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int );//定义ADL主控制创建函数
typedef int (*ADL_MAIN_CONTROL_DESTROY )();//定义ADL销毁函数
typedef int (*ADL_ADAPTER_NUMBEROFADAPTERS_GET ) ( int* );//获取容器指针获得
typedef int (*ADL2_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int, ADL_CONTEXT_HANDLE*);//定义ADL2控制创建函数
typedef int (*ADL2_MAIN_CONTROL_DESTROY )( ADL_CONTEXT_HANDLE);//ADL2上下文销毁函数
typedef int (*ADL2_ADAPTER_ACTIVE_GET ) (ADL_CONTEXT_HANDLE, int, int* );//ADL2容器激活获得函数
typedef int (*ADL2_DISPLAY_MODES_GET )(ADL_CONTEXT_HANDLE, int iAdapterIndex, int iDisplayIndex, int* lpNumModes, ADLMode** lppModes);//ADL2容器获得函数

//利用函数指针初始化函数
ADL_MAIN_CONTROL_CREATE				ADL_Main_Control_Create;//主控生成 
ADL_MAIN_CONTROL_DESTROY			ADL_Main_Control_Destroy;//主控销毁
ADL_ADAPTER_NUMBEROFADAPTERS_GET	ADL_Adapter_NumberOfAdapters_Get;//主要容器获得
ADL2_MAIN_CONTROL_CREATE			ADL2_Main_Control_Create;//ADL2主控创建
ADL2_MAIN_CONTROL_DESTROY			ADL2_Main_Control_Destroy;//ADL2主控销毁
ADL2_DISPLAY_MODES_GET				ADL2_Display_Modes_Get;//ADL2展示模型
ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get;//ADL2容器激活

// Memory allocation function //定义分配内存函数
void* __stdcall ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

// Optional Memory de-allocation function //定义内存释放函数
void __stdcall ADL_Main_Memory_Free ( void* lpBuffer )
{
    if ( NULL != lpBuffer )
    {
        free ( lpBuffer );
        lpBuffer = NULL;
    }
}

#if defined (LINUX)
// equivalent functions in linux
void * GetProcAddress( void * pLibrary, const char * name)
{
    return dlsym( pLibrary, name);
}

#endif
//通过dll或者so文件初始化ADL
int InitADL ()
{
#if defined (LINUX)
    void *hDLL;		// Handle to .so library
#else
    HINSTANCE hDLL;		// Handle to DLL  //https://blog.csdn.net/liu_yude/article/details/45949933
#endif

	#if defined (LINUX)
    hDLL = dlopen( "libatiadlxx.so", RTLD_LAZY|RTLD_GLOBAL);
#else
    //加载dll并使用
    hDLL = LoadLibrary("atiadlxx.dll");
    if (hDLL == NULL)
        // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        hDLL = LoadLibrary("atiadlxy.dll");
#endif

    //dll加载失败
	if (NULL == hDLL)
    {
        printf("ADL library not found!\n");
		return ADL_ERR;
    }
    //通过GetProcAddress函数找到，函数对应地址并且初始化，再强制类型转换获得对应函数
	ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE) GetProcAddress(hDLL,"ADL_Main_Control_Create");
    ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL_Main_Control_Destroy");
    ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET) GetProcAddress(hDLL,"ADL_Adapter_NumberOfAdapters_Get");

	ADL2_Main_Control_Create = (ADL2_MAIN_CONTROL_CREATE) GetProcAddress(hDLL,"ADL2_Main_Control_Create");
    ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL2_Main_Control_Destroy");
    ADL2_Display_Modes_Get = (ADL2_DISPLAY_MODES_GET) GetProcAddress(hDLL,"ADL2_Display_Modes_Get");
	ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");

	if (NULL == ADL_Main_Control_Create ||
        NULL == ADL_Main_Control_Destroy ||
        NULL == ADL_Adapter_NumberOfAdapters_Get ||
		NULL == ADL2_Main_Control_Create ||
        NULL == ADL2_Main_Control_Destroy ||
        NULL == ADL2_Display_Modes_Get ||
		NULL == ADL2_Adapter_Active_Get)
	{
	    printf("ADL's API is missing!\n");
		return ADL_ERR; 
	}

	return ADL_OK;
}


//Retrieves active status of given adapter. Implements the retrieval as isolated ADL2 transaction.
//Note that the function can be nested inside another ADL2 transaction without interfering with it.
//In real application it would be much more efficient to share the same context with the parent transaction by passing the context handle in the function argument list.   
//获取容器激活状态
int GetAdapterActiveStatus (int adapterId, int& active)
{
	//定义上下文空指针
	ADL_CONTEXT_HANDLE context = NULL;	
	//默认没有激活
	active = 0;
    //创建ADL控制器
	if (ADL_OK != ADL2_Main_Control_Create (ADL_Main_Memory_Alloc, 1, &context))
	{
		printf ("Failed to initialize nested ADL2 context");
		return ADL_ERR;
	}

	//获取容器活动状态
	if (ADL_OK != ADL2_Adapter_Active_Get(context, adapterId , &active))
	{
		printf ("Failed to get adapter status");
	}
	//销毁活动
	if (ADL_OK != ADL2_Main_Control_Destroy (context))
	{
		printf ("Failed to destroy nested ADL2 context");
		return ADL_ERR;
	}
	return ADL_OK;
}

//Demonstrates execution of multiple nested ADL2 transactions that are executed on the same thread. 
//Uncoordinated ADL2 transactions can be also executed on separate thread. 
//打印容器信息
int PrintAdapterInfo (int adapterId)
{
	//ADL上下文
	ADL_CONTEXT_HANDLE context = NULL;	
    
    //分配上下文内存
	if (ADL_OK != ADL2_Main_Control_Create (ADL_Main_Memory_Alloc, 1, &context))
	{
		printf ("Failed to initialize ADL2 context");
		return ADL_ERR;
	}

	int active = 0;

	//Invoking additional nested ADL2 based transaction on the same thread to demonstrate that multiple ADL2 transactions can be executed at the same time inside 
	//the process without interfering. Not the most efficient way to work with ADL. In real application it would be much more efficient to re-use  context of parent
	//transaction by passing it to GetAdapterActiveStatus.  
	//在同一个线程上调用其他嵌套的基于ADL2的事务，以证明可以在进程内同时执行多个ADL2事务而不会产生干扰。 不是使用ADL最有效的方法。 在实际应用中，通过将父事务的上下文传递给GetAdapterActiveStatus来更有效。

	//获取容器活动状态
	if (ADL_OK == GetAdapterActiveStatus (adapterId, active))
	{
		printf ("*************************************************\n" );


		printf ("Adapter %d is %s\n", adapterId, (active)?"active":"not active" );
		if (active)
		{
			int numModes;//模型数量			
			ADLMode* adlMode;//定义模型基本信息数据结构

			//获取ADL模型的基本信息
			if (ADL_OK == ADL2_Display_Modes_Get (context, adapterId, -1, &numModes, &adlMode))
			{
				if (numModes == 1)
				{
					printf ("Adapter %d resolution is %d by %d\n", adapterId, adlMode->iXRes, adlMode->iYRes );
					//释放模型内存句柄
					ADL_Main_Memory_Free (adlMode);
				}
			}
		}
	}
	//销毁上下文句柄
	if (ADL_OK != ADL2_Main_Control_Destroy (context))
	{
		printf ("Failed to destroy ADL2 context");
		return ADL_ERR;
	}
	return ADL_OK;
}

int main (int c,char* k[],char* s[])
{
	//初始化ADL，加载动态dll
	if (ADL_OK != InitADL ())
	{
		return 0;
	}

    // Initialize legacy ADL transaction.Note that applications still can mix ADL and ADL2 API providing that only single 
	// transaction that uses legacy ADL APIs exists at any given time in the process. Numer of ADL2 transactions is not limited.  
	// The second parameter is 1, which means:
    // retrieve adapter information only for adapters that are physically present and enabled in the system
    // 
    // 初始化旧版ADL事务。请注意，应用程序仍然可以混合使用ADL和ADL2 API，前提是在此过程中的任何给定时间只存在使用旧版ADL API的单个事务。 ADL2交易的数量不限。 
    // 第二个参数是1，表示：仅检索系统中物理存在和启用的适配器的适配器信息
    // 
    if ( ADL_OK != ADL_Main_Control_Create (ADL_Main_Memory_Alloc, 1) )
	{
	    printf("ADL Initialization Error!\n");
		return 0;
	}

	int  iNumberAdapters;//定义容器数量

    // Obtain the number of adapters for the system
    if ( ADL_OK != ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters ) )
	{
	    printf("Cannot get the number of adapters!\n");
		return 0;
	}
	//输出每一块显示窗口的信息
    for (int adapterId = 0; adapterId < iNumberAdapters; adapterId++ )
    {
		if (ADL_OK != PrintAdapterInfo (adapterId))
			break;
	}

	//销毁主控内存
	//Finishing legacy ADL transaction
	 if ( ADL_OK != ADL_Main_Control_Destroy ())
	 {
		 printf ("Failed to destroy ADL context");
	 }

	return 0;
}
