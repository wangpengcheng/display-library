///
///  Copyright (c) 2008 - 2012 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file eyefinity.cpp

#include <windows.h>
#include "..\..\include\adl_sdk.h"
#include "eyefinity.h"
#include <stdio.h>
//定义全局ADL上下文
ADL_CONTEXT_HANDLE ADLContext_ = NULL;

// Comment out one of the two lines below to allow or supress diagnostic messages
// #define PRINTF
#define PRINTF printf

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int(*ADL2_MAIN_CONTROLX2_CREATE)                               (ADL_MAIN_MALLOC_CALLBACK, int iEnumConnectedAdapter_, ADL_CONTEXT_HANDLE* context_, ADLThreadingModel);
typedef int(*ADL2_MAIN_CONTROL_DESTROY)                                (ADL_CONTEXT_HANDLE);
typedef int(*ADL2_ADAPTER_ADAPTERINFOX3_GET)                           (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* numAdapters, AdapterInfo** lppAdapterInfo);
typedef int(*ADL2_DISPLAY_DISPLAYINFO_GET)                             (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumDisplays, ADLDisplayInfo ** lppInfo, int iForceDetect);
typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_VALID)                          (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLSLSMap slsMap, int iNumDisplayTarget, ADLSLSTarget* lpSLSTarget, int* lpSupportedSLSLayoutImageMode, int* lpReasonForNotSupportSLS, int iOption);

// SLS functions
// 反射函数
typedef int(*ADL2_DISPLAY_SLSMAPINDEX_GET) (ADL_CONTEXT_HANDLE, int, int, ADLDisplayTarget *, int *);
typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_GET) (ADL_CONTEXT_HANDLE, int, int, ADLSLSMap*, int*, ADLSLSTarget**, int*, ADLSLSMode**, int*, ADLBezelTransientMode**, int*, ADLBezelTransientMode**, int*, ADLSLSOffset**, int);

typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_DELETE)                         (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iSLSMapIndex);
typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_CREATE)                         (ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLSLSMap SLSMap, int iNumTarget, ADLSLSTarget* lpSLSTarget, int iBezelModePercent, int *lpSLSMapIndex, int iOption);


typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_REARRANGE) (ADL_CONTEXT_HANDLE, int, int, int, ADLSLSTarget*, ADLSLSMap, int);
typedef int(*ADL2_DISPLAY_SLSMAPCONFIG_SETSTATE) (ADL_CONTEXT_HANDLE, int, int, int);
typedef int(*ADL2_DISPLAY_SLSMAPINDEXLIST_GET) (ADL_CONTEXT_HANDLE, int, int*, int**, int);
typedef int(*ADL2_DISPLAY_MODES_GET) (ADL_CONTEXT_HANDLE, int, int, int*, ADLMode **);
typedef int(*ADL2_DISPLAY_MODES_SET) (ADL_CONTEXT_HANDLE, int, int, int, ADLMode*);
typedef int(*ADL2_DISPLAY_BEZELOFFSET_SET) (ADL_CONTEXT_HANDLE, int, int, int, LPADLSLSOffset, ADLSLSMap, int);

//display map functions
//显示map函数
typedef int(*ADL2_DISPLAY_DISPLAYMAPCONFIG_GET)                        (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumDisplayMap, ADLDisplayMap** lppDisplayMap, int* lpNumDisplayTarget, ADLDisplayTarget** lppDisplayTarget, int iOptions);
typedef int(*ADL2_DISPLAY_DISPLAYMAPCONFIG_SET) (ADL_CONTEXT_HANDLE, int, int, ADLDisplayMap*, int, ADLDisplayTarget*);

// adapter functions
// 显卡函数
typedef int(*ADL2_DISPLAY_POSSIBLEMODE_GET) (ADL_CONTEXT_HANDLE, int, int*, ADLMode**);
typedef int(*ADL2_ADAPTER_PRIMARY_SET) (ADL_CONTEXT_HANDLE, int);
typedef int(*ADL2_ADAPTER_PRIMARY_GET) (ADL_CONTEXT_HANDLE, int*);
typedef int(*ADL2_ADAPTER_ACTIVE_SET) (ADL_CONTEXT_HANDLE, int, int, int*);



