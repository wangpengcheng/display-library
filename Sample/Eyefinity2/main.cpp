///
///  Copyright (c) 2008 - 2012 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file main.cpp

#include <windows.h>
#include <stdio.h>
//--- 使用 eyefinity 头文件 ---
#include "eyefinity.h"
#include <string.h>
#include <stdlib.h>

//main 主函数
int main (
	int c, //参数统计
	char* k[], //参数1指针
	char* s[] //参数2 指针
	)
{
   
	//定义显示索引和显示计数   
	int iDisplayIndex[6],displayCount=0;
	//临时计数和存在参数
	int temp= 0,iValidParameters=1;
	//水平和竖直参数
	int iHBezel=0,iVBezel=0;
	int i=0, j=0;
	//存在输入
	int validInput=-1;
	//宽和高
	int xRes=0,yRes=0;
	char *token, *val, *sSep; 
	char str[256],op;
	//参数数组
	int args[3],argc=1;	
	//参数数组分配内存
	memset(args,-1,sizeof(args));
	//初始化ADL
	if (initializeADL())
	{
		//为步长分配内存
		sSep = (char*)malloc(sizeof(char));
		//如果只有一个参数，输入错误信息
		if (c== 1)
		{
			//输出帮助信息
			printSyntax();
		}
		else
		{
			//如果参数大于1，依次读取参数
			while (argc++ < c)
			{
				k++;//k指针后移
				//拷贝k中的数据到str
				strcpy(str, *k);
				*sSep = *str;//获取首字符
				val = strtok (str,sSep);//将字符分割开
				if (*sSep == 'i')//如果是i 输出显示序号信息
				{
					printDisplayIndexes();
				}
				//否则继续解析
				else if (val != NULL)
				{
					//判断首字符
					switch (*sSep)
					{
					case 'e'://如果是e				
						token = strtok (val,",");//按照，分割字符串
						while (token != NULL)//遍历数组，在粗分割
						{
							args[temp++] = atoi(token);// 将字符串转换为整数
							token = strtok (NULL,",");//重新设置token指针							
						}		
						op ='e';//设置op为'e'
						break;		
					case 'r'://设置显示	
						token = strtok (val,",");//获取剩下的参数
						while (token != NULL)
						{
							args[temp++] = atoi(token);
							token = strtok (NULL,",");
						}	
						op ='r';
						break;
					case 'd'://断开连接	
						token = strtok (val,",");
						while (token != NULL)
						{
							args[temp++] = atoi(token);
							token = strtok (NULL,",");
						}	
						op ='d';
						break;
					case 'b'://设置偏移	
						token = strtok (val,",");
						while (token != NULL)
						{
							args[temp++] = atoi(token);
							token = strtok (NULL,",");
						}	
						op ='b';
						break;
					case 's'://设置宽度
						token = strtok (val,",");
						while (token != NULL)
						{
							args[temp++] = atoi(token);
							token = strtok (NULL,",");
						}
						op ='s';
						break;
					case 'm'://获取序号	
						token = strtok (val,",");
						while (token != NULL)
						{
							iDisplayIndex[displayCount++] = atoi(token);//将数字转化为序号
							token = strtok (NULL,",");
						}		
						break;
					case 'x'://设置宽度
						xRes= atoi(val);			
						break;
					case 'y'://设置高度	
						yRes= atoi(val);			
						break;	
					case 'v'://设置位置	
						iVBezel= atoi(val);			
						break;
					case 'h'://设置水平边框
						if ( val!= NULL)
							iHBezel= atoi(val);	
						break;		
					default://空参数
						iValidParameters = 0;
						break;
					}
		
				}
				else//空参数为0
					iValidParameters = 0;
			}
			//如果输入参数为空
			if (!iValidParameters)
			{
				printf("not a valid parameteres \n");
				printSyntax();
			}
			//否则设置偏移
			else if (iHBezel >0 && iVBezel >0 && args[0] != -1 && op =='b')
			{
				setBezelOffsets(args[0], iHBezel, iVBezel);
			}
			//否则设置显示长宽
			else if (xRes >0 && yRes >0 && args[0] != -1 && op =='s')
			{
				setResolution(args[0], xRes, yRes);
			}
			//设置显示序号
			else if (args[0] != -1 && args[1] != -1 && args[2] != -1 && (displayCount == args[1] * args[2]) && op =='e')
			{
				setAdapterDisplaysToEyefinity(args[0], args[1], args[2], iDisplayIndex, displayCount , 0);
			}
			//重新设置序号
			else if (args[0] != -1 && displayCount > 0 && op =='r')
			{
				setAdapterDisplaysToEyefinity(args[0], 0, 0, iDisplayIndex, displayCount , 1);
			}	
			//断开连接
			else if (args[0] != -1 && op =='d')
			{
				disableAdapterEyefinityMapping(args[0]);
			}				
		}
		free(sSep);
		deinitializeADL();	
	}
	
    return 0;
}
void printSyntax()
{
	printf ("This Application accepts below parameters\n");
	printf ("-----------------------------------------\n");
	printf ("Display Information \t eyefinity i\n");
	printf ("\t\t \t Ex: eyefinity i\n\n\n");
	printf ("Creat Eyefinity \t eyefinity e<AdapterIndex>,<Columns>,<Rows> \n\t\t\t\t   m<DisplayIndex1,DisplayIndex2...,DisplayIndexN>\n");
	printf ("\t\t \t Ex: eyefinity e13,2,3 m0,2,3,4,5,1\n\n\n");
	printf ("Disable Eyefinity \t eyefinity d<AdapterIndex>\n");
	printf ("\t\t \t Ex: eyefinity d13\n\n\n");
	printf ("Re arrange Eyefinity \t eyefinity r<AdapterIndex> \n\t\t\t\t   m<DisplayIndex1,DisplayIndex2...,DisplayIndexN>\n");
	printf ("\t\t \t Ex: eyefinity r13 m0,2,3,4,5,1\n\n\n");
	printf ("Set Bezel Offsets \t eyefinity b<AdapterIndex> \n\t\t\t\t   h<Horizontal Offset in pixel> \n\t\t\t\t   v<Vertical Offset in pixel>\n");
	printf ("\t\t \t Ex: eyefinity b13 h120 v130\n\n\n");
	printf ("Set Resolution \t\t eyefinity s<AdapterIndex> \n\t\t\t\t   x<Horizontal resolution> \n\t\t\t\t   y<Vertical resolution>\n");
	printf ("\t\t \t Ex: eyefinity s13 x1920 y1200\n\n\n");
	
}

