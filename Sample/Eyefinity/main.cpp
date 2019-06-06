///
///  Copyright (c) 2008 - 2010 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file main.c

#include <windows.h>
#include <stdio.h>
#include "eyefinity.h"

#define DISPLAY_NAME_LEN 32
//主函数
int main (int c,char* k[],char* s[])
{
	// OSDispalyName
	char OSDisplayName[DISPLAY_NAME_LEN];	
    //显示信息数量
    int iNumDisplaysInfo = 0;
	//初始化Eyefinity结构体列表
	EyefinityInfoStruct eyefinityInfo = {0};
	//显示信息结构体指针
	DisplayInfoStruct *pDisplaysInfo = NULL;
	
	 // Get the default active display
	 // 获取默认激活的显示器
 	int iDevNum = 0;
	int dwFlags = 0;
	//显示器设备函数
	DISPLAY_DEVICE displayDevice;
	//开始预分配内存，并对每个内存置空值
    memset(&OSDisplayName,'\0', DISPLAY_NAME_LEN);
	
	displayDevice.cb = sizeof(displayDevice);
	//确认显示器的数目，并且获取相关信息
	while ( EnumDisplayDevices(0, iDevNum, &displayDevice, 0) ) {
		//复制OSDisplayName。
		if (0 != (displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) ) {
			memcpy(OSDisplayName, displayDevice.DeviceName, DISPLAY_NAME_LEN);
			break;
		}
		iDevNum++;
	}

	// Find out if this display has an Eyefinity config enabled
	// 扎到已经有 Eyefinity配置的显示
	if (TRUE == atiEyefinityGetConfigInfo( OSDisplayName, &eyefinityInfo, &iNumDisplaysInfo, &pDisplaysInfo )) {
		//如果已经激活
		if (TRUE == eyefinityInfo.iSLSActive) {
			int iCurrentDisplaysInfo = 0;
			//输出相关信息
			//输出显示器名称
			printf ( "\nEYEFINITY ENABLED for display name %s:\n", OSDisplayName);
			// 输出宽度和高度
			printf ( " SLS grid is %i displays wide by %i displays tall.\n", eyefinityInfo.iSLSGridWidth, eyefinityInfo.iSLSGridHeight );
			//输出分辨率
			printf ( " SLS resolution is %ix%i pixels.\n", eyefinityInfo.iSLSWidth, eyefinityInfo.iSLSHeight );
			//是否有遮挡补偿
			if (TRUE == eyefinityInfo.iBezelCompensatedDisplay ) {
				printf ( " SLS is bezel-compensated.\n" );
			}
			//遍历显示器信息，输出其，相关坐标和位置
			for ( iCurrentDisplaysInfo = 0; iCurrentDisplaysInfo < iNumDisplaysInfo; iCurrentDisplaysInfo++ ) {
				printf ( "\nDisplay %i\n", iCurrentDisplaysInfo);

				if ( TRUE == pDisplaysInfo[iCurrentDisplaysInfo].iPreferredDisplay ) {
					printf ( " Preferred/main monitor\n");
				}
				//相对坐标
				printf ( " SLS grid coord [%i,%i]\n", pDisplaysInfo[iCurrentDisplaysInfo].iGridXCoord, pDisplaysInfo[iCurrentDisplaysInfo].iGridYCoord );
				//基础坐标
				printf ( " Base coord [%i,%i]\n", pDisplaysInfo[iCurrentDisplaysInfo].displayRect.iXOffset, pDisplaysInfo[iCurrentDisplaysInfo].displayRect.iYOffset );
				// 分辨率
				printf ( " Dimensions [%ix%i]\n", pDisplaysInfo[iCurrentDisplaysInfo].displayRect.iWidth, pDisplaysInfo[iCurrentDisplaysInfo].displayRect.iHeight );
				//可见基础坐标
				printf ( " Visible base coord [%i,%i]\n", pDisplaysInfo[iCurrentDisplaysInfo].displayRectVisible.iXOffset, pDisplaysInfo[iCurrentDisplaysInfo].displayRectVisible.iYOffset );
				//可见维度
				printf ( " Visible dimensions [%ix%i]\n", pDisplaysInfo[iCurrentDisplaysInfo].displayRectVisible.iWidth, pDisplaysInfo[iCurrentDisplaysInfo].displayRectVisible.iHeight );
			}
		}
		else {
			//没有查到到相关配置的信息
			printf ( "\nEYEFINITY DISABLED for display name %s.\n", OSDisplayName);
		}
	} else {
		//获取配置信息失败
		printf ( "Eyefinity configuration query failed for display name %s.\n", OSDisplayName);
	}
	//释放信息结构体        
    if (iNumDisplaysInfo > 0) {
        atiEyefinityReleaseConfigInfo(&pDisplaysInfo);
    }
    return 0;
}