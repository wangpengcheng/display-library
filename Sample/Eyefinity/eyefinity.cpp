///
///  Copyright (c) 2008 - 2018 Advanced Micro Devices, Inc.

///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file eyefinity.c

#include <windows.h>
#include <algorithm>
#include "eyefinity.h"

//给printf别名
#define PRINTF printf

//定义获取信息相关函数
typedef int(*ADL2_ADAPTER_ADAPTERINFOX3_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* numAdapters, AdapterInfo** lppAdapterInfo);
//
typedef int(*ADL2_DISPLAY_DISPLAYMAPCONFIG_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumDisplayMap, ADLDisplayMap**  lppDisplayMap,
    int* lpNumDisplayTarget, ADLDisplayTarget** lppDisplayTarget, int iOptions);

typedef int(*ADL2_DISPLAY_SLSMAPINDEXLIST_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumSLSMapIndexList, int** lppSLSMapIndexList,
    int iOptions);

typedef int(*ADL2_DISPLAY_SLSMAPCONFIGX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iSLSMapIndex, ADLSLSMap* lpSLSMap, int*                                                            lpNumSLSTarget, ADLSLSTarget** lppSLSTarget, int* lpNumStandardMode, ADLSLSMode** lppStandardMode,
    int* lpNumStandardModeOffsets, ADLSLSOffset** lppStandardModeOffsets,
    int* lpNumBezelMode, ADLBezelTransientMode** lppBezelMode, int* lpNumTransientMode,
    ADLBezelTransientMode** lppTransientMode, int* lpNumSLSOffset, ADLSLSOffset** lppSLSOffset, int iOption);

typedef int(*ADL2_MAIN_CONTROL_DESTROY) (ADL_CONTEXT_HANDLE);
typedef int(*ADL2_MAIN_CONTROLX2_CREATE)  (ADL_MAIN_MALLOC_CALLBACK, int iEnumConnectedAdapter_, ADL_CONTEXT_HANDLE* context_, ADLThreadingModel);

// ------实例化函数指针 start------
ADL2_ADAPTER_ADAPTERINFOX3_GET    ADL2_Adapter_AdapterInfoX3_Get = NULL;
ADL2_DISPLAY_DISPLAYMAPCONFIG_GET ADL2_Display_DisplayMapConfig_Get = NULL;
ADL2_DISPLAY_SLSMAPINDEXLIST_GET  ADL2_Display_SLSMapIndexList_Get = NULL;
ADL2_DISPLAY_SLSMAPCONFIGX2_GET   ADL2_Display_SLSMapConfigX2_Get = NULL;
ADL2_MAIN_CONTROLX2_CREATE        ADL2_Main_ControlX2_Create = NULL;
ADL2_MAIN_CONTROL_DESTROY         ADL2_Main_Control_Destroy = NULL;
//-----实例化函数指针 end------

//设置静态上下文
static ADL_CONTEXT_HANDLE ADLContext_ = NULL;



//显示匹配
static bool DisplaysMatch(const ADLDisplayID& one_, const ADLDisplayID& other_);
//查找展示页面
static int  FindSLSTarget(ADLDisplayID displayID_, int numDisplays_, const ADLSLSTarget* slsTargets_);
//
static int  GpuBDF(const int& busNo_, const int& devNo_, const int& funcNo_) { return ((busNo_ & 0xFF) << 8) | ((devNo_ & 0x1F) << 3) | (funcNo_ & 0x07); }
//获取初始转接器ID
static int  GetPrimaryAdpaterId(char displayName[]);
//获取宽度
static int  GetWidth(ADLMode& oneMode_);
//获取高度
static int  GetHeight(ADLMode& oneMode_);
//分配内存
static void* __stdcall ADL_Main_Memory_Alloc(int iSize_)
{
    void* lpBuffer = malloc(iSize_);
    return lpBuffer;
}

