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
			for(iCurrentTransientMode = 0; iCurrentTransientMode < iNumCreatedTransientModes; iCurrentTransientMode++)
			{
				// we need to figure out the positioning of this display in the SLS
				pTransientModes[iCurrentTransientMode].iXRes = lppCreatedTransientModes[iCurrentTransientMode].displayMode.iXRes;
				pTransientModes[iCurrentTransientMode].iYRes = lppCreatedTransientModes[iCurrentTransientMode].displayMode.iYRes;
                iRetVal = ADL2_Display_Modes_Set(ADLContext_, iAdapterIndex, -1, 1, &pTransientModes[iCurrentTransientMode]);
				break;
			}

			if (iRetVal == 0)
			{
                iRetVal = ADL2_Display_SLSMapConfig_Get(ADLContext_, iAdapterIndex, lpCreatedSLSMap.iSLSMapIndex, &lpCreatedSLSMap,
														&iNumCreatedSLSTargets, &lppCreatedSLSTargets,
														&iNumCreatedSLSNativeModes, &lppCreatedSLSNativeModes,
														&iNumCreatedBezelModes, &lppCreatedBezelModes,
														&iNumCreatedTransientModes, &lppCreatedTransientModes,
														&iNumCreatedBezelOffsets, &lppCreatedBezelOffsets,
														ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE );

				if (iNumCreatedBezelOffsets > 0)
				{
					iReferenceWidth = lppCreatedBezelOffsets[iNumCreatedBezelOffsets - 1].iDisplayWidth;
					iReferenceHeight = lppCreatedBezelOffsets[iNumCreatedBezelOffsets - 1].iDisplayHeight;

					
					//Try applying Permanently
					lpAppliedBezelOffsets = (ADLSLSOffset*) malloc(iNumCreatedSLSTargets * sizeof(ADLSLSOffset));
					//Set the initial offsets to start setting the user offsets
					for(iCurrentSLSTarget = 0; iCurrentSLSTarget < iNumCreatedSLSTargets; iCurrentSLSTarget++)
					{
						for (iCurrentBezelOffset = 0; iCurrentBezelOffset< iNumCreatedBezelOffsets; iCurrentBezelOffset++)
						{
							if (lppCreatedBezelOffsets[iCurrentBezelOffset].iBezelModeIndex == lppCreatedTransientModes[iNumCreatedTransientModes - 1].iSLSModeIndex &&
								lppCreatedBezelOffsets[iCurrentBezelOffset].displayID.iDisplayLogicalIndex == lppCreatedSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex)
							{
								lpAppliedBezelOffsets[iCurrentSLSTarget].iBezelOffsetX = lppCreatedBezelOffsets[iCurrentBezelOffset].iBezelOffsetX - (lppCreatedSLSTargets[iCurrentSLSTarget].iSLSGridPositionX * iHbezel);
								lpAppliedBezelOffsets[iCurrentSLSTarget].iBezelOffsetY = lppCreatedBezelOffsets[iCurrentBezelOffset].iBezelOffsetY - (lppCreatedSLSTargets[iCurrentSLSTarget].iSLSGridPositionY * iVbezel);

								lpAppliedBezelOffsets[iCurrentSLSTarget].iAdapterIndex = iAdapterIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].iSLSMapIndex = lpCreatedSLSMap.iSLSMapIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayLogicalAdapterIndex = iAdapterIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayLogicalIndex = lppCreatedSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayPhysicalAdapterIndex = iAdapterIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].displayID.iDisplayPhysicalIndex = lppCreatedSLSTargets[iCurrentSLSTarget].displayTarget.displayID.iDisplayLogicalIndex;
								lpAppliedBezelOffsets[iCurrentSLSTarget].iDisplayWidth = iReferenceWidth;
								lpAppliedBezelOffsets[iCurrentSLSTarget].iDisplayHeight = iReferenceHeight;
							}
						}
					}

					lpCreatedSLSMap.iSLSMapMask = 0;
					lpCreatedSLSMap.iSLSMapValue = iSLSMapValue;
                    iRetVal = ADL2_Display_BezelOffset_Set(ADLContext_, iAdapterIndex, lpCreatedSLSMap.iSLSMapIndex, iNumCreatedSLSTargets, lpAppliedBezelOffsets, lpCreatedSLSMap, ADL_DISPLAY_BEZELOFFSET_COMMIT | ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE);
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