HINSTANCE hDLL;//初始化dll句柄
//创建ADL2控制句柄
ADL2_MAIN_CONTROLX2_CREATE              ADL2_Main_ControlX2_Create = NULL;
//销毁ADL2控制句柄
ADL2_MAIN_CONTROL_DESTROY               ADL2_Main_Control_Destroy = NULL;
//显卡信息获取函数
ADL2_ADAPTER_ADAPTERINFOX3_GET          ADL2_Adapter_AdapterInfoX3_Get = NULL;
//显示，显示信息获取
ADL2_DISPLAY_DISPLAYINFO_GET            ADL2_Display_DisplayInfo_Get = NULL;
//显示SLS图配置函数 
ADL2_DISPLAY_SLSMAPCONFIG_VALID         ADL2_Display_SLSMapConfig_Valid = NULL;
//显示下标函数
ADL2_DISPLAY_SLSMAPINDEX_GET		 ADL2_Display_SLSMapIndex_Get = NULL;
//显示SLSMap配置获取
ADL2_DISPLAY_SLSMAPCONFIG_GET	 ADL2_Display_SLSMapConfig_Get = NULL;
//SLSMap配置销毁
ADL2_DISPLAY_SLSMAPCONFIG_DELETE  ADL2_Display_SLSMapConfig_Delete = NULL;
//SLSMap配置获取
ADL2_DISPLAY_SLSMAPCONFIG_CREATE  ADL2_Display_SLSMapConfig_Create = NULL;
//SLS顺序更改函数,更改画面的显示顺序
ADL2_DISPLAY_SLSMAPCONFIG_REARRANGE  ADL2_Display_SLSMapConfig_Rearrange = NULL;
//连接或者断开SLS 连接
ADL2_DISPLAY_SLSMAPCONFIG_SETSTATE ADL2_Display_SLSMapConfig_SetState = NULL;
//获取当前显示的模型信息，输入显卡编号，显示窗口编号，输出模型数量和模型相关信息
ADL2_DISPLAY_MODES_GET			 ADL2_Display_Modes_Get = NULL;
//针对特殊的输入适配器的可能存在模式
ADL2_DISPLAY_POSSIBLEMODE_GET ADL2_Display_PossibleMode_Get = NULL;
//设置显示模式
ADL2_DISPLAY_MODES_SET			 ADL2_Display_Modes_Set = NULL;
//此函数检索当前显示映射配置，包括映射到每个显示的控制器和适配器。
ADL2_DISPLAY_DISPLAYMAPCONFIG_GET ADL2_Display_DisplayMapConfig_Get = NULL;
//设置每个的适配器
ADL2_DISPLAY_DISPLAYMAPCONFIG_SET ADL2_Display_DisplayMapConfig_Set = NULL;
//Set SLS bezel offsets for each display index.
ADL2_DISPLAY_BEZELOFFSET_SET		 ADL2_Display_BezelOffset_Set = NULL;
//重新设置初始化显卡
ADL2_ADAPTER_PRIMARY_SET ADL2_Adapter_Primary_Set = NULL;
//重新获取显卡信息
ADL2_ADAPTER_PRIMARY_GET ADL2_Adapter_Primary_Get = NULL;
//此功能启用或禁用指定显示的扩展桌面模式。
ADL2_ADAPTER_ACTIVE_SET ADL2_Adapter_Active_Set = NULL;
//开辟内存操作
// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}
//释放内存操作
// Optional Memory de-allocation function
void __stdcall ADL_Main_Memory_Free ( void** lpBuffer )
{
    if ( NULL != *lpBuffer )
    {
        free ( *lpBuffer );
        *lpBuffer = NULL;
    }
}
//初始化ADL
int initializeADL()
{
	
	// Load the ADL dll
	{
		hDLL = LoadLibrary(TEXT("atiadlxx.dll"));
		if (hDLL == NULL)
		{
			// A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
			// Try to load the 32 bit library (atiadlxy.dll) instead
			hDLL = LoadLibrary(TEXT("atiadlxy.dll"));
		}

		if (NULL == hDLL)
		{
			PRINTF("Failed to load ADL library\n");
			return FALSE;
		}
	}

	// Get & validate function pointers
	{
		//获取相关对应函数
        ADL2_Main_ControlX2_Create = (ADL2_MAIN_CONTROLX2_CREATE)GetProcAddress(hDLL, "ADL2_Main_ControlX2_Create");
		ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL2_Main_Control_Destroy");
        ADL2_Adapter_AdapterInfoX3_Get = (ADL2_ADAPTER_ADAPTERINFOX3_GET)GetProcAddress(hDLL, "ADL2_Adapter_AdapterInfoX3_Get");
        ADL2_Display_SLSMapConfig_Valid = (ADL2_DISPLAY_SLSMAPCONFIG_VALID)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfig_Valid");
		ADL2_Display_SLSMapIndex_Get = (ADL2_DISPLAY_SLSMAPINDEX_GET)GetProcAddress(hDLL,"ADL2_Display_SLSMapIndex_Get");
		ADL2_Display_SLSMapConfig_Get = (ADL2_DISPLAY_SLSMAPCONFIG_GET)GetProcAddress(hDLL,"ADL2_Display_SLSMapConfig_Get");
		ADL2_Display_Modes_Get = (ADL2_DISPLAY_MODES_GET)GetProcAddress(hDLL,"ADL2_Display_Modes_Get");
		ADL2_Display_PossibleMode_Get = (ADL2_DISPLAY_POSSIBLEMODE_GET)GetProcAddress(hDLL,"ADL2_Display_PossibleMode_Get");
		ADL2_Display_Modes_Set = (ADL2_DISPLAY_MODES_SET)GetProcAddress(hDLL,"ADL2_Display_Modes_Set");
		ADL2_Display_SLSMapConfig_Delete = (ADL2_DISPLAY_SLSMAPCONFIG_DELETE)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfig_Delete");
		ADL2_Display_SLSMapConfig_Create = (ADL2_DISPLAY_SLSMAPCONFIG_CREATE)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfig_Create");
		ADL2_Display_SLSMapConfig_Rearrange = (ADL2_DISPLAY_SLSMAPCONFIG_REARRANGE)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfig_Rearrange");
		ADL2_Display_SLSMapConfig_SetState = (ADL2_DISPLAY_SLSMAPCONFIG_SETSTATE)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfig_SetState");
		ADL2_Display_DisplayMapConfig_Get = (ADL2_DISPLAY_DISPLAYMAPCONFIG_GET)GetProcAddress(hDLL,"ADL2_Display_DisplayMapConfig_Get");
		ADL2_Display_DisplayMapConfig_Set = (ADL2_DISPLAY_DISPLAYMAPCONFIG_SET)GetProcAddress(hDLL, "ADL2_Display_DisplayMapConfig_Set");
		ADL2_Display_BezelOffset_Set = (ADL2_DISPLAY_BEZELOFFSET_SET) GetProcAddress(hDLL, "ADL2_Display_BezelOffset_Set");
		ADL2_Display_DisplayInfo_Get = (ADL2_DISPLAY_DISPLAYINFO_GET) GetProcAddress(hDLL,"ADL2_Display_DisplayInfo_Get");
		ADL2_Adapter_Primary_Set = (ADL2_ADAPTER_PRIMARY_SET) GetProcAddress(hDLL,"ADL2_Adapter_Primary_Set");
		ADL2_Adapter_Primary_Get = (ADL2_ADAPTER_PRIMARY_GET) GetProcAddress(hDLL,"ADL2_Adapter_Primary_Get");
		ADL2_Adapter_Active_Set = (ADL2_ADAPTER_ACTIVE_SET) GetProcAddress(hDLL,"ADL2_Adapter_Active_Set");
        //检查函数指针是否都获取到了
        if (NULL == ADL2_Main_ControlX2_Create ||
            NULL == ADL2_Main_Control_Destroy ||
            NULL == ADL2_Adapter_AdapterInfoX3_Get ||
            NULL == ADL2_Display_SLSMapConfig_Valid ||
			 NULL == ADL2_Display_SLSMapIndex_Get ||
			 NULL == ADL2_Display_SLSMapConfig_Get ||
			 NULL == ADL2_Display_Modes_Get ||
			 NULL == ADL2_Display_Modes_Set ||
			 NULL == ADL2_Display_DisplayMapConfig_Get ||
			 NULL == ADL2_Display_SLSMapConfig_Delete ||
			 NULL == ADL2_Display_SLSMapConfig_Create ||
			 NULL == ADL2_Display_SLSMapConfig_Rearrange ||
			 NULL == ADL2_Display_SLSMapConfig_SetState ||
			 NULL == ADL2_Display_DisplayMapConfig_Set ||
			 NULL == ADL2_Display_BezelOffset_Set ||
			 NULL == ADL2_Display_DisplayInfo_Get ||
			 NULL == ADL2_Adapter_Primary_Set ||
			 NULL == ADL2_Adapter_Primary_Get ||
			 NULL == ADL2_Adapter_Active_Set ||
			 NULL == ADL2_Display_PossibleMode_Get
			 )
		{
			PRINTF("Failed to get ADL function pointers\n");
			return FALSE;
		}
	}
	//创建控制句柄
    if (ADL_OK != ADL2_Main_ControlX2_Create(ADL_Main_Memory_Alloc, 1, &ADLContext_, ADL_THREADING_LOCKED))
	{
		PRINTF("ADL2_Main_ControlX2_Create() failed\n");
		return FALSE;
	}
	
	return TRUE;
}	
//销毁ADL
void deinitializeADL()
{
    if (NULL != ADL2_Main_Control_Destroy)
        ADL2_Main_Control_Destroy(ADLContext_);

    FreeLibrary(hDLL);
}


// 设置显卡显示到宽域增强

int setAdapterDisplaysToEyefinity(
	int iAdapterIndex, //显卡编号
	int iRows, //行
	int iColumns,//列
	int iDisplayMapIndexes[], //显示映射编号数组 
	int iNumOfDisplays, //显示编号
	int iSLSRearrange //设置顺序
	)
{
	int iRetVal = 1;//判断是否有返回值

	int iSLSMapIndexOut;//SLS映射的最大值
    //int  iAdapterIndex;
	int i=0,j=0,iCurrentSLSTarget;//显卡编号
	int displayFound=0;//显示是否找到
	ADLSLSTarget* pSLSTargets = NULL;//SLS目标
	ADLSLSMap slsMap;//ADL SLS映射

	//to get the modes
	int iNumModes;
	//ADL 模式指针
	ADLMode* pModes=NULL;  //ADL模型

	//need for Get Display Mappings
	//显示映射需要参数
	//目标显示数量
	int iNumDisplayTarget = 0;
	// ADL显示目标
	ADLDisplayTarget *lpDisplayTarget = NULL;
	//显示映射数量
	int iNumDisplayMap = 0;
	//显示映射
	ADLDisplayMap *lpDisplayMap = NULL;
	//SLS 映射下标
	int iSLSMapIndex = -1;
	//SLS 映射值
	int iSLSMapValue = 512;//Fill

	//Holds the refresh SLS info params
	//保持SLS的刷新频率
	//创建SLS映射
	ADLSLSMap lpCreatedSLSMap;
	//SLS目标
	ADLSLSTarget* lppCreatedSLSTargets = NULL;
	//SLS动态模型
	ADLSLSMode* lppCreatedSLSNativeModes = NULL;
	//缝隙模式
	ADLBezelTransientMode* lppCreatedBezelModes = NULL;
	//临时模式
	ADLBezelTransientMode* lppCreatedTransientModes = NULL;
	//SLS偏移
	ADLSLSOffset* lppCreatedBezelOffsets = NULL;
	//创建的SLS目标数量
	int iNumCreatedSLSTargets = 0;
	//创建的相对模型目标数量
	int iNumCreatedSLSNativeModes = 0;
	//创建的边缘模型数量
	int iNumCreatedBezelModes = 0;
	//创建的临时模型数量
	int iNumCreatedTransientModes = 0;
	//边缘偏移
	int iNumCreatedBezelOffsets = 0;
	LPCSTR EnvironmentVariable = "ADL_4KWORKAROUND_CANCEL";

    //ADLDisplayTarget* lpDisplayTarget = (ADLDisplayTarget*)ADL_Main_Memory_Alloc(iNumOfDisplays * sizeof(ADLDisplayTarget));
    //ZeroMemory(lpDisplayTarget, iNumOfDisplays * sizeof(ADLDisplayTarget));

	//为目标SLS创建内存空间
	pSLSTargets = (ADLSLSTarget*) malloc(iNumOfDisplays * sizeof(ADLSLSTarget));	
	
 	//iAdapterIndex = getAdapterIndex(iDisplayMapIndexes[0]);
	//如果显卡编号不等于-1
	if (iAdapterIndex != -1)
	{
		//如果存在顺序，
		if (!iSLSRearrange)
		{
			//Makeing this Adapter as Primary Adapater
			//初始化显卡
			setPrimaryAdapter(iAdapterIndex);

			//Disabling Extended Surfaces
			//禁止边缘扩展和复制
			setAdapterDisplaysToDisable(iAdapterIndex);
		
			//set to Clone Disabled displays
			//禁止复制
			setAdapterDisplaysToClone(iAdapterIndex, iDisplayMapIndexes, iNumOfDisplays);
		}
		//获取model信息，存储在iNumModes 和pModes中
        iRetVal = ADL2_Display_Modes_Get(ADLContext_, iAdapterIndex, -1, &iNumModes, &pModes);
        
		//Setting the SLSMAP Information
		//初始化，ADLSLSMap映射
		memset(&slsMap, 0, sizeof(ADLSLSMap));
		//设置SLS列数,为输入列
		slsMap.grid.iSLSGridColumn = iColumns;
		//设置SLS行数,为输入行
		slsMap.grid.iSLSGridRow = iRows;
		//显卡编号,为输入编号
		slsMap.iAdapterIndex = iAdapterIndex;
		//显卡映射编号为-1
		slsMap.iSLSMapIndex = iSLSMapIndex; 
		//设置动态模型数量
		slsMap.iNumNativeMode = 0;
		//边缘模型数
		slsMap.iNumBezelMode = 0;
		//设置旋转角度
		slsMap.iOrientation = pModes[0].iOrientation;
		//设置映射值
		slsMap.iSLSMapValue = 0;

        
		//获取显示映射配置
		//此函数检索当前的显示映射配置，包括映射到每个显示的控制器和适配器。
        iRetVal = ADL2_Display_DisplayMapConfig_Get(
        	ADLContext_, //ADL上下文 in
        	iAdapterIndex,//图形适配器编号 out
			&iNumDisplayMap,//显示映射数量 out
			&lpDisplayMap, //显示映射 out
			&iNumDisplayTarget,//显示目标数量 out
			&lpDisplayTarget, //显示目标 out
			ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO//指示音频采样率的最大数目 in
			);

        //成功获取
		if (ADL_OK == iRetVal&& 1<iNumDisplayTarget)
		{
			//获取SLS映射的index,
            iRetVal = ADL2_Display_SLSMapIndex_Get(
            	ADLContext_,//ADL上下文 in
            	iAdapterIndex,//显示器编号 in
				iNumDisplayTarget,//显示目标数量 in
				lpDisplayTarget, //目标显示数组
				&iSLSMapIndex //SLS映射下标
				);
            // 成功查询到对应的SLS映射编号
			if (ADL_OK == iRetVal && iSLSMapIndex != -1)
			{
				//This is a temporary workaround to enable SLS.
				//Set this variable to any value.
				//允许使用SLS
				SetEnvironmentVariable(EnvironmentVariable, "TRUE");
				//ADL获取显示配置
                iRetVal = ADL2_Display_SLSMapConfig_Get(
                	ADLContext_, //in ADL上下文
                	iAdapterIndex, //in 显示器编号
                	iSLSMapIndex,  //in 映射编号
                	&lpCreatedSLSMap, //out SLS map数据
					&iNumCreatedSLSTargets,  //out 指向变量的指针，该变量将接收SLS映射中的目标数。
					&lppCreatedSLSTargets,// out 指向SLS映射中的目标变量的指针。数据类型为adldisplayTarget应用程序不需要分配内存，但应该释放指针，因为内存是由adl回调函数分配的。
					&iNumCreatedSLSNativeModes, //out 指向一个变量的指针，该变量将接收SLS配置支持的本机SLS模式数。
					&lppCreatedSLSNativeModes,//out  指向包含本机SLS模式偏移量的变量的指针。数据类型为adlslsoffset。应用程序不需要分配内存，但应该释放指针，因为内存是由ADL回调函数分配的。
					&iNumCreatedBezelModes, //out 指向一个变量的指针，该变量将接收SLS配置支持的边框模式数。
					&lppCreatedBezelModes, //out 指向包含边框SLS模式的变量的指针。数据类型为ADLSLSMODE。应用程序不需要分配内存，但应该释放指针，因为内存是由ADL回调函数分配的。
					&iNumCreatedTransientModes, //out 指向一个变量的指针，该变量将接收SLS配置支持的瞬态模式数。
					&lppCreatedTransientModes, //out 指向包含瞬态SLS模式的变量的指针。数据类型为ADLSLSMODE。应用程序不需要分配内存，但应该释放指针，因为内存是由ADL回调函数分配的。
					&iNumCreatedBezelOffsets, //out 指向一个变量的指针，该变量将接收由SLS配置支持的SLS偏移量的数目。
					&lppCreatedBezelOffsets,//out 指向包含SLS偏移量的变量的指针。数据类型为adlslsoffset。应用程序不需要分配内存，但应该释放指针，因为内存是由ADL回调函数分配的。
					ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE //in  指定SLS网格数据的布局类型。它是位向量。有两种类型的SLS布局：S，相对于景观（Ref）和相对于当前角度（Ref）
					);

                //查询成功
				if (ADL_OK == iRetVal)
				{
					//如果存在设置显示顺序	
					if (iSLSRearrange)
					{
						// 列数为
						iColumns = lpCreatedSLSMap.grid.iSLSGridColumn;
						// slsMap的列数
						slsMap.grid.iSLSGridColumn = lpCreatedSLSMap.grid.iSLSGridColumn;
						//行数
						iRows = lpCreatedSLSMap.grid.iSLSGridRow;
						slsMap.grid.iSLSGridRow = lpCreatedSLSMap.grid.iSLSGridRow;
					}
					//设置映射索引
					slsMap.iSLSMapIndex = iSLSMapIndex; 
					//本机模式数量
					slsMap.iNumNativeMode = iNumCreatedSLSNativeModes;
					//本机边框模式数量
					slsMap.iNumBezelMode = iNumCreatedBezelModes;
				}

			}
		}		
		//如果存在顺序
		if (iSLSRearrange)
		{
			//显示数目不等于创建目标数目
			if (iNumOfDisplays != iNumCreatedSLSTargets)
			{
				PRINTF("Number of displays currently in SLS are not matched with number of displays provided for re arrange\n");
				//SLS中当前显示的数量与重新排列所提供的显示数量不匹配。
				return 0;
			}
			//当SLS中当前显示数目与重新排列的目标数目相同的时候
			//
			for (i=0; i<iNumCreatedSLSTargets;i++)
			{
				//查找显示
				displayFound = 0;
				// 遍历当前的SLS
				for (j=0;j<iNumCreatedSLSTargets;j++)
				{
					//如果当前显示的逻辑序号等于显示映射序号
					if (lppCreatedSLSTargets[j].displayTarget.displayID.iDisplayLogicalIndex == iDisplayMapIndexes[i])
					{
						//找到了
						displayFound = 1;
						//返回
						break;
					}
				}
				//如果没有找到
				if (!displayFound)
				{
					PRINTF("provided display is not particepating in SLS\n");
					//提供的显示在SLS中不参与
					return 0;
				}
			}
		}
		//设当前SLS目标为0
		iCurrentSLSTarget=0;
		//按照行列遍历
		for (i=0; i<iRows; i++)
		{
			for (j=0; j<iColumns; j++)
			{
				//设置目标显示的显示器编号
				pSLSTargets[iCurrentSLSTarget].iAdapterIndex = iAdapterIndex;
				//为显示目标分配内存
				memset(&(pSLSTargets[iCurrentSLSTarget].displayTarget), 0, sizeof(ADLDisplayTarget));
				//设置目标显示的逻辑索引编号为当前显示编号，这个是手动输入的
				pSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex = iDisplayMapIndexes[iCurrentSLSTarget];
				//设置当前的物理编号
				pSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayPhysicalIndex = iDisplayMapIndexes[iCurrentSLSTarget];
				// 设置逻辑显卡
				pSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalAdapterIndex = iAdapterIndex;
				// 设置物理显卡
				pSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayPhysicalAdapterIndex = iAdapterIndex;
				//设置映射索引值
				pSLSTargets[iCurrentSLSTarget].iSLSMapIndex = iSLSMapIndex;
				//为显示范围，分配内存
				memset(&(pSLSTargets[iCurrentSLSTarget].viewSize), 0, sizeof(ADLMode));
				// 显示尺寸:显卡编号
				pSLSTargets[iCurrentSLSTarget].viewSize.iAdapterIndex = iAdapterIndex;
				//显示尺寸 模型
				pSLSTargets[iCurrentSLSTarget].viewSize.iModeFlag = pModes[0].iModeFlag;
				//设置显示旋转角
				pSLSTargets[iCurrentSLSTarget].viewSize.iOrientation = pModes[0].iOrientation;
				//设置显示刷新率
				pSLSTargets[iCurrentSLSTarget].viewSize.fRefreshRate = pModes[0].fRefreshRate;
				//设置显示色彩深度
				pSLSTargets[iCurrentSLSTarget].viewSize.iColourDepth = pModes[0].iColourDepth;
				//设置显示x坐标
				pSLSTargets[iCurrentSLSTarget].viewSize.iXPos = pModes[0].iXPos;
				//设置显示y坐标
				pSLSTargets[iCurrentSLSTarget].viewSize.iYPos = pModes[0].iYPos;
				//设置X轴宽度
				pSLSTargets[iCurrentSLSTarget].viewSize.iXRes = pModes[0].iXRes;
				//设置Y轴宽度
				pSLSTargets[iCurrentSLSTarget].viewSize.iYRes = pModes[0].iYRes;

				//设置网格布局的坐标
				pSLSTargets[iCurrentSLSTarget].iSLSGridPositionX = j;
				pSLSTargets[iCurrentSLSTarget].iSLSGridPositionY = i;

                // this is used only for SLS builder; set "Enabled" in all other cases
                pSLSTargets[iCurrentSLSTarget].iSLSTargetValue = 0x0001;
                //设置目标掩码
                pSLSTargets[iCurrentSLSTarget].iSLSTargetMask = 0x0001;
                //当前目标数+1
				iCurrentSLSTarget++;
			}		
		}

        bool ok = false;
        //设置支持的输出显示模式
        int supportedLayoutModes = -1, 
        //不支持
        reasonForNotSupport = -1, 
        //相关参数选项
        option = ADL_DISPLAY_SLSMAPCONFIG_CREATE_OPTION_RELATIVETO_CURRENTANGLE;
        // Validate Fit
        // 验证匹配
        slsMap.iSLSMapValue |= ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FIT;
        //
        int ret = ADL2_Display_SLSMapConfig_Valid(
        	ADLContext_, //ADL上下文
        	iAdapterIndex, //显卡编号
        	slsMap,  //系统映射
        	iNumOfDisplays,  //显示数量
        	pSLSTargets, //SLS目标
        	&supportedLayoutModes, //支持的显示模式 
        	&reasonForNotSupport, //不支持的显示模式
        	option//选项参数
        	);
        //查询成功，并且没有不支持的显示参数
        if (ADL_OK == ret && 0 == reasonForNotSupport)
        {
            ok = true;
        }
        // Validate Fill
        // 验证匹配
        if (!ok)
        {
        	//进行与运算
            slsMap.iSLSMapValue ^= ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FIT;
            //进行或运算
            slsMap.iSLSMapValue |= ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FILL;
            //
            int ret = ADL2_Display_SLSMapConfig_Valid(
            	ADLContext_, 
            	iAdapterIndex, 
            	slsMap, 
            	iNumOfDisplays, 
            	pSLSTargets, 
            	&supportedLayoutModes, 
            	&reasonForNotSupport, 
            	option
            	);
            //查询成功重新设置ok
            if (ADL_OK == ret && 0 == reasonForNotSupport)
                ok = true;
        }
        //如果没有按照顺序
		if (!iSLSRearrange)
		{
			//此函数创建具有给定网格、目标和边框模式百分比的SLS配置。如果成功创建了SLS映射，它将输出一个SLS映射索引。
            iRetVal = ADL2_Display_SLSMapConfig_Create(
                    ADLContext_,//ADL上下文
					iAdapterIndex,//ADL 显示器
					slsMap,//指定包含SLS网格和方向信息的显示SLS地图数据。它的类型是adlsmap,前面已经指定过了
					iNumOfDisplays,//设置显示的数目
					pSLSTargets,//用于创建SLS的显示数组。数据类型为adlslsTarget。
					0, // bezel mode percent 指定边框模式百分比。如果这些显示不支持边框，请将其设置为0。
					&iSLSMapIndexOut, //指向将接收新创建的SLS映射索引的变量的指针。如果SLS网格创建失败，则其值为-1，否则为非负整数。
                    option //指定SLS网格数据的布局类型。它是位向量。SLS布局有两种类型：相对于横向（Ref）和相对于当前角度（Ref）。
					);
            //如果上面语句支撑成功
			if (iRetVal == ADL_OK)
				PRINTF("SLS created Successfully\n");			
			else
			{
					PRINTF("Unable to create SLS\n");
			}
		}
		else //按照排序来
		{
			//此功能更改SLS地图中的显示顺序，以便使图像正确显示在显示屏上。
			//
            iRetVal = ADL2_Display_SLSMapConfig_Rearrange(
            	ADLContext_, //ADL句柄 
            	iAdapterIndex,  //显卡序号
            	slsMap.iSLSMapIndex,  //SLS地图信息的下标索引
            	iNumOfDisplays, //显示的数量
            	pSLSTargets, //SLS目标
            	lpCreatedSLSMap,  // 指定SLS映射信息。它的类型是adlsmap。唯一需要的数据是地图方向。所有其他的都被函数忽略。
            	option //选择参数
            	);
            //创建成功
			if (iRetVal == ADL_OK)
				PRINTF("SLS Re-arranged Successfully\n");			
			else
				{
					PRINTF("Unable to re-arrange SLS\n");
				}
				
		}
	}
	//--- 销毁 start---
	ADL_Main_Memory_Free((void**)&pSLSTargets);
	ADL_Main_Memory_Free((void**)&lppCreatedSLSTargets);
	ADL_Main_Memory_Free((void**)&lppCreatedSLSNativeModes);
	ADL_Main_Memory_Free((void**)&lppCreatedBezelModes);
	ADL_Main_Memory_Free((void**)&lppCreatedTransientModes);
	ADL_Main_Memory_Free((void**)&lppCreatedBezelOffsets);
	ADL_Main_Memory_Free((void**)&pModes);
	ADL_Main_Memory_Free((void**)&lpDisplayMap);
	ADL_Main_Memory_Free((void**)&lpDisplayTarget);
	//--- 销毁 end--- 
	return iRetVal;//是否创建成功
}
//设置显示偏移
int setBezelOffsets(
	int iAdapterIndex,//显卡 
	int iHbezel, //水平偏移
	int iVbezel //垂直偏移
	)
{
	int iRetVal = 1;

	int iSLSMapIndexOut;//SLS映射输出索引
	//设置辅助i,j参数，当前SLS目标
	int i=0,j=0, iCurrentSLSTarget;

	//need for Get Display Mappings
	//获取显示映射需要的参数
	//目标显示数量
	int iNumDisplayTarget = 0;
	//目标显示指针
	ADLDisplayTarget *lpDisplayTarget = NULL;
	//显示映射数量
	int iNumDisplayMap = 0;
	//ADL显示映射
	ADLDisplayMap *lpDisplayMap = NULL;
	// SLS映射索引
	int iSLSMapIndex = -1;

	//Holds the refresh SLS info params
	//刷新相关参数
	//SLSmap
	ADLSLSMap lpCreatedSLSMap;
	//SLS目标指针
	ADLSLSTarget* lppCreatedSLSTargets = NULL;
	//SLS模型指针
	ADLSLSMode* lppCreatedSLSNativeModes = NULL;
	//
	ADLBezelTransientMode* lppCreatedBezelModes = NULL;
	ADLBezelTransientMode* lppCreatedTransientModes = NULL;
	ADLSLSOffset* lppCreatedBezelOffsets = NULL;
	int iNumCreatedSLSTargets = 0;
	int iNumCreatedSLSNativeModes = 0;
	int iNumCreatedBezelModes = 0;
	int iNumCreatedTransientModes = 0;
	int iNumCreatedBezelOffsets = 0;
	int iCurrentTransientMode;
	//For Bezel Width and Height Ref Values
	//设置参考宽度和高度
	int iReferenceWidth = 0;
	int iReferenceHeight = 0;
	int iCurrentBezelOffset = 0;
	int iNumModes;
	//设置SLS映射值
	int iSLSMapValue = 512;//Fill
	//设置ADL模式指针
	ADLMode *pDisplayMode = NULL;	
	//设置应用边框偏移
	ADLSLSOffset* lpAppliedBezelOffsets = NULL;
	//设置临时边框应用偏移
	ADLSLSOffset* lpTempAppliedBezelOffsets = NULL;
	LPCSTR EnvironmentVariable = "ADL_4KWORKAROUND_CANCEL";
	//获取当前的显示配置
    iRetVal = ADL2_Display_DisplayMapConfig_Get(
    	ADLContext_, 
    	iAdapterIndex,
		&iNumDisplayMap,
		&lpDisplayMap, 
		&iNumDisplayTarget, 
		&lpDisplayTarget, 
		ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO
		);
    //正常获取之后
	if (ADL_OK == iRetVal&& 1<iNumDisplayTarget)
	{
		//获取SLS映射索引
        iRetVal = ADL2_Display_SLSMapIndex_Get(
        	ADLContext_,//上下文 
        	iAdapterIndex,//显卡
			iNumDisplayTarget,//显示目标总数
			lpDisplayTarget,//显示目标数组
			&iSLSMapIndex //SLS映射索引
			);
        //映射缩影存在
		if (iSLSMapIndex != -1)
		{
			//This is a temporary workaround to enable SLS.
			//Set this variable to any value.
			
			//设置工作环境可行
			SetEnvironmentVariable(EnvironmentVariable, "TRUE");
			//获取SLS映射配置
            iRetVal = ADL2_Display_SLSMapConfig_Get(
            	ADLContext_, 
            	iAdapterIndex, 
            	iSLSMapIndex, 
            	&lpCreatedSLSMap,
				&iNumCreatedSLSTargets, 
				&lppCreatedSLSTargets,
				&iNumCreatedSLSNativeModes, 
				&lppCreatedSLSNativeModes,
				&iNumCreatedBezelModes, 
				&lppCreatedBezelModes,
				&iNumCreatedTransientModes, 
				&lppCreatedTransientModes,
				&iNumCreatedBezelOffsets, 
				&lppCreatedBezelOffsets,
				ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE 
				);
            //初始化配置没有问题
			if (ADL_OK == iRetVal)
			{
				//iRetVal = ADL_Display_Modes_Get(iAdapterIndex, -1, &iNumModes, &pDisplayMode);

				//currentMode = pDisplayMode[iNumModes - 1];
				//for(iCurrentDisplayMode = 0; iCurrentDisplayMode < iNumModes; iCurrentDisplayMode++)
				//{
				//	// we need to figure out the positioning of this display in the SLS
				//	pDisplayMode[iCurrentDisplayMode].iXRes = currentMode.iXRes / lpCreatedSLSMap.grid.iSLSGridColumn;
				//	pDisplayMode[iCurrentDisplayMode].iYRes = currentMode.iYRes / lpCreatedSLSMap.grid.iSLSGridRow;
				//	iRetVal = ADL_Display_Modes_Set(iAdapterIndex, -1, 1, &pDisplayMode[iCurrentDisplayMode]);
				//	break;
				//}
				//断开原有的连接，为更新做准备
                iRetVal = ADL2_Display_SLSMapConfig_SetState(ADLContext_, iAdapterIndex, iSLSMapIndex, 0);
			}		
		}
			
	}
	//输入参数校验正确
	if (iRetVal == 0 && (0 != iHbezel || 0 != iVbezel))
	{
		//根据输入，创建SLS映射配置
		iRetVal = ADL2_Display_SLSMapConfig_Create (
            ADLContext_,//in
			iAdapterIndex,//in
			lpCreatedSLSMap,//in
			iNumCreatedSLSTargets,//in
			lppCreatedSLSTargets,//in
			10, // bezel mode percent
			&iSLSMapIndexOut,//out  指向生成的SLS Map的初始索引
			ADL_DISPLAY_SLSMAPCONFIG_CREATE_OPTION_RELATIVETO_LANDSCAPE
			);

	
		//获取SLS Map的配置参数
        iRetVal = ADL2_Display_SLSMapConfig_Get(
        	ADLContext_, 
        	iAdapterIndex, 
        	lpCreatedSLSMap.iSLSMapIndex, 
        	&lpCreatedSLSMap,
			&iNumCreatedSLSTargets,
			&lppCreatedSLSTargets,
			&iNumCreatedSLSNativeModes, 
			&lppCreatedSLSNativeModes,
			&iNumCreatedBezelModes, 
			&lppCreatedBezelModes,
			&iNumCreatedTransientModes, 
			&lppCreatedTransientModes,
			&iNumCreatedBezelOffsets, 
			&lppCreatedBezelOffsets,
			ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE
			);

		//创建的临时模型数量
		if (iNumCreatedTransientModes > 0)
		{
			//ADL临时模型指针
			ADLMode *pTransientModes = NULL;
            //获取当前显示的模型，返回模型数量和数组
            iRetVal = ADL2_Display_Modes_Get(ADLContext_, iAdapterIndex, -1, &iNumModes, &pTransientModes);

			//lppCreatedTransientModes = (ADLBezelTransientMode*) malloc(iNumCreatedTransientModes * sizeof(ADLBezelTransientMode));
			//遍历临时模型更改位置
			for(iCurrentTransientMode = 0; iCurrentTransientMode < iNumCreatedTransientModes; iCurrentTransientMode++)
			{
				// we need to figure out the positioning of this display in the SLS
				//指明显示位置
				pTransientModes[iCurrentTransientMode].iXRes = lppCreatedTransientModes[iCurrentTransientMode].displayMode.iXRes;
				pTransientModes[iCurrentTransientMode].iYRes = lppCreatedTransientModes[iCurrentTransientMode].displayMode.iYRes;
                //设置显示位置
                iRetVal = ADL2_Display_Modes_Set(ADLContext_, iAdapterIndex, -1, 1, &pTransientModes[iCurrentTransientMode]);
				break;
			}
			//设置成功
			if (iRetVal == 0)
			{
				//重新查询 SLS映射配置
                iRetVal = ADL2_Display_SLSMapConfig_Get(
                	ADLContext_, 
                	iAdapterIndex, 
                	lpCreatedSLSMap.iSLSMapIndex, 
                	&lpCreatedSLSMap,
					&iNumCreatedSLSTargets, 
					&lppCreatedSLSTargets,
					&iNumCreatedSLSNativeModes, 
					&lppCreatedSLSNativeModes,
					&iNumCreatedBezelModes, 
					&lppCreatedBezelModes,
					&iNumCreatedTransientModes, 
					&lppCreatedTransientModes,
					&iNumCreatedBezelOffsets, 
					&lppCreatedBezelOffsets,
					ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE
					);
                //开始设置偏移
                //检查偏移设置是否大于0
				if (iNumCreatedBezelOffsets > 0)
				{
					//设置参考高度为边缘偏移
					iReferenceWidth = lppCreatedBezelOffsets[iNumCreatedBezelOffsets - 1].iDisplayWidth;
					iReferenceHeight = lppCreatedBezelOffsets[iNumCreatedBezelOffsets - 1].iDisplayHeight;

					
					//Try applying Permanently
					//为应用偏移结构体，分配内存
					lpAppliedBezelOffsets = (ADLSLSOffset*) malloc(iNumCreatedSLSTargets * sizeof(ADLSLSOffset));
					//Set the initial offsets to start setting the user offsets
					//设置初始化的偏移，到用户起始设置
					for(iCurrentSLSTarget = 0; iCurrentSLSTarget < iNumCreatedSLSTargets; iCurrentSLSTarget++)//遍历目标
					{
						for (iCurrentBezelOffset = 0; iCurrentBezelOffset< iNumCreatedBezelOffsets; iCurrentBezelOffset++)//遍历当前偏移
						{
							if (lppCreatedBezelOffsets[iCurrentBezelOffset].iBezelModeIndex == lppCreatedTransientModes[iNumCreatedTransientModes - 1].iSLSModeIndex &&
								lppCreatedBezelOffsets[iCurrentBezelOffset].displayID.iDisplayLogicalIndex == lppCreatedSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex)//索引相同的时候
							{
								lpAppliedBezelOffsets[iCurrentSLSTarget].iBezelOffsetX = lppCreatedBezelOffsets[iCurrentBezelOffset].iBezelOffsetX - (lppCreatedSLSTargets[iCurrentSLSTarget].iSLSGridPositionX * iHbezel);//设置X轴的偏移
								lpAppliedBezelOffsets[iCurrentSLSTarget].iBezelOffsetY = lppCreatedBezelOffsets[iCurrentBezelOffset].iBezelOffsetY - (lppCreatedSLSTargets[iCurrentSLSTarget].iSLSGridPositionY * iVbezel);//设置y轴的偏移

								lpAppliedBezelOffsets[iCurrentSLSTarget].iAdapterIndex = iAdapterIndex;//设置显卡
								lpAppliedBezelOffsets[iCurrentSLSTarget].iSLSMapIndex = lpCreatedSLSMap.iSLSMapIndex;//设置映射编号
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayLogicalAdapterIndex = iAdapterIndex;//设置显示的先打编号
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayLogicalIndex = lppCreatedSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex;//设置显示下标
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayPhysicalAdapterIndex = iAdapterIndex;//设置物理显卡
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayPhysicalIndex = lppCreatedSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex;//设置逻辑显卡
								lpAppliedBezelOffsets[iCurrentSLSTarget].iDisplayWidth = iReferenceWidth;//设置显示宽度
								lpAppliedBezelOffsets[iCurrentSLSTarget].iDisplayHeight = iReferenceHeight;//设置显示高度
							}
						}
					}
					//设置掩码为0
					lpCreatedSLSMap.iSLSMapMask = 0;
					//设置验证匹配
					lpCreatedSLSMap.iSLSMapValue = iSLSMapValue;
                    //设置相关偏移
                    iRetVal = ADL2_Display_BezelOffset_Set(
                    	ADLContext_, 
                    	iAdapterIndex, 
                    	lpCreatedSLSMap.iSLSMapIndex, 
                    	iNumCreatedSLSTargets, 
                    	lpAppliedBezelOffsets, //位置偏移相关参数
                    	lpCreatedSLSMap, 
                    	ADL_DISPLAY_BEZELOFFSET_COMMIT | ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE
                    	);

					if (iRetVal != 0)
					{
						PRINTF("Setting bezel offsets failed");
					}
				}
			}
		}
	}

	return iRetVal;
}
//设置解决方案,输入显卡编号，宽度，高度
int setResolution(int iAdapterIndex, int iXRes, int iYRes)
{
	//
	int iNumModes, //模型数量 
	iNumModesPriv, //
	iRetVal;//函数是否成功
	
	ADLMode *pModes = NULL,//模式指针 
	*lppModesPriv = NULL;//模式指针预览
	int iResfound = 0,i;//长度查找辅助函数
	//如果显卡索引存在
	if (iAdapterIndex != -1)
	{
		// Check if the mode is in the possible mode list.
		// 确认显卡可能存在显示模型
        iRetVal = ADL2_Display_PossibleMode_Get(
        	ADLContext_, 
        	iAdapterIndex, 
        	&iNumModesPriv,// lppmodes数组长度
        	&lppModesPriv //lppmodes 数组，存储可能存在的模型
        	);
        //成功获取
		if (NULL != lppModesPriv)
		{
			//遍历并且查找，宽度匹配的
			for ( i=0;i<iNumModesPriv;i++)
			{
				if( lppModesPriv[i].iXRes == iXRes && lppModesPriv[i].iYRes == iYRes)
				{			
					iResfound=1;//找到就直接跳出
					break;
				}
			}			
		}
		//如果存在相同尺寸的
		if (iResfound)
		{
			//获取当前显卡内的所有显示模式
            iRetVal = ADL2_Display_Modes_Get(ADLContext_, iAdapterIndex, -1, &iNumModes, &pModes);
			//查询成功
			if (iRetVal == ADL_OK)
			{
				//设置第一个模式为可能存在模型
				pModes[0].iXRes = iXRes;
				pModes[0].iYRes = iYRes;
				//更改显示模式
                iRetVal = ADL2_Display_Modes_Set(ADLContext_, iAdapterIndex, -1, 1, &pModes[0]);
				//更高成功
				if (iRetVal != ADL_OK)
				{
					PRINTF("unable to set provided resolution \n");
				}
			}
		}
		else
		{
			PRINTF("not a valid resolution \n");
		}
	}
	else
	{
		PRINTF("not a valid adapter index \n");
	}
	//释放相关内存
	ADL_Main_Memory_Free((void**)&pModes);
	ADL_Main_Memory_Free((void**)&lppModesPriv);
	return TRUE;
}
//设置初始化显卡
int setPrimaryAdapter(int iAdapterIndex)
{
	//记录当前显卡编号
	int iCurrentAdapterIndex = 0;
    //获取显卡初始化后的编号
    if (ADL_OK == ADL2_Adapter_Primary_Get(ADLContext_, &iCurrentAdapterIndex))
	{
		//如果编号不同，并且可以更改
		if (iCurrentAdapterIndex != iAdapterIndex && CanSetPrimary(iAdapterIndex ,iCurrentAdapterIndex))
		{
			//重新设置显卡编号
            ADL2_Adapter_Primary_Set(ADLContext_, iAdapterIndex);
		}
	}
	return TRUE;
}
//禁止显卡显示
int setAdapterDisplaysToDisable(int iAdapterIndex)
{
	//显卡列表
	int iAdapterIndexes[6];
	//显示列表
	int iDisplayIndexes[6];
	//计数
	int iCount=0;
	//是否激活
	int active;
	//编号
	int i;
	//获取显示在显卡中的索引
	getDisplayIndexesofOneGPU(iAdapterIndex, &iAdapterIndexes[0], &iDisplayIndexes[0], &iCount);

	for (i=0;i<iCount;i++)
	{
		// 在先科给定的索引中查找不相同数目的缩影
		if (iAdapterIndex != iAdapterIndexes[i] && iAdapterIndexes[i] != -1)
		{
			//禁止扩展显示到其它桌面
            ADL2_Adapter_Active_Set(ADLContext_, iAdapterIndexes[i], 0, &active);
		}
	}
	return TRUE;
}
//设置显卡可以复制显示
int setAdapterDisplaysToClone(
	int iAdapterIndex,  //显卡编号
	int iDisplayIndexes[],  //显示数组
	int iDisplaysCount //显示的总数
	)
{
	//辅助计数 i
	int i;
	// ADL显示映射
	ADLDisplayMap mapArray;
    //ADL目标映射
    ADLDisplayTarget* pDisplayTargets = NULL;

    //模式数量
	int iNumModes;
	//ADL 模型指针
	ADLMode *pModes = NULL;
	//初试化显示目标指针
	pDisplayTargets = (ADLDisplayTarget*) malloc(iDisplaysCount * sizeof(ADLDisplayTarget));
	//设置为0值，防止内存更改存在的错误
	memset(&(mapArray), 0, sizeof(ADLDisplayMap));
	//映射数组的目标显示数目
	mapArray.iNumDisplayTarget = iDisplaysCount;
	//目标显示下标索引
	mapArray.iDisplayMapIndex = 0;
	//获取显示的模式
    ADL2_Display_Modes_Get(ADLContext_,iAdapterIndex, -1, &iNumModes, &pModes);
	//为mapArray 初始化值
	memset(&(mapArray.displayMode), 0, sizeof(ADLMode));
	//设置显卡编号
	mapArray.displayMode.iAdapterIndex = iAdapterIndex;
	//设置目标显示模式的参数为原来的显示模式
	mapArray.displayMode.iModeFlag = pModes[0].iModeFlag;
	mapArray.displayMode.iOrientation = pModes[0].iOrientation;
	mapArray.displayMode.fRefreshRate = pModes[0].fRefreshRate;
	mapArray.displayMode.iColourDepth = pModes[0].iColourDepth;
	mapArray.displayMode.iXPos = pModes[0].iXPos;//x坐标
	mapArray.displayMode.iYPos = pModes[0].iYPos;//y坐标
	mapArray.displayMode.iXRes = pModes[0].iXRes;//宽度
	mapArray.displayMode.iYRes = pModes[0].iYRes;//高度

	// 遍历显示数目
	for (i=0;i<iDisplaysCount;i++)
	{
		//初始化值
		memset(&(pDisplayTargets[i].displayID), 0, sizeof(ADLDisplayID));
		//为显示目标数组初始化值
		memset(&(pDisplayTargets[i]), 0, sizeof(ADLDisplayTarget));
		//设置逻辑显卡为当前显卡
		pDisplayTargets[i].displayID.iDisplayLogicalAdapterIndex = iAdapterIndex;
		//设置逻辑序列为当前序列
		pDisplayTargets[i].displayID.iDisplayLogicalIndex = iDisplayIndexes[i];
		pDisplayTargets[i].iDisplayMapIndex = 0;
	}
	//设置显示映射配置，主要更改了映射的相关参数，个目标的逻辑显卡序号
    ADL2_Display_DisplayMapConfig_Set(
    	ADLContext_, 
    	iAdapterIndex, 
    	1, 
    	&mapArray, 
    	iDisplaysCount, 
    	pDisplayTargets
    	);
    //释放显示内存
	ADL_Main_Memory_Free((void**)&pDisplayTargets);
	return TRUE;
}