static void __stdcall ADL_Main_Memory_Free(void** lpBuffer_)
{
    if (NULL != lpBuffer_ && NULL != *lpBuffer_) {
        free(*lpBuffer_);
        *lpBuffer_ = NULL;
    }
}
//获取上下文
ADL_CONTEXT_HANDLE GetADLContext()
{
    return ADLContext_;
}
//初始化ADL
bool InitADL()
{
    // Load the ADL dll
    HINSTANCE hDLL = LoadLibrary(TEXT("atiadlxx.dll"));
    if (hDLL == NULL) {
        // A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        hDLL = LoadLibrary(TEXT("atiadlxy.dll"));    
        if (hDLL == NULL) {
            PRINTF("Failed to load ADL library\n");
            return false;
        }
    }

    // Get & validate function pointers
    // 获取函数指针---函数指针实例化    
    ADL2_Adapter_AdapterInfoX3_Get = (ADL2_ADAPTER_ADAPTERINFOX3_GET)GetProcAddress(hDLL, "ADL2_Adapter_AdapterInfoX3_Get");
    ADL2_Display_DisplayMapConfig_Get = (ADL2_DISPLAY_DISPLAYMAPCONFIG_GET)GetProcAddress(hDLL, "ADL2_Display_DisplayMapConfig_Get");
    ADL2_Display_SLSMapIndexList_Get = (ADL2_DISPLAY_SLSMAPINDEXLIST_GET)GetProcAddress(hDLL, "ADL2_Display_SLSMapIndexList_Get");
    ADL2_Display_SLSMapConfigX2_Get = (ADL2_DISPLAY_SLSMAPCONFIGX2_GET)GetProcAddress(hDLL, "ADL2_Display_SLSMapConfigX2_Get");
    ADL2_Main_ControlX2_Create = (ADL2_MAIN_CONTROLX2_CREATE)GetProcAddress(hDLL, "ADL2_Main_ControlX2_Create");
    ADL2_Main_Control_Destroy = (ADL2_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL2_Main_Control_Destroy");
    //获取函数指针--结束
    if (NULL == ADL2_Adapter_AdapterInfoX3_Get ||
        NULL == ADL2_Display_DisplayMapConfig_Get ||
        NULL == ADL2_Display_SLSMapIndexList_Get ||
        NULL == ADL2_Display_SLSMapConfigX2_Get ||
        NULL == ADL2_Main_ControlX2_Create) {
        PRINTF("Failed to get ADL function pointers\n");
        return false;
    }
    //创建上下文
    if (ADL_OK != ADL2_Main_ControlX2_Create(ADL_Main_Memory_Alloc, 1, &ADLContext_, ADL_THREADING_LOCKED)) {
        PRINTF("ADL_Main_Control_Create() failed\n");
        return false;
    }
    return true;
}
//销毁ADL
void DestoryADL()
{
    if (NULL != ADL2_Main_Control_Destroy)//先检查函数指针是否为空，是则
        ADL2_Main_Control_Destroy(ADLContext_);
}
//ATI 获取配置信息
int atiEyefinityGetConfigInfo(char OSDisplayName[], EyefinityInfoStruct *lpEyefinityInfo, int *lpNumDisplaysInfo, DisplayInfoStruct **lppDisplaysInfo)
{

    LPCSTR EnvironmentVariable = "ADL_4KWORKAROUND_CANCEL";
    // This is a temporary workaround to enable SLS.
    // Set this variable to any value.
    SetEnvironmentVariable(EnvironmentVariable, "TRUE");

    if (NULL == lpEyefinityInfo ||
        NULL == lpNumDisplaysInfo ||
        NULL == lppDisplaysInfo) {
        return FALSE;
    }
    //初始化ADL
    if (!InitADL()) {
        PRINTF("Failed to initiliaze ADL Library!\n");
        return FALSE;
    }
    //初始化指针
    int primaryIndex = GetPrimaryAdpaterId(OSDisplayName);
    //
    if (primaryIndex < 0) {
        PRINTF("Failed to get primary adapter id!\n");
        return FALSE;
    }
    //显示向量
    vector<TopologyDisplay> displays(0);
    //重设向量大小为24
    displays.reserve(24);
    //
    int numDesktops = 0, numDisplays = 0;
    //显示图
    ADLDisplayMap*    adlDesktops = NULL;
    //显示标签
    ADLDisplayTarget* adlDisplays = NULL;
    //获取显示的位置
    int adlRet = ADL2_Display_DisplayMapConfig_Get(GetADLContext(), primaryIndex, &numDesktops, &adlDesktops, &numDisplays, \
        &adlDisplays, ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO);
    //  This function retrieves the current display map configurations, including the controllers and adapters mapped to each display.
	//  [in] 	context,: 	Client's ADL context handle ADL_CONTEXT_HANDLE obtained from ADL2_Main_Control_Create.
	//	[in] 	iAdapterIndex 	The ADL index handle of the desired adapter. A value of -1 returns all display configurations for the system across multiple GPUs.
	//	[out] 	lpNumDisplayMap 	The pointer to the number of retrieved display maps.
	//	[out] 	lppDisplayMap 	The pointer to the pointer to the display manner information. Refer to the ADLDisplayMap structure for more information.
	//	[out] 	lpNumDisplayTarget 	The pointer to the display target sets retrieved.
	//	[out] 	lppDisplayTarget 	The pointer to the pointer to the display target buffer. Refer to the ADLDisplayTarget structure for more information.
	//	[in] 	iOptions 	The function option. ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO.
    //
    if (ADL_OK == adlRet || ADL_OK_WARNING == adlRet) {
        //循环输出相关信息
        for (int deskIdx = 0; deskIdx < numDesktops; deskIdx++) {
            // 
            ADLDisplayMap oneAdlDesktop = adlDesktops[deskIdx];//获取桌面信息
            ADLDisplayID preferredDisplay{ 0 };//都初始化为0

            //If discover a rotation: swap X and Y in the mode
			//如果旋转成垂直，则直接交换想x，y
            if (90 == oneAdlDesktop.displayMode.iOrientation || 270 == oneAdlDesktop.displayMode.iOrientation) {
                int oldXRes = oneAdlDesktop.displayMode.iXRes;
                oneAdlDesktop.displayMode.iXRes = oneAdlDesktop.displayMode.iYRes;
                oneAdlDesktop.displayMode.iYRes = oldXRes;
            }

            // By default non-SLS; one row, one column
			//默认情况下，一行，一列
            int rows = 1, cols = 1;
            // By default SLsMapIndex is -1 and SLS Mode is fill
			// 默认下标为1
            int slsMapIndex = -1, slsMode = ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FILL;
			//设置转接器初始值为0
            int numAdapters = 0;
			//转接器所有信息指针
            AdapterInfo* allAdapterInfo = NULL;
            //获取所有转接器信息
			ADL2_Adapter_AdapterInfoX3_Get(GetADLContext(), -1, &numAdapters, &allAdapterInfo);
            //如果指针未空，直接设置转接器为0
			if (NULL == allAdapterInfo)
                numAdapters = 0;
			//遍历显示信息
            for (int dispIdx = 0, foundDisplays = 0; dispIdx < numDisplays; dispIdx++) {
				//获取显示标签信息
                ADLDisplayTarget oneAdlDisplay = adlDisplays[dispIdx];
                //如果显示模式与当前的桌面模式相匹配
				if (oneAdlDisplay.iDisplayMapIndex == oneAdlDesktop.iDisplayMapIndex) {
					//如果初始索引与物理索引相同
                    if (primaryIndex == oneAdlDisplay.displayID.iDisplayPhysicalAdapterIndex) {
                        //add a display in list. For SLS this info will be updated later
						//在显示列表中添加新的显示画面
                        displays.push_back(TopologyDisplay(oneAdlDisplay.displayID, 0,
                            oneAdlDesktop.displayMode.iXRes, oneAdlDesktop.displayMode.iYRes, //size
                            0, 0,	 //offset in desktop
                            0, 0)); //grid location (0-based)

                                    // count it and bail out of we found enough
                        foundDisplays++;//统计显示数目
						//直到所有的都添加完毕
                        if (foundDisplays == oneAdlDesktop.iNumDisplayTarget)
                            break;
                    }
                }
            }
			//如果显示数量大于1并且显示队列大于0
            if (numDisplays > 1 && displays.size() > 0) {
				//获取第一个显示
                TopologyDisplay firstDisplay = displays[0];
				//目标显示
                preferredDisplay = firstDisplay.DisplayID();
				//更改整体显示信息
                lpEyefinityInfo->iSLSWidth = adlDesktops[0].displayMode.iXRes;
                lpEyefinityInfo->iSLSHeight = adlDesktops[0].displayMode.iYRes;
				//设置总高度
                int numSLSMaps = 0;
				//设置Map
                int* slsMapIDxs = NULL;

                if (ADL_OK == ADL2_Display_SLSMapIndexList_Get(GetADLContext(), primaryIndex, &numSLSMaps, &slsMapIDxs, ADL_DISPLAY_SLSMAPINDEXLIST_OPTION_ACTIVE)) {

                    // Declare data describing the SLS before the loop
                    ADLSLSMap slsMap;
                    int numSLSTargets = 0;
                    ADLSLSTarget* slsTargets = NULL;
					//初始化标准模式，标准偏移，遮挡模式，暂停模式，遮挡偏移
                    int numStandardModes = 0, numStandardModesOffsets = 0, numBezelModes = 0, numTransientModes = 0, numBezelModesOffsets = 0;
					//初始化标准模式指针
                    ADLSLSMode* standardModes = NULL;
                    //查找相关操作
					ADLBezelTransientMode *bezelModes = NULL, *transientModes = NULL;
                    //设置标准模式的偏移
                    ADLSLSOffset *standardModesOffsets = NULL, *bezelTransientModesOffsets = NULL;
                    //遍历每个虚拟映射
                    for (int slsMapIdx = 0; slsMapIdx < numSLSMaps; slsMapIdx++) {
                        //SLS加锁
                        bool isActiveSLS = false;
                        // We got the SLS OK and it has the same number of displays as the current desktop
                        // This function retrieves an SLS configuration, which includes the, SLS map, SLS targets, SLS standard modes, bezel modes or a transient mode, and offsets
                        if (ADL_OK == ADL2_Display_SLSMapConfigX2_Get(GetADLContext(), primaryIndex, slsMapIDxs[slsMapIdx], &slsMap, &numSLSTargets, &slsTargets,
                            &numStandardModes, &standardModes,
                            &numStandardModesOffsets, &standardModesOffsets,
                            &numBezelModes, &bezelModes, &numTransientModes,
                            &transientModes, &numBezelModesOffsets,
                            &bezelTransientModesOffsets, ADL_DISPLAY_SLSMAPCONFIG_GET_OPTION_RELATIVETO_CURRENTANGLE)
                            && numSLSTargets == oneAdlDesktop.iNumDisplayTarget) {
                            //获取行
                            cols = slsMap.grid.iSLSGridColumn;
                            //获取列
                            rows = slsMap.grid.iSLSGridRow;
                            //更改相关信息
                            lpEyefinityInfo->iSLSGridWidth = cols;
                            lpEyefinityInfo->iSLSGridHeight = rows;
                            //设置显示匹配为true
                            bool displaysMatch = true;
                            // Match slsTargets and the ones in this desktop, which are in the displays vector
                            // 匹配SLS标签
                            for (int j = 0; j < numSLSTargets; j++) {
                                // find the SLS display into the display map allocation; find_if using lambda (perf: O(n))
                                auto disp = std::find_if(displays.begin(), displays.end(),
                                    [&](const TopologyDisplay& oneDisplay) { return (DisplaysMatch(oneDisplay.DisplayID(), slsTargets[j].displayTarget.displayID)); });
                                //如果是尾部指针则直接失效
                                if (disp == displays.end()) {
                                    displaysMatch = false;
                                    break;
                                }
                            }

                            // Found the SLS for this desktop; see if it is active by checking if current mode is an SLS one
                            // 确认模型是否为 SLS中的一个
                            if (displaysMatch) {//在模型中开始遍历
                                //开始遍历
                                for (int slsModeIdx = 0; slsModeIdx < numStandardModes; slsModeIdx++) {
                                    if (standardModes[slsModeIdx].displayMode.iXRes == GetWidth(oneAdlDesktop.displayMode) &&
                                        standardModes[slsModeIdx].displayMode.iYRes == GetHeight(oneAdlDesktop.displayMode)) {
                                        isActiveSLS = true;
                                        // Ditch the displays and add new ones from standardModesOffsets for this standard mode for each SLS display
                                        //清除显示列表
                                        displays.clear();
                                        // 开始设置偏移
                                        for (int oneModeOffset = 0, foundDisplays = 0; oneModeOffset < numStandardModesOffsets; oneModeOffset++) {
                                            // this is offset for the matched mode												
                                            // 对于匹配的模型设置偏移
                                            if (standardModesOffsets[oneModeOffset].iBezelModeIndex == standardModes[slsModeIdx].iSLSModeIndex) {
                                                //查找标签
                                                int index = FindSLSTarget(standardModesOffsets[oneModeOffset].displayID, numSLSTargets, slsTargets);
                                                //设置旋转角
                                                int angle = (index != -1) ? slsTargets[index].viewSize.iOrientation : 0;
                                                //设置行
                                                int row = (index != -1) ? slsTargets[index].iSLSGridPositionY : 0;
                                                //设置列
                                                int col = (index != -1) ? slsTargets[index].iSLSGridPositionX : 0;
                                                //在列表中加入显示元素
                                                displays.push_back(TopologyDisplay(standardModesOffsets[oneModeOffset].displayID, angle,
                                                    standardModesOffsets[oneModeOffset].iDisplayWidth, standardModesOffsets[oneModeOffset].iDisplayHeight,
                                                    standardModesOffsets[oneModeOffset].iBezelOffsetX, standardModesOffsets[oneModeOffset].iBezelOffsetY,
                                                    row, col));//increase by one

                                                // count it and bail if we found enough displays
                                                // 统计总共的数量
                                                foundDisplays++;
                                                if (foundDisplays == numSLSTargets)
                                                    break;
                                            }
                                        }
                                        break;
                                    }
                                }
                                //是否激活SLS,在线路中显示
                                if (!isActiveSLS) {
                                    for (int slsModeIdx = 0; slsModeIdx < numBezelModes; slsModeIdx++) {
                                        if (bezelModes[slsModeIdx].displayMode.iXRes == GetWidth(oneAdlDesktop.displayMode) &&
                                            bezelModes[slsModeIdx].displayMode.iYRes == GetHeight(oneAdlDesktop.displayMode)) {

                                            lpEyefinityInfo->iBezelCompensatedDisplay = TRUE;
                                            isActiveSLS = true;
                                            displays.clear();

                                            for (int oneModeOffset = 0, foundDisplays = 0; oneModeOffset < numBezelModesOffsets; oneModeOffset++) {
                                                if (bezelTransientModesOffsets[oneModeOffset].iBezelModeIndex == bezelModes[slsModeIdx].iSLSModeIndex) {
                                                    int index = FindSLSTarget(bezelTransientModesOffsets[oneModeOffset].displayID, numSLSTargets, slsTargets);
                                                    int angle = (index != -1) ? slsTargets[index].viewSize.iOrientation : 0;
                                                    int row = (index != -1) ? slsTargets[index].iSLSGridPositionY : 0;
                                                    int col = (index != -1) ? slsTargets[index].iSLSGridPositionX : 0;

                                                    displays.push_back(TopologyDisplay(bezelTransientModesOffsets[oneModeOffset].displayID, angle,
                                                        bezelTransientModesOffsets[oneModeOffset].iDisplayWidth, bezelTransientModesOffsets[oneModeOffset].iDisplayHeight,
                                                        bezelTransientModesOffsets[oneModeOffset].iBezelOffsetX, bezelTransientModesOffsets[oneModeOffset].iBezelOffsetY,
                                                        row, col));

                                                    foundDisplays++;
                                                    if (foundDisplays == numSLSTargets)
                                                        break;
                                                }
                                            }
                                            // Found we are on bezel SLS mode
                                            break;
                                        }
                                    }
                                }

                                if (!isActiveSLS) {
                                    for (int slsModeIdx = 0; slsModeIdx < numTransientModes; slsModeIdx++) {
                                        if (transientModes[slsModeIdx].displayMode.iXRes == GetWidth(oneAdlDesktop.displayMode) &&
                                            transientModes[slsModeIdx].displayMode.iYRes == GetHeight(oneAdlDesktop.displayMode)) {

                                            isActiveSLS = true;
                                            displays.clear();

                                            for (int oneModeOffset = 0, foundDisplays = 0; oneModeOffset < numBezelModesOffsets; oneModeOffset++) {
                                                // this is offset for the matched mode
                                                if (bezelTransientModesOffsets[oneModeOffset].iBezelModeIndex == transientModes[slsModeIdx].iSLSModeIndex)
                                                {
                                                    int index = FindSLSTarget(bezelTransientModesOffsets[oneModeOffset].displayID, numSLSTargets, slsTargets);
                                                    int angle = (index != -1) ? slsTargets[index].viewSize.iOrientation : 0;
                                                    int row = (index != -1) ? slsTargets[index].iSLSGridPositionY : 0;
                                                    int col = (index != -1) ? slsTargets[index].iSLSGridPositionX : 0;

                                                    displays.push_back(TopologyDisplay(bezelTransientModesOffsets[oneModeOffset].displayID, angle,
                                                        bezelTransientModesOffsets[oneModeOffset].iDisplayWidth, bezelTransientModesOffsets[oneModeOffset].iDisplayHeight,
                                                        bezelTransientModesOffsets[oneModeOffset].iBezelOffsetX, bezelTransientModesOffsets[oneModeOffset].iBezelOffsetY,
                                                        row, col));

                                                    foundDisplays++;
                                                    if (foundDisplays == numSLSTargets)
                                                        break;
                                                }
                                            }
                                            // Found we are on SLS transient mode
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        //释放各种内存
                        ADL_Main_Memory_Free((void**)&slsTargets);
                        ADL_Main_Memory_Free((void**)&standardModes);
                        ADL_Main_Memory_Free((void**)&bezelModes);
                        ADL_Main_Memory_Free((void**)&transientModes);
                        ADL_Main_Memory_Free((void**)&standardModesOffsets);
                        ADL_Main_Memory_Free((void**)&bezelTransientModesOffsets);

                        if (isActiveSLS) {
                            lpEyefinityInfo->iSLSActive = TRUE;
                            slsMapIndex = slsMapIDxs[slsMapIdx];
                            slsMode = slsMap.iSLSMapValue;
                            break;
                        }
                    }
                }
                ADL_Main_Memory_Free((void**)&slsMapIDxs);
            }
        }
    }

    // Fill out the eyefinity display info
    // 输出显示信息
    if (displays.size() >= 1) {
        size_t numDisplay = displays.size();
         if (*lppDisplaysInfo == NULL) {
            *lppDisplaysInfo = (DisplayInfoStruct*)malloc(sizeof(DisplayInfoStruct) * numDisplay);
            if (NULL == *lppDisplaysInfo) {
                PRINTF("ppDisplaysInfo allocation failed\n");
            }
            memset(*lppDisplaysInfo, '\0', sizeof(DisplayInfoStruct) * numDisplay);
         }
         *lpNumDisplaysInfo = (int)displays.size();
         
         for (int i = 0; i < displays.size(); i ++) {         
             DisplayInfoStruct *lpDisplaysInfo = NULL;
             lpDisplaysInfo = &((*lppDisplaysInfo)[i]);

             auto disp = displays[i];
             lpDisplaysInfo->iGridXCoord = disp.Col();
             lpDisplaysInfo->iGridYCoord = disp.Row();

             lpDisplaysInfo->displayRect.iXOffset = disp.Left();
             lpDisplaysInfo->displayRect.iYOffset = disp.Top();             
             lpDisplaysInfo->displayRect.iWidth = disp.Width();
             lpDisplaysInfo->displayRect.iHeight = disp.Height();

             lpDisplaysInfo->displayRectVisible = lpDisplaysInfo->displayRect;
             lpDisplaysInfo->iPreferredDisplay = (i == 0) ? TRUE : FALSE;
         }
    }
    DestoryADL();
    return TRUE;
}
//获取ADLMode的宽度
int GetWidth(ADLMode& oneMode_)
{
    return (90 == oneMode_.iOrientation || 270 == oneMode_.iOrientation) ? oneMode_.iYRes : oneMode_.iXRes;
}
//获取ADLMode的高度
int GetHeight(ADLMode& oneMode_)
{
    return (90 == oneMode_.iOrientation || 270 == oneMode_.iOrientation) ? oneMode_.iXRes : oneMode_.iYRes;
}
//查找SLS标签的
int FindSLSTarget(ADLDisplayID displayID_, int numDisplays_, const ADLSLSTarget* slsTargets_)
{
    int index = -1;
    if (NULL != slsTargets_) {
        for (int i = 0; i < numDisplays_; i++) {
            if (DisplaysMatch(slsTargets_[i].displayTarget.displayID, displayID_)) {
                index = i;
                break;
            }
        }
    }
    return index;
}
//显示匹配，找寻对应的ADLDisplayID
bool DisplaysMatch(const ADLDisplayID& one_, const ADLDisplayID& other_)
{
    bool match = (-1 != one_.iDisplayLogicalIndex && -1 != other_.iDisplayLogicalIndex) ? (one_.iDisplayLogicalIndex == other_.iDisplayLogicalIndex) :
        (-1 == one_.iDisplayLogicalIndex && -1 != other_.iDisplayLogicalIndex) ? (one_.iDisplayPhysicalIndex == other_.iDisplayLogicalIndex) :
        (-1 != one_.iDisplayLogicalIndex && -1 == other_.iDisplayLogicalIndex) ? (one_.iDisplayLogicalIndex == other_.iDisplayPhysicalIndex) :
        false;

    if (match) {
        match = (one_.iDisplayPhysicalAdapterIndex == other_.iDisplayPhysicalAdapterIndex);
        if (!match && NULL != ADL2_Adapter_AdapterInfoX3_Get) {
            int oneBDF, otherBDF;
            LPAdapterInfo adNfo = NULL;
            ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, one_.iDisplayPhysicalAdapterIndex, NULL, &adNfo);
            if (NULL != adNfo) {
                oneBDF = GpuBDF(adNfo->iBusNumber, adNfo->iDeviceNumber, adNfo->iFunctionNumber);
                ADL_Main_Memory_Free((void**)&adNfo);

                ADL2_Adapter_AdapterInfoX3_Get(ADLContext_, other_.iDisplayPhysicalAdapterIndex, NULL, &adNfo);
                if (NULL != adNfo) {
                    otherBDF = GpuBDF(adNfo->iBusNumber, adNfo->iDeviceNumber, adNfo->iFunctionNumber);
                    ADL_Main_Memory_Free((void**)&adNfo);
                    match = (oneBDF == otherBDF);
                }
            }
        }
    }
    return match;
}
//获取初始显卡ID
int GetPrimaryAdpaterId(char displayName[])
{
    int adlRet = ADL_ERR;
    int numAdapters = 0;
    AdapterInfo*   allAdapterInfo = NULL;

    adlRet = ADL2_Adapter_AdapterInfoX3_Get(GetADLContext(), -1, &numAdapters, &allAdapterInfo);
    if (ADL_OK != adlRet)
        return -1;

    int primaryIndex = -1;
    for (int i = 0; i < numAdapters; i++) {
        int vendorID = allAdapterInfo[i].iVendorID;
        if (vendorID != 1002)
            continue;
        //查找相同名称的显示器，获取其编号
        if (strcmp(allAdapterInfo[i].strDisplayName, displayName) == 0) {
            primaryIndex = allAdapterInfo[i].iAdapterIndex;
            break;
        }
    }
    return primaryIndex;
}
//释放配置信息指针
int atiEyefinityReleaseConfigInfo(DisplayInfoStruct **lppDisplaysInfo)
{
    ADL_Main_Memory_Free((void**)lppDisplaysInfo);
    return TRUE;
}