int setResolution(int iAdapterIndex, int iXRes, int iYRes)
{
	int iNumModes, iNumModesPriv, iRetVal;
	ADLMode *pModes = NULL, *lppModesPriv = NULL;
	int iResfound = 0,i;
	if (iAdapterIndex != -1)
	{
		// Check if the mode is in the possible mode list.
        iRetVal = ADL2_Display_PossibleMode_Get(ADLContext_, iAdapterIndex, &iNumModesPriv, &lppModesPriv);
		if (NULL != lppModesPriv)
		{
			for ( i=0;i<iNumModesPriv;i++)
			{
				if( lppModesPriv[i].iXRes == iXRes && lppModesPriv[i].iYRes == iYRes)
				{			
					iResfound=1;
					break;
				}
			}			
		}

		if (iResfound)
		{
            iRetVal = ADL2_Display_Modes_Get(ADLContext_, iAdapterIndex, -1, &iNumModes, &pModes);
			if (iRetVal == ADL_OK)
			{
				pModes[0].iXRes = iXRes;
				pModes[0].iYRes = iYRes;
                iRetVal = ADL2_Display_Modes_Set(ADLContext_, iAdapterIndex, -1, 1, &pModes[0]);
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
	ADL_Main_Memory_Free((void**)&pModes);
	ADL_Main_Memory_Free((void**)&lppModesPriv);
	return TRUE;
}

int setPrimaryAdapter(int iAdapterIndex)
{
	int iCurrentAdapterIndex = 0;
    if (ADL_OK == ADL2_Adapter_Primary_Get(ADLContext_, &iCurrentAdapterIndex))
	{
		if (iCurrentAdapterIndex != iAdapterIndex && CanSetPrimary(iAdapterIndex ,iCurrentAdapterIndex))
		{
            ADL2_Adapter_Primary_Set(ADLContext_, iAdapterIndex);
		}
	}
	return TRUE;
}

int setAdapterDisplaysToDisable(int iAdapterIndex)
{
	int iAdapterIndexes[6];
	int iDisplayIndexes[6];
	int iCount=0;
	int active;
	int i;
	getDisplayIndexesofOneGPU(iAdapterIndex, &iAdapterIndexes[0], &iDisplayIndexes[0], &iCount);

	for (i=0;i<iCount;i++)
	{
		if (iAdapterIndex != iAdapterIndexes[i] && iAdapterIndexes[i] != -1)
		{
            ADL2_Adapter_Active_Set(ADLContext_, iAdapterIndexes[i], 0, &active);
		}
	}
	return TRUE;
}

int setAdapterDisplaysToClone(int iAdapterIndex, int iDisplayIndexes[], int iDisplaysCount)
{
	int i;
	ADLDisplayMap mapArray;
    ADLDisplayTarget* pDisplayTargets = NULL;


	int iNumModes;
	ADLMode *pModes = NULL;
	pDisplayTargets = (ADLDisplayTarget*) malloc(iDisplaysCount * sizeof(ADLDisplayTarget));
	memset(&(mapArray), 0, sizeof(ADLDisplayMap));
	mapArray.iNumDisplayTarget = iDisplaysCount;
	mapArray.iDisplayMapIndex = 0;
	
    ADL2_Display_Modes_Get(ADLContext_,iAdapterIndex, -1, &iNumModes, &pModes);
	memset(&(mapArray.displayMode), 0, sizeof(ADLMode));
	mapArray.displayMode.iAdapterIndex = iAdapterIndex;
	mapArray.displayMode.iModeFlag = pModes[0].iModeFlag;
	mapArray.displayMode.iOrientation = pModes[0].iOrientation;
	mapArray.displayMode.fRefreshRate = pModes[0].fRefreshRate;
	mapArray.displayMode.iColourDepth = pModes[0].iColourDepth;
	mapArray.displayMode.iXPos = pModes[0].iXPos;
	mapArray.displayMode.iYPos = pModes[0].iYPos;
	mapArray.displayMode.iXRes = pModes[0].iXRes;
	mapArray.displayMode.iYRes = pModes[0].iYRes;

	for (i=0;i<iDisplaysCount;i++)
	{
		memset(&(pDisplayTargets[i].displayID), 0, sizeof(ADLDisplayID));
		memset(&(pDisplayTargets[i]), 0, sizeof(ADLDisplayTarget));
		pDisplayTargets[i].displayID.iDisplayLogicalAdapterIndex = iAdapterIndex;
		pDisplayTargets[i].displayID.iDisplayLogicalIndex = iDisplayIndexes[i];
		pDisplayTargets[i].iDisplayMapIndex = 0;
	}
    ADL2_Display_DisplayMapConfig_Set(ADLContext_, iAdapterIndex, 1, &mapArray, iDisplaysCount, pDisplayTargets);

	ADL_Main_Memory_Free((void**)&pDisplayTargets);
	return TRUE;
}

int disableAdapterEyefinityMapping(int iAdapterIndex)
{
	int iNumDisplayTarget = 0;
	ADLDisplayTarget *lpDisplayTarget = NULL;
	int iNumDisplayMap = 0;
	ADLDisplayMap *lpDisplayMap = NULL;
	int iSLSMapIndex = -1,iRetVal=-1;

    iRetVal = ADL2_Display_DisplayMapConfig_Get(ADLContext_, iAdapterIndex,
														&iNumDisplayMap, &lpDisplayMap, 
														&iNumDisplayTarget, &lpDisplayTarget, 
														ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO );
	if (ADL_OK == iRetVal&& 1<iNumDisplayTarget)
	{
        iRetVal = ADL2_Display_SLSMapIndex_Get(ADLContext_, iAdapterIndex,
			iNumDisplayTarget,
			lpDisplayTarget,
			&iSLSMapIndex);

		if (ADL_OK == iRetVal && iSLSMapIndex != -1)
		{
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
	ADL_Main_Memory_Free((void**)&lpDisplayMap);
	return TRUE;
}

int printDisplayIndexes()
{
	int  iNumberAdapters=0, iNumDisplays;
    int  iAdapterIndex;
    int  iDisplayIndex[6];
	int iBusNumber;
	int i=0,j=0,k=0,l=0, iGPUfound=0, iDisplayFound=0, iGPUIndex=0,iCount=0, iGPUCounter=0;
	LPAdapterInfo     lpAdapterInfo = NULL;
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL;
    int igpuBusIndexes[4];

	// Obtain the number of adapters for the system
    if (ADL_OK != ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, -1, &iNumberAdapters, &lpAdapterInfo))
	{
	       PRINTF("ADL2_Adapter_AdapterInfoX3_Get failed!\n");
		   return 0;
	}
    
	for (iGPUIndex = 0; iGPUIndex < 4; iGPUIndex++)
	{
		igpuBusIndexes[iGPUIndex] = -1;
	}

	PRINTF (" Adapter and Displays Indexes <AdapterIndex, DisplayIndex> \n");
	PRINTF (" --------------------------------------------------------- \n");
    // Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		iGPUfound = 0;
		iBusNumber = lpAdapterInfo[ i ].iBusNumber;
		for (iGPUIndex = 0; iGPUIndex < 4; iGPUIndex++)
		{
			if(igpuBusIndexes[iGPUIndex] != -1 && igpuBusIndexes[iGPUIndex] == iBusNumber)
			{
				iGPUfound = 1;
				break;
			}
		}

		if (!iGPUfound)
		{
			for (l=0;l<6;l++)
			{
				iDisplayIndex[l] = -1;				
			}
			iCount =0;
			PRINTF (" Displays Connected to GPU #%d\n", iGPUCounter);
			PRINTF (" ----------------------------- \n");
			for (j = 0; j < iNumberAdapters; j++)
			{
				if (iBusNumber == lpAdapterInfo[ j ].iBusNumber)
				{
					iAdapterIndex = lpAdapterInfo[ j ].iAdapterIndex;
					ADL_Main_Memory_Free ((void**) &lpAdlDisplayInfo );
                    if (ADL_OK != ADL2_Display_DisplayInfo_Get(ADLContext_, lpAdapterInfo[j].iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 0))
						continue;

					for ( k = 0; k < iNumDisplays; k++ )
					{
							//For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
						if (( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED  ) != 
							( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED 	&
								lpAdlDisplayInfo[ k ].iDisplayInfoValue ) )
							continue;   // Skip the not connected or not mapped displays         
					
						iDisplayFound = 0;
						for (l=0;l<iCount;l++)
						{
							if (lpAdlDisplayInfo[k].displayID.iDisplayLogicalIndex == iDisplayIndex[l])
							{
								iDisplayFound = 1;
								break;
							}
						}
						if (!iDisplayFound)
						{
							iDisplayIndex[iCount] = lpAdlDisplayInfo[ k ].displayID.iDisplayLogicalIndex;
							(iCount)++;
							PRINTF(" {%d,%d} \t",lpAdlDisplayInfo[ k ].displayID.iDisplayLogicalAdapterIndex, lpAdlDisplayInfo[ k ].displayID.iDisplayLogicalIndex);
						}	
					}
				}
				
			}
			PRINTF("\n \n");
			igpuBusIndexes[iGPUCounter++] = iBusNumber;

		}
	}

    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
	PRINTF (" --------------------------------------------------------- \n");
	PRINTF ("* if adapter index -1 means, Disabled Display\n");
	PRINTF (" --------------------------------------------------------- \n");
	return TRUE;
}

int getDisplayIndexesofOneGPU(int iCurrentAdapterIndex, int* lpAdpaterIndexes, int* lpDisplayIndexes, int* lpCount )
{
	int  iNumberAdapters, iNumDisplays;
    int  iAdapterIndex;
  	int iBusNumber;
	int i=0,j=0, found=0,k=0;
	LPAdapterInfo     lpAdapterInfo = NULL;
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL;
	*lpCount = 0;

    if (ADL_OK != ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, -1, &iNumberAdapters, &lpAdapterInfo))
    {
        PRINTF("ADL2_Adapter_AdapterInfoX3_Get failed!\n");
        return 0;
    }

	for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (lpAdapterInfo[i].iAdapterIndex == iCurrentAdapterIndex)
		{
			iBusNumber = lpAdapterInfo[i].iBusNumber;
		}
	}
    // Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (iBusNumber != lpAdapterInfo[ i ].iBusNumber)
			continue;

		iAdapterIndex = lpAdapterInfo[ i ].iAdapterIndex;
		ADL_Main_Memory_Free ((void**) &lpAdlDisplayInfo );
        if (ADL_OK != ADL2_Display_DisplayInfo_Get(ADLContext_, lpAdapterInfo[i].iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 0))
			continue;

        for ( j = 0; j < iNumDisplays; j++ )
        {
				//For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
            if (( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED  ) != 
                ( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED 	&
					lpAdlDisplayInfo[ j ].iDisplayInfoValue ) )
                continue;   // Skip the not connected or not mapped displays
         
			found =1;

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
				lpAdpaterIndexes[*lpCount] = lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalAdapterIndex;
				lpDisplayIndexes[*lpCount] = lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalIndex;
				(*lpCount)++;
			}			
		}
	}
    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
	return TRUE;
}

int CanSetPrimary(int iAdapterIndex, int iCurrentPrimaryAdapterIndex)
{
	int  iNumberAdapters;
	int iBusNumber = 0, iCurrentPrimaryAdapterBusNumber = 0;
	int i;
	LPAdapterInfo     lpAdapterInfo = NULL;
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL;

    if (ADL_OK != ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, -1, &iNumberAdapters, &lpAdapterInfo))
    {
        PRINTF("ADL2_Adapter_AdapterInfoX3_Get failed!\n");
        return 0;
    }

	//Finding Adapater Index for SLS creation.
    // Repeat for all available adapters in the system
    for ( i = 0; i < iNumberAdapters; i++ )
    {
		if (iAdapterIndex == lpAdapterInfo[ i ].iAdapterIndex)
		{
			iBusNumber = lpAdapterInfo[ i ].iBusNumber;
		}       
		if (iCurrentPrimaryAdapterIndex == lpAdapterInfo[ i ].iAdapterIndex)
		{
			iCurrentPrimaryAdapterBusNumber = lpAdapterInfo[ i ].iBusNumber;
		}
	}

    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
	if (iCurrentPrimaryAdapterBusNumber == iBusNumber)
	{
		return 1;
	}

	return 0;
}