//禁止显卡宽域映射，输入显卡序号；主要是讲显卡与映射之间的连接断掉
int disableAdapterEyefinityMapping(int iAdapterIndex)
{
	//记录显示目标数量
	int iNumDisplayTarget = 0;
	//ADL显示目标函数
	ADLDisplayTarget *lpDisplayTarget = NULL;
	//统计显示映射
	int iNumDisplayMap = 0;
	//ADL显示映射数组指针
	ADLDisplayMap *lpDisplayMap = NULL;
	//SLS映射索引，函数真值
	int iSLSMapIndex = -1,iRetVal=-1;
	//获取当前显卡的显示映射配置
    iRetVal = ADL2_Display_DisplayMapConfig_Get(
    	ADLContext_, 
    	iAdapterIndex,
		&iNumDisplayMap, 
		&lpDisplayMap, 
		&iNumDisplayTarget, 
		&lpDisplayTarget, 
		ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO
		);
    //函数执行成功，并且显示画面大于1进行接下来的活动
	if (ADL_OK == iRetVal&& 1<iNumDisplayTarget)
	{
		//获取SLS显示映射索引
        iRetVal = ADL2_Display_SLSMapIndex_Get(
        	ADLContext_, 
        	iAdapterIndex,
			iNumDisplayTarget,//目标显示数量
			lpDisplayTarget,//目标显示数组
			&iSLSMapIndex //SLS映射索引
			);
        //获取成功
		if (ADL_OK == iRetVal && iSLSMapIndex != -1)
		{
			//重新设置显卡和SLS连接为0
            iRetVal = ADL2_Display_SLSMapConfig_SetState(ADLContext_, iAdapterIndex, iSLSMapIndex, 0);
			if (iRetVal != ADL_OK)
			{
				PRINTF ("Unable to Disable SLS");
			}
		}
		else
		{
			PRINTF ("SLS is not created on this adapter");
		}
	}
	//否则指直接销毁
	ADL_Main_Memory_Free((void**)&lpDisplayMap);
	//返回 true
	return TRUE;
}
//输出显示索引
int printDisplayIndexes()
{
	//显卡数目，显示数目
	int  iNumberAdapters=0, iNumDisplays;
    //显卡索引值
    int  iAdapterIndex;
    //显示索引值
    int  iDisplayIndex[6];
	//总线数目
	int iBusNumber;
	
	int i=0,j=0,k=0,l=0,//遍历辅助元素 
	iGPUfound=0,  //GPU查找
	iDisplayFound=0,  //显示查找
	iGPUIndex=0, //GPU索引
	iCount=0,  //统计总数
	iGPUCounter=0; //GOU 计数
	LPAdapterInfo     lpAdapterInfo = NULL;//显卡指针
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL; //ADL显示信息
    int igpuBusIndexes[4]; //gpu总线数组

	// Obtain the number of adapters for the system
	// 查询系统中的显卡数量
    if (ADL_OK != ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, -1, &iNumberAdapters, &lpAdapterInfo))
	{
	       PRINTF("ADL2_Adapter_AdapterInfoX3_Get failed!\n");
		   return 0;
	}
    //初始化GPU数组
	for (iGPUIndex = 0; iGPUIndex < 4; iGPUIndex++)
	{
		igpuBusIndexes[iGPUIndex] = -1;
	}
	//输出 显卡和显示标号
	PRINTF (" Adapter and Displays Indexes <AdapterIndex, DisplayIndex> \n");
	PRINTF (" --------------------------------------------------------- \n");
    // Repeat for all available adapters in the system
    // 遍历显卡
    for ( i = 0; i < iNumberAdapters; i++ )
    {
    	//初始化GPU found 
		iGPUfound = 0;
		//获取总线信息
		iBusNumber = lpAdapterInfo[ i ].iBusNumber;
		//查找总线下标对应的值
		for (iGPUIndex = 0; iGPUIndex < 4; iGPUIndex++)
		{
			if(igpuBusIndexes[iGPUIndex] != -1 && igpuBusIndexes[iGPUIndex] == iBusNumber)
			{
				iGPUfound = 1;
				break;
			}
		}
		// 如果没有找到对应的GPU
		if (!iGPUfound)
		{
			//初始化6个显示下标数组
			for (l=0;l<6;l++)
			{
				iDisplayIndex[l] = -1;				
			}
			//统计计数
			iCount =0;
			//显示连接到GPU 编号
			PRINTF (" Displays Connected to GPU #%d\n", iGPUCounter);
			PRINTF (" ----------------------------- \n");
			// 遍历显卡
			for (j = 0; j < iNumberAdapters; j++)
			{
				//如果连接总线数目与显示器总线数目相同
				if (iBusNumber == lpAdapterInfo[ j ].iBusNumber)
				{
					//获取当前显卡编号
					iAdapterIndex = lpAdapterInfo[ j ].iAdapterIndex;
					//释放原来的ALD显示信息内存
					ADL_Main_Memory_Free ((void**) &lpAdlDisplayInfo );
                    if (ADL_OK != ADL2_Display_DisplayInfo_Get(//循环知道找到有输出的显卡信息
                    	ADLContext_, 
                    	lpAdapterInfo[j].iAdapterIndex, 
                    	&iNumDisplays, 
                    	&lpAdlDisplayInfo, 
                    	0)
                    	)
						continue;
					// 遍历显示信息
					for ( k = 0; k < iNumDisplays; k++ )
					{
							// For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
							// 对于每个显示器，检查其状态。仅在已连接和映射的情况下使用显示器（IDisplayinfoValue:位0和1）
						//主要是为了找到连接的显示信息序号K
						if (( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED  ) != 
							( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED 	&
								lpAdlDisplayInfo[ k ].iDisplayInfoValue ) )
							continue;   // Skip the not connected or not mapped displays         
						// 是否找到
						iDisplayFound = 0;
						// 遍历统计
						for (l=0;l<iCount;l++)
						{
							//查找逻辑序号是否和显示序号相同
							if (lpAdlDisplayInfo[k].displayID.iDisplayLogicalIndex == iDisplayIndex[l])
							{
								//找到了显示
								iDisplayFound = 1;
								break;
							}
						}
						//没有找到显示
						if (!iDisplayFound)
						{
							//重新设置最后一个显示逻辑编号
							iDisplayIndex[iCount] = lpAdlDisplayInfo[ k ].displayID.iDisplayLogicalIndex;
							(iCount)++;//添加计数
							//输出显示逻辑显卡编号和逻辑显示编号
							PRINTF(" {%d,%d} \t",lpAdlDisplayInfo[ k ].displayID.iDisplayLogicalAdapterIndex, lpAdlDisplayInfo[ k ].displayID.iDisplayLogicalIndex);
						}	
					}
				}
				
			}
			PRINTF("\n \n");
			//设置下一个GPU的总线编号
			igpuBusIndexes[iGPUCounter++] = iBusNumber;

		}
	}

    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
	PRINTF (" --------------------------------------------------------- \n");
	PRINTF ("* if adapter index -1 means, Disabled Display\n");
	PRINTF (" --------------------------------------------------------- \n");
	return TRUE;
}
//获取GPU显示的编号索引
int getDisplayIndexesofOneGPU(
	int iCurrentAdapterIndex, //当前显卡编号 
	int* lpAdpaterIndexes, //lp显卡索引
	int* lpDisplayIndexes, //显示索引
	int* lpCount //lp计数
	)
{
	//显卡数量
	int  iNumberAdapters, 
	iNumDisplays;//显示数量
    int  iAdapterIndex;//显卡索引
  	int iBusNumber;//总线数目
	int i=0,j=0, found=0,k=0;
	LPAdapterInfo     lpAdapterInfo = NULL;
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL;
	*lpCount = 0;//lpCount=0;
	//获取显卡的信息
    if (ADL_OK != ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, -1, &iNumberAdapters, &lpAdapterInfo))
    {
        PRINTF("ADL2_Adapter_AdapterInfoX3_Get failed!\n");
        return 0;
    }
    //遍历显卡列表
	for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[i].iAdapterIndex == iCurrentAdapterIndex)
		{
			//获取当前显卡的总线数目
			iBusNumber = lpAdapterInfo[i].iBusNumber;
		}
	}
    // Repeat for all available adapters in the system
    // 对系统中所有可用的适配器重复此步骤
    for ( i = 0; i < iNumberAdapters; i++ )
    {
    	//找到总线相同的数目就执行下面的操作
		if (iBusNumber != lpAdapterInfo[ i ].iBusNumber)
			continue;
		//更改显卡缩影
		iAdapterIndex = lpAdapterInfo[ i ].iAdapterIndex;
		//释放原来的lp显示信息
		ADL_Main_Memory_Free ((void**) &lpAdlDisplayInfo );
        //如果成功获取到该显卡的显示信息就继续
        if (ADL_OK != ADL2_Display_DisplayInfo_Get(ADLContext_, lpAdapterInfo[i].iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 0))
			continue;
		//遍历显示信息
        for ( j = 0; j < iNumDisplays; j++ )
        {
				//For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
				//对于每个显示器，检查其状态。仅在已连接和映射的情况下使用显示器（IDisplayinfoValue:位0和1）
            if (( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED  ) != 
                ( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED 	&
					lpAdlDisplayInfo[ j ].iDisplayInfoValue ) )
                continue;   // Skip the not connected or not mapped displays 
            				// 跳过未连接或未映射的显示
         
			found =1;
			//根本不会进入循环，不知道为什么要用???
			//玄幻查找ipconut相同的
			for (k=0;k<*lpCount;k++)
			{
				if (lpAdlDisplayInfo[ j].displayID.iDisplayLogicalIndex == lpDisplayIndexes[k])
				{
					found = 0;
					break;
				}
			}
			if (found)
			{
				//更改显卡的逻辑显卡顺序
				lpAdpaterIndexes[*lpCount] = lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalAdapterIndex;
				lpDisplayIndexes[*lpCount] = lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalIndex;
				(*lpCount)++;//lpCount数目加加
			}			
		}
	}
    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
	return TRUE;
}
//初始化显卡，主要是确认一下信息
int CanSetPrimary(
	int iAdapterIndex,//显卡编号 
	int iCurrentPrimaryAdapterIndex //当前初始化的显卡编号
	)
{
	int  iNumberAdapters;//显卡数量
	int iBusNumber = 0, //总线数目
	iCurrentPrimaryAdapterBusNumber = 0;//当前初始化的总线数目
	int i;//顺序标号
	LPAdapterInfo     lpAdapterInfo = NULL; //显卡信息指针
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL; //显示信息指针
    //获取显卡数目和显卡信息数组
    if (ADL_OK != ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, -1, &iNumberAdapters, &lpAdapterInfo))
    {
        PRINTF("ADL2_Adapter_AdapterInfoX3_Get failed!\n");
        return 0;
    }

	//Finding Adapater Index for SLS creation.
    // Repeat for all available adapters in the system
    // 正在查找用于创建SLS的adapater索引。
    // 对系统中所有可用的适配器重复此步骤
    // 查找显示器信息中的显示器索引
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (iAdapterIndex == lpAdapterInfo[ i ].iAdapterIndex)
		{
			//更新总线数目
			iBusNumber = lpAdapterInfo[ i ].iBusNumber;
		}       
		if (iCurrentPrimaryAdapterIndex == lpAdapterInfo[ i ].iAdapterIndex)
		{
			iCurrentPrimaryAdapterBusNumber = lpAdapterInfo[ i ].iBusNumber;
		}
	}
	//释放显示器数组信息
    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
	//当前显示器总线数目与显示器列表总线数目相同
	if (iCurrentPrimaryAdapterBusNumber == iBusNumber)
	{
		return 1; //返回成功
	}

	return 0;
}



