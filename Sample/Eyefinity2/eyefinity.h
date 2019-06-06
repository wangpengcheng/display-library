///
///  Copyright (c) 2008 - 2012 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file eyefinity.h

#ifndef EYEFINITY_H_
#define EYEFINITY_H_

#include <windows.h>
#include "..\..\include\adl_sdk.h"
#include "eyefinity.h"
#include <stdio.h>
//---声明函数 start---
//设置显卡显示器到视觉增强
extern int setAdapterDisplaysToEyefinity(int iAdapterIndex, int iRows, int iColumns,int iDisplayMapIndexes[], int iNumOfDisplays, int iSLSRearrange);
//设置拼缝
extern int setBezelOffsets(int iAdapterIndex, int iHbezel, int iVbezel);
//设置解决方案
extern int setResolution(int iAdapterIndex, int iXRes, int iYRes);
//初始化显卡
extern int setPrimaryAdapter(int iAdapterIndex);
//禁用显卡显示
extern int setAdapterDisplaysToDisable(int iAdapterIndex);
//复制显卡显示
extern int setAdapterDisplaysToClone(int iAdapterIndex, int iDisplayIndexes[], int iDisplaysCount);
//禁止显卡映射
extern int disableAdapterEyefinityMapping(int iAdapterIndex);
//输出显卡下标
extern int printDisplayIndexes();
//显卡中的显示序号
extern int getDisplayIndexesofOneGPU(int iCurrentAdapterIndex, int *lpAdpaterIndexes, int *lpDisplayIndexes, int *lpCount );
//设置初始化
extern int CanSetPrimary(int iAdapterIndex, int iCurrentPrimaryAdapterIndex);
//初始化ADL
extern int initializeADL();
//销毁ADL
extern void deinitializeADL();
//动态打印相关信息
extern void printSyntax();
//---声明函数 end---

#endif /* EYEFINITY_H_ */
