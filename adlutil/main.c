///
///  Copyright (c) 2010 Advanced Micro Devices, Inc.
 
///  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
///  EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
///  WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

/// \file main.c
/// \brief C/C++ ADL application to retrieve and set TimigOverride and resolution/color depth modes
///
///  Author: Borislav Zahariev


#if defined (LINUX)
#include "../include/adl_sdk.h"
#include <dlfcn.h>	//dyopen, dlsym, dlclose
#include <errno.h> // fopen_s
#include <stdlib.h>	
#include <string.h>	//memeset

// Linux equivalent of sprintf_s
#define sprintf_s snprintf
// Linux equivalent of fscanf_s
#define fscanf_s fscanf
// Linux equivalent of GetProcAddress
#define GetProcAddress dlsym

#else
#include <windows.h>
#include "..\include\adl_sdk.h"
#endif

#include <stdio.h>


//定义个种函数内存分配
typedef int ( *ADL_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int );
typedef int ( *ADL_MAIN_CONTROL_DESTROY )();
typedef int ( *ADL_ADAPTER_NUMBEROFADAPTERS_GET ) ( int* );
typedef int ( *ADL_ADAPTER_ADAPTERINFO_GET ) ( LPAdapterInfo, int );
typedef int ( *ADL_ADAPTER_ACTIVE_GET ) ( int, int* );
typedef int ( *ADL_DISPLAY_COLORCAPS_GET ) ( int, int, int *, int * );
typedef int ( *ADL_DISPLAY_COLOR_GET ) ( int, int, int, int *, int *, int *, int *, int * );
typedef int ( *ADL_DISPLAY_COLOR_SET ) ( int, int, int, int );
typedef int ( *ADL_DISPLAY_DISPLAYINFO_GET ) ( int, int *, ADLDisplayInfo **, int );
typedef int ( *ADL_DISPLAY_MODES_GET ) (int iAdapterIndex, int iDisplayIndex, int* lpNumModes, ADLMode** lppModes);
typedef int ( *ADL_DISPLAY_MODES_SET ) (int iAdapterIndex, int iDisplayIndex, int lpNumModes, ADLMode * lpModes);
typedef int ( *ADL_DISPLAY_MODETIMINGOVERRIDE_GET) (int iAdapterIndex, int iDisplayIndex, ADLDisplayMode *lpModeIn, ADLDisplayModeInfo *lpModeInfoOut);
typedef int ( *ADL_DISPLAY_MODETIMINGOVERRIDE_SET) (int iAdapterIndex, int iDisplayIndex, ADLDisplayModeInfo *lpMode, int iForceUpdate);
typedef int ( *ADL_DISPLAY_MODETIMINGOVERRIDELIST_GET) (int iAdapterIndex, int iDisplayIndex, int iMaxNumOfOverrides, ADLDisplayModeInfo *lpModeInfoList, int *lpNumOfOverrides);
typedef int ( *ADL_ADAPTER_VIDEOBIOSINFO_GET )( int iAdapterIndex, ADLBiosInfo* lpBiosInfo );


int OpenADL( void );//加载ADL
void CloseADL( void );//关闭ADL
int GetValue( char * name, int * iValue, int line );//获取相关值
int GetHex(  char * name, int * iValue, int line );//
int GetFloat(  char * name, float * fValue, int line );//获取浮点数
int AdlModeFromFile( LPADLMode  lpADLMode );//配置文件加载ADL
int AdlModeToFile( LPADLMode lpADLMode );//将ADL配置加载到文件
int AdlDisplayModeInfoFromFile(  int * lpAdapterIdx, int * lpDisplayIdx, ADLDisplayModeInfo * lpModeInfoList );//从文件中加载显示信息
int AdlDisplayModeInfoToFile( FILE * file, int iAdapterIdx, int iDisplayIdx, ADLDisplayModeInfo * lpModeInfoList );//将显示信息，加载到文件
int AdlBiosInfoToFile( int iAdapterIndex, int iDisplayIndex, ADLBiosInfo * lpAdlBI );//ADLbiso信息加载到文件
int DisplayErrorAndPause( char * sError );//显示错误和暂停
void ShowHelp(void);//显示帮助信息

//分配主要内存
void* __stdcall ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}
//销毁主要内存
void __stdcall ADL_Main_Memory_Free ( void** lpBuffer )
{
    if ( NULL != *lpBuffer )
    {
        free ( *lpBuffer );
        *lpBuffer = NULL;
    }
}

//命令参数选项
enum	COMMAND	{ 
	NONE,//无 
	GETALL, //获得所有
	GETMODE, //获得模式
	SETMODE, //设置模式
	INFOOVER, //信息覆盖
	SETOVER, //设置覆盖
	GETLIST, //获取列表
	GENERATE, //
	BIOSINFO //nios信息
};

#if defined (LINUX)
// Linux equivalent function of fopen_s
int fopen_s ( FILE ** file, const char *filename, const char *mode )
{
	if ( NULL == file )
		return EINVAL;

	( *file ) = fopen ( filename, mode );

	if ( NULL != *file )
		return 0;
	else
		return errno;
}

#endif

#if defined (LINUX)
    void *hDLL;		// Handle to .so library
#else
    HINSTANCE hDLL;		// Handle to DLL //获取dll句柄，dll全局变量
#endif

    LPAdapterInfo     lpAdapterInfo = NULL; //窗口
    LPADLDisplayInfo  lpAdlDisplayInfo = NULL; //显示的基本信息
	LPADLMode lpADLMode = NULL; //显示模式
	ADLDisplayModeInfo * lpModeInfoList = NULL; //显示模式列表

	FILE * file = NULL;//文件，定义读入文件
	FILE * file2 = NULL;//文件2
	char msg[ 128 ];//信息字符串128
	char err[ 128 ];//错误信息
	int sMsg = sizeof( msg );//分配内存
	int sErr = sizeof( err );//分配内存

int main( int argc, char *argv[] )
{
    ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get;//获取显示的数量
    ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get;//获取显示接口
    ADL_ADAPTER_ACTIVE_GET           ADL_Adapter_Active_Get;//获取活动的显示接口
    ADL_DISPLAY_DISPLAYINFO_GET      ADL_Display_DisplayInfo_Get;//获取显示信息
	ADL_DISPLAY_MODES_GET			ADL_Display_Modes_Get;//获取显示模式
	ADL_DISPLAY_MODES_SET			ADL_Display_Modes_Set;//设置显示模式
	ADL_DISPLAY_MODETIMINGOVERRIDE_GET	ADL_Display_ModeTimingOverride_Get;//此函数检索指定显示的显示模式时序覆盖信息。
	ADL_DISPLAY_MODETIMINGOVERRIDE_SET	ADL_Display_ModeTimingOverride_Set; //设置显示模式
	ADL_DISPLAY_MODETIMINGOVERRIDELIST_GET	ADL_Display_ModeTimingOverrideList_Get;//模型运行时重载队列
	ADL_ADAPTER_VIDEOBIOSINFO_GET	ADL_Adapter_VideoBiosInfo_Get;//获取转接器的基本bios信息
	
    int  i, j, k; 
    int  ADL_Err;//ADL 错误信息
    int  iActive; //激活信息
    int  iNumberAdapters;//转接器 数目
    int  iAdapterIndex;//转接器索引下标
    int  iDisplayIndex;//显示器下标
    int  iNumDisplays;//显示的数目
    int  iNumModes;//显示模式
	int  iMaxNumOfOverrides = 100;		// Adjust this if necessary, but 100 seems enough.
	int  lpNumOfOverrides; //lp重载数目
	int command = NONE;//指令

	ADLMode adlmode; //adl模式
	ADLDisplayMode AdlDM;//adl显示模式，包含，像素宽、高，色彩深度和刷新频率
	ADLDisplayModeInfo AdlDmi;//ADL模式基本信息
	ADLBiosInfo AdlBI;//ADLBIOS信息

    memset ( &AdlDM,'\0', sizeof (AdlDM));//ADL显示信息分配内存
	memset ( &AdlDmi,'\0', sizeof (AdlDmi));//ADL基本信息分配内存

	if ( argc < 2 )//参数判断，输入参数小于就直接返回帮助错误信息
	{
		ShowHelp();//显示帮助信息
		return 0;
	}

	// Get the display mode
	if ( 0 == strcmp (argv[ 1] , "get" ) )//存在get指令
	{
		command = GETMODE;//初始化模式为get模式
		if ( 3 == argc )//读取剩余参数，剩余参数为3
		{
			if ( fopen_s( &file, argv[ 2 ],"w") )//打开保存文件
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
		else
			file = stdout;//将标准输出，重定位到file文件
	}
	// Get the Override Info
	//检查输入指令，获取重载信息
	else if ( 0 == strcmp (argv[ 1] , "info" ) )
	{
		command = INFOOVER;
		if ( 4 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"r") )//打开相关文件
			{
				printf( "Error openning file %s!\n", argv[2] );//输出错误文件路径
				return 0;
			}
			if ( fopen_s( &file2, argv[ 3 ],"w") )//打开输出文件
			{
				printf( "Error openning file %s!\n", argv[3] );
				return 0;
			}
		}
		//如果输入参数为3
		else if ( 3 == argc )
		{
			//打开文件地址
			if ( fopen_s( &file, argv[ 2 ],"r") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
			file2 = stdout;
		}
		else
		{
			//输入错误提示，输出帮助信息
			printf( "The 'info' command requires at least one file\n" );
			ShowHelp();
			return 0;
		}
	}
	// Get the Override LIst
	else if ( 0 == strcmp (argv[ 1] , "list" ) )//获取重载列表
	{
		command = GETLIST;//模式为获取列表
		if ( 3 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"w") )//打开输出文件
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
		else
			file = stdout;
	}
	// Generate pattern
	// 通用选项
	else if ( 0 == strcmp (argv[ 1] , "gen" ) )
	{
		command = GENERATE;
		if ( 3 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"w") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
		else
			file = stdout;//输出文件
	}
	// Set mode according to the settings in a text file
	// 根据txt文件设置模式
	else if ( 0 == strcmp (argv[ 1] , "set" ) )
	{
		command = SETMODE;
		if ( argc < 3 )
		{
			//显示错误并且中断
			DisplayErrorAndPause( "ERROR: The 'set' command requires a filename\n" );
			return 0;
		}
		else
		{
			//打开相关文件
			if ( fopen_s( &file, argv[ 2 ],"r") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
	}
	// Set Override mode according to the settings in a text file
	// 根据文件设置重载模式
	else if ( 0 == strcmp (argv[ 1] , "over" ) )
	{
		command = SETOVER;
		if ( argc < 3 )
		{
			DisplayErrorAndPause( "ERROR: The 'over' command requires a filename\n" );
			return 0;
		}
		else
		{
			if ( fopen_s( &file, argv[ 2 ],"r") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
	}
	// Retrieves all modes from the system across multiple GPUs
	// 通过多个GPU从系统中检索所有模式
	else 	if ( 0 == strcmp (argv[ 1] , "all" ) )
	{
		command = GETALL;
		if ( 3 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"w") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
		else
			file = stdout;
	}
	else 	if ( 0 == strcmp (argv[ 1] , "bios" ) )
	{
		//bios信息
		command = BIOSINFO;
		if ( 3 == argc )
		{
			if ( fopen_s( &file, argv[ 2 ],"w") )
			{
				printf( "Error openning file %s!\n", argv[2] );
				return 0;
			}
		}
		else
			file = stdout;
	}
	else
	{
		//输入错误，显示帮助信息
			printf( "\nERROR: Unrecognized command!\n" );
			ShowHelp();
			return 0;
	}
			//打开ADL
			ADL_Err = OpenADL();
			// Error during ADL initialization?
			// 初始化并且获取各种函数
			// 
			if (ADL_OK != ADL_Err )
  					return DisplayErrorAndPause( "ERROR: ADL Initialization error!" );
			// Get the function pointers from ADL:
			// 获取函数指针
			ADL_Display_Modes_Get = (ADL_DISPLAY_MODES_GET)GetProcAddress(hDLL,"ADL_Display_Modes_Get");
			//获取显示模式函数
			if ( NULL == ADL_Display_Modes_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_Modes_Get not available!" );
			ADL_Display_Modes_Set = (ADL_DISPLAY_MODES_SET)GetProcAddress(hDLL,"ADL_Display_Modes_Set");
			if ( NULL == ADL_Display_Modes_Set )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_Modes_Set not available!" );
	        ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL,"ADL_Adapter_NumberOfAdapters_Get");
			if ( NULL == ADL_Adapter_NumberOfAdapters_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Adapter_NumberOfAdapters_Get not available!" );
            ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL,"ADL_Adapter_AdapterInfo_Get");
			if ( NULL == ADL_Adapter_AdapterInfo_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Adapter_AdapterInfo_Get not available!" );
			ADL_Adapter_Active_Get = (ADL_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL,"ADL_Adapter_Active_Get");
			if ( NULL == ADL_Adapter_Active_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Adapter_Active_Get not available!" );
			ADL_Display_DisplayInfo_Get = (ADL_DISPLAY_DISPLAYINFO_GET)GetProcAddress(hDLL,"ADL_Display_DisplayInfo_Get");
			if ( NULL == ADL_Display_DisplayInfo_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_DisplayInfo_Get not available!" );
			ADL_Display_ModeTimingOverride_Get = (ADL_DISPLAY_MODETIMINGOVERRIDE_GET)GetProcAddress(hDLL,"ADL_Display_ModeTimingOverride_Get");
			if ( NULL == ADL_Display_ModeTimingOverride_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverride_Get not available!" );
			ADL_Display_ModeTimingOverride_Set = (ADL_DISPLAY_MODETIMINGOVERRIDE_SET)GetProcAddress(hDLL,"ADL_Display_ModeTimingOverride_Set");
			if ( NULL == ADL_Display_ModeTimingOverride_Set )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverride_Set not available!" );
			ADL_Display_ModeTimingOverrideList_Get = (ADL_DISPLAY_MODETIMINGOVERRIDELIST_GET)GetProcAddress(hDLL,"ADL_Display_ModeTimingOverrideList_Get");
			if ( NULL == ADL_Display_ModeTimingOverrideList_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverrideList_Get not available!" );
			ADL_Adapter_VideoBiosInfo_Get = (ADL_ADAPTER_VIDEOBIOSINFO_GET)GetProcAddress(hDLL,"ADL_Adapter_VideoBiosInfo_Get");
			if ( NULL == ADL_Adapter_VideoBiosInfo_Get )
		  			return DisplayErrorAndPause( "ERROR: ADL_Adapter_VideoBiosInfo_Get not available!" );

		//获取信息模式
		if ( GETALL == command )
		{
			//获取显示模式函数
				ADL_Err = ADL_Display_Modes_Get ( -1, -1, &iNumModes, &lpADLMode );
				/*
				[in] 	iAdapterIndex 	The ADL index handle of the desired adapter. A value of -1 retrieves all modes for the system across multiple GPUs.
				[in] 	iDisplayIndex 	The desired display index. If the index is -1, this field is ignored.
				[out] 	lpNumModes 	The pointer to the number of modes retrieved.
				[out] 	lppModes 	The pointer to the pointer to the retrieved display modes. Refer to the Display ADLMode structure for more information.
				 */
				if ( lpADLMode )
				{
					for ( i = 0; i < iNumModes; i++ )
					{
						//将信息加载到文件
						ADL_Err = AdlModeToFile( &lpADLMode[ i ] );
					}
				}
				if ( ADL_OK != ADL_Err )
					DisplayErrorAndPause( "ERROR: Cannot save data to file!" );
				CloseADL();
				return ADL_Err;
		}
		//设置命令模式
		if ( SETMODE == command )
		{
			iNumModes = 1;
			//从文件中加载模型
			ADL_Err = AdlModeFromFile( &adlmode );
			//文件读取错误，输出错误信息
			if ( ADL_OK != ADL_Err )
			{
				if ( 0 < ADL_Err )
						sprintf_s( msg, sMsg, "ERROR in file: %s\n%s", argv[ 2 ], err );
				else
						sprintf_s( msg, sMsg, "ERROR accessing file: %s", argv[ 2 ] );
				DisplayErrorAndPause( msg );
				CloseADL();
				return ADL_ERR;
			}

			// Obtain the ADL_Display_Modes_Set
			ADL_Err = ADL_Display_Modes_Set( adlmode.iAdapterIndex, adlmode.displayID.iDisplayLogicalIndex, iNumModes, &adlmode );

			if ( ADL_OK != ADL_Err )
				DisplayErrorAndPause( "ERROR: Calling ADL_Display_Modes_Set()" );
			CloseADL();
			return ADL_Err;
		}
		//设置显示模式
		if ( SETOVER == command )
		{
				//从文件中获取显示模型信息
				ADL_Err = AdlDisplayModeInfoFromFile( &iAdapterIndex, &iDisplayIndex, &AdlDmi );
				if ( ADL_OK != ADL_Err )
				{
					if ( 0 < ADL_Err )
							sprintf_s( msg, sMsg, "ERROR in file: %s\n%s", argv[ 2 ], err );
					else
							sprintf_s( msg, sMsg, "ERROR accessing file: %s", argv[ 2 ] );
					DisplayErrorAndPause( msg );
					CloseADL();
					return ADL_ERR;
				}
				//设置显示刷新模式
				ADL_Err = ADL_Display_ModeTimingOverride_Set( iAdapterIndex, iDisplayIndex,  &AdlDmi, 1 );
				if ( ADL_OK != ADL_Err )
					DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverride_Set() failed!" );

				CloseADL();
				return ADL_Err;
		}
		//获取刷新信息
		if ( INFOOVER == command )
		{
				ADL_Err = AdlModeFromFile( &adlmode );
				if ( ADL_OK != ADL_Err )
				{
					if ( 0 < ADL_Err )
							sprintf_s( msg, sMsg, "ERROR in file: %s\n%s", argv[ 2 ], err );
					else
							sprintf_s( msg, sMsg, "ERROR accessing file: %s", argv[ 2 ] );
					DisplayErrorAndPause( msg );
				}
				else
				{
					AdlDM.iPelsHeight = adlmode.iYRes;//y高度
					AdlDM.iPelsWidth = adlmode.iXRes;//x宽度
					AdlDM.iBitsPerPel = adlmode.iColourDepth;//色彩深度
					AdlDM.iDisplayFrequency = (int)adlmode.fRefreshRate; //刷新频率

					ADL_Err = ADL_Display_ModeTimingOverride_Get( adlmode.iAdapterIndex, adlmode.displayID.iDisplayLogicalIndex,  &AdlDM, &AdlDmi );
					if ( ADL_OK != ADL_Err )
						DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverride_Get() failed!" );
					else
						//将信息存储到文件中
						AdlDisplayModeInfoToFile( file2, adlmode.iAdapterIndex, adlmode.displayID.iDisplayLogicalIndex, &AdlDmi );
				}
				CloseADL();
				return ADL_Err;
		}

		// Obtain the number of adapters for the system
		// 获取转接器数目
        ADL_Adapter_NumberOfAdapters_Get ( &iNumberAdapters );

        if ( 0 < iNumberAdapters )
        {
        	//分配内存，存储转接器信息
            lpAdapterInfo = malloc ( sizeof (AdapterInfo) * iNumberAdapters );
            //设置初始化
            memset ( lpAdapterInfo,'\0', sizeof (AdapterInfo) * iNumberAdapters );

            // Get the AdapterInfo structure for all adapters in the system
            //获取转接器的所有信息
            ADL_Adapter_AdapterInfo_Get (lpAdapterInfo, sizeof (AdapterInfo) * iNumberAdapters);
        }
		else
		{
				DisplayErrorAndPause( "ERROR: No adapters found in this system!" );
				CloseADL();
				return ADL_Err;
		}

        // Repeat for all available adapters in the system
        // 获取所有转接器信息
        for ( i = 0; i < iNumberAdapters; i++ )
        {
        		//获取转接器编号
				iAdapterIndex = lpAdapterInfo[ i ].iAdapterIndex;
				//he function is used to check if the adapter associated with iAdapterIndex is active. Logic is different for Windows and Linux!
				ADL_Err = ADL_Adapter_Active_Get ( iAdapterIndex, &iActive );
				// If the adapter is not active skip the steps below.
				if ( 0 == iActive || ADL_OK != ADL_Err)
					continue;
				//释放显示指针信息
				ADL_Main_Memory_Free ( (void **)&lpAdlDisplayInfo );
				//获取显示信息
				ADL_Err = ADL_Display_DisplayInfo_Get (iAdapterIndex, &iNumDisplays, &lpAdlDisplayInfo, 1);

				if (ADL_OK != ADL_Err)
					continue;

				for ( j = 0; j < iNumDisplays; j++ )
				{
					// For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
					if (  ( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED ) != 
						( ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED	 &
							lpAdlDisplayInfo[ j ].iDisplayInfoValue ) )
							continue;   // Skip the not connected or not mapped displays

						// Is the display mapped to this adapter?
						// 判断显示是否连接到转换器接口
						if ( iAdapterIndex != lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalAdapterIndex )
							continue;

						iDisplayIndex = lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalIndex;
						
						if ( BIOSINFO == command )
						{
								// Obtain the ASIC Bios Info
								// 获取BIos信息
								ADL_Err = ADL_Adapter_VideoBiosInfo_Get ( iAdapterIndex, &AdlBI );
								if ( ADL_OK != ADL_Err )
									DisplayErrorAndPause( "ERROR: ADL_Adapter_VideoBiosInfo_Get() failed!" );
								else
								{
									//将信息存储到文件
									ADL_Err = AdlBiosInfoToFile( iAdapterIndex, iDisplayIndex, &AdlBI );
									if ( ADL_OK != ADL_Err )
										DisplayErrorAndPause( "ERROR: Cannot save data to file!" );
								}
						}

						if ( GETMODE == command )
						{
								// Obtain the ADL_Display_Modes_Get
								ADL_Err = ADL_Display_Modes_Get ( iAdapterIndex, iDisplayIndex, &iNumModes, &lpADLMode );
								if ( ADL_OK != ADL_Err )
									DisplayErrorAndPause( "ERROR: ADL_Display_Modes_Get() failed!" );
								else
								{
									if ( lpADLMode )
									{
										ADL_Err = AdlModeToFile( lpADLMode );
									}

									if ( ADL_OK != ADL_Err )
										DisplayErrorAndPause( "ERROR: Cannot save data to file!" );
								}
						}
						//获取列表信息
						if ( GETLIST == command )
						{
								//分配内存
								lpModeInfoList = malloc ( sizeof (ADLDisplayModeInfo) * iMaxNumOfOverrides );
								//设置初始值
								memset ( lpModeInfoList, '\0', sizeof (ADLDisplayModeInfo) * iMaxNumOfOverrides );
								//获取刷新信息
								ADL_Err = ADL_Display_ModeTimingOverrideList_Get( iAdapterIndex, iDisplayIndex,
																				 iMaxNumOfOverrides, lpModeInfoList, &lpNumOfOverrides);
								if ( ADL_OK != ADL_Err )
									DisplayErrorAndPause( "ERROR: ADL_Display_ModeTimingOverrideList_Get() failed!" );
								else
								{
										if ( 0 == lpNumOfOverrides )
											printf( "\nTimingOverride list for AdapterIndex %d, DisplayIndex %d is empty!\n",  iAdapterIndex, iDisplayIndex );
										else
										{
											for ( k = 0; k < lpNumOfOverrides; k++ )
												ADL_Err = AdlDisplayModeInfoToFile( file, iAdapterIndex, iDisplayIndex, &lpModeInfoList[ k ] );
										}
								}
								//释放内存
							   ADL_Main_Memory_Free ( (void **)&lpModeInfoList );
						}
						//
						if ( GENERATE == command )
						{
							ADL_Err = ADL_Display_Modes_Get ( iAdapterIndex, iDisplayIndex, &iNumModes, &lpADLMode );
							if ( ADL_OK != ADL_Err )
								DisplayErrorAndPause( "ERROR: ADL_Display_Modes_Get() failed!" );
							else
							{
								AdlDmi.iTimingStandard = ADL_DL_MODETIMING_STANDARD_CVT; // a valid standard. Avoid 0
								AdlDmi.iRefreshRate = (int)lpADLMode->fRefreshRate;
								AdlDmi.iPelsWidth = lpADLMode->iXRes;
								AdlDmi.iPelsHeight = lpADLMode->iYRes;
								ADL_Err = AdlDisplayModeInfoToFile( file, iAdapterIndex, iDisplayIndex, &AdlDmi );
								if ( ADL_OK != ADL_Err )
									DisplayErrorAndPause( "ERROR: Generating a template failed!" );
							}
				}
            }
        }
		CloseADL();
		return 0;
}
//将模式添加到文件中
int AdlModeToFile( LPADLMode lpADLMode )
{
	//指针未空指直接返回
	if ( NULL == file || NULL == lpADLMode )
		return ADL_ERR_NULL_POINTER;
	//输出相关信息
	fprintf( file, "%-15s %d\n", "AdapterIndex", lpADLMode->iAdapterIndex );
	fprintf( file, "%-15s %d\n", "DisplayIndex", lpADLMode->displayID.iDisplayLogicalIndex);
	fprintf( file, "%-15s %d\n", "Width", lpADLMode->iXRes);
	fprintf( file, "%-15s %d\n", "Height", lpADLMode->iYRes);
	fprintf( file, "%-15s %d\n", "ColorDepth", lpADLMode->iColourDepth);
	fprintf( file, "%-15s %.2f\n", "RefreshRate", lpADLMode->fRefreshRate);
	fprintf( file, "%-15s %d\n", "XPos", lpADLMode->iXPos);
	fprintf( file, "%-15s %d\n", "YPos", lpADLMode->iYPos);
	fprintf( file, "%-15s %d\n", "Orientation", lpADLMode->iOrientation);
	fprintf( file, "%-15s %d\n", "ModeFlag", lpADLMode->iModeFlag);
	fprintf( file, "%-15s 0x%04X\n", "ModeMask", lpADLMode->iModeMask);
	fprintf( file, "%-15s 0x%04X\n\n", "ModeValue", lpADLMode->iModeValue);

	return ADL_OK;
}
//从文件中加载配置
int AdlModeFromFile( LPADLMode  lpADLMode )
{
	int iValue;
	float fValue;
	int line = 0;

	if ( NULL == file || NULL == lpADLMode )
		return ADL_ERR_NULL_POINTER;

	if ( ADL_OK == GetValue( "AdapterIndex" , &iValue, ++line ))
		lpADLMode->iAdapterIndex = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "DisplayIndex" , &iValue, ++line  ))
		lpADLMode->displayID.iDisplayLogicalIndex = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "Width" , &iValue, ++line  ))
		lpADLMode->iXRes = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "Height" , &iValue, ++line  ))
		lpADLMode->iYRes = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "ColorDepth" , &iValue, ++line  ))
		lpADLMode->iColourDepth = iValue;
	else
		return line;

	if ( ADL_OK == GetFloat( "RefreshRate" , &fValue, ++line  ))
		lpADLMode->fRefreshRate = fValue;
	else
		return line;

	if ( ADL_OK == GetValue( "XPos" , &iValue, ++line  ))
		lpADLMode->iXPos = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "YPos" , &iValue, ++line  ))
		lpADLMode->iYPos = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "Orientation" , &iValue, ++line  ))
		lpADLMode->iOrientation = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "ModeFlag" , &iValue, ++line  ))
		lpADLMode->iModeFlag = iValue;
	else
		return line;

	if ( ADL_OK == GetHex( "ModeMask" , &iValue, ++line  ))
		lpADLMode->iModeMask = iValue;
	else
		return line;

	if ( ADL_OK == GetHex( "ModeValue" , &iValue, ++line ))
		lpADLMode->iModeValue = iValue;
	else
		return line;

	return ADL_OK;
}
//将模型信息，添加到文件
int AdlDisplayModeInfoToFile( FILE * file, int iAdapterIdx, int iDisplayIdx, ADLDisplayModeInfo * lpModeInfoList )
{
	if ( NULL == file || NULL == lpModeInfoList )
		return ADL_ERR_NULL_POINTER;

			fprintf( file, "%-17s %d\n", "AdapterIndex", iAdapterIdx );
			fprintf( file, "%-17s %d\n", "DisplayIndex", iDisplayIdx );
			fprintf( file, "%-17s %d\n", "TimingStandard", lpModeInfoList->iTimingStandard );
			fprintf( file, "%-17s %d\n", "PossibleStandard", lpModeInfoList->iPossibleStandard );
			fprintf( file, "%-17s %d\n", "RefreshRate", lpModeInfoList->iRefreshRate );
			fprintf( file, "%-17s %d\n", "Width", lpModeInfoList->iPelsWidth );
			fprintf( file, "%-17s %d\n", "Height", lpModeInfoList->iPelsHeight );
			fprintf( file, "%-17s %d\n", "TimingFlags", lpModeInfoList->sDetailedTiming.sTimingFlags );
			fprintf( file, "%-17s %d\n", "HTotal", lpModeInfoList->sDetailedTiming.sHTotal );
			fprintf( file, "%-17s %d\n", "HDisplay", lpModeInfoList->sDetailedTiming.sHDisplay );
			fprintf( file, "%-17s %d\n", "HSyncStart", lpModeInfoList->sDetailedTiming.sHSyncStart );
			fprintf( file, "%-17s %d\n", "HSyncWidth", lpModeInfoList->sDetailedTiming.sHSyncWidth );
			fprintf( file, "%-17s %d\n", "VTotal", lpModeInfoList->sDetailedTiming.sVTotal );
			fprintf( file, "%-17s %d\n", "VDisplay", lpModeInfoList->sDetailedTiming.sVDisplay );
			fprintf( file, "%-17s %d\n", "VSyncStart", lpModeInfoList->sDetailedTiming.sVSyncStart );
			fprintf( file, "%-17s %d\n", "VSyncWidth", lpModeInfoList->sDetailedTiming.sVSyncWidth );
			fprintf( file, "%-17s %d\n", "PixelClock", lpModeInfoList->sDetailedTiming.sPixelClock );
			fprintf( file, "%-17s %d\n", "HOverscanRight", lpModeInfoList->sDetailedTiming.sHOverscanRight );
			fprintf( file, "%-17s %d\n", "HOverscanLeft", lpModeInfoList->sDetailedTiming.sHOverscanLeft );
			fprintf( file, "%-17s %d\n", "VOverscanBottom", lpModeInfoList->sDetailedTiming.sVOverscanBottom );
			fprintf( file, "%-17s %d\n", "VOverscanTop", lpModeInfoList->sDetailedTiming.sVOverscanTop );
//			fprintf( file, "%-17s %d\n", "Overscan8B", lpModeInfoList->sDetailedTiming.sOverscan8B );
//			fprintf( file, "%-17s %d\n", "OverscanGR", lpModeInfoList->sDetailedTiming.sOverscanGR );
			fprintf( file, "\n" );

	return ADL_OK;
}
//从文件中提取相关信息
int AdlDisplayModeInfoFromFile( int * lpAdapterIdx, int * lpDisplayIdx, ADLDisplayModeInfo * lpModeInfoList )
{
	int iValue;
	int line = 0;

	if ( NULL == lpModeInfoList || NULL == lpAdapterIdx || NULL == lpDisplayIdx )
		return ADL_ERR_NULL_POINTER;

	if ( ADL_OK == GetValue( "AdapterIndex" , &iValue, ++line ))
		*lpAdapterIdx = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "DisplayIndex" , &iValue, ++line ))
		*lpDisplayIdx = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "TimingStandard" , &iValue, ++line ))
		lpModeInfoList->iTimingStandard = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "PossibleStandard" , &iValue, ++line ))
		lpModeInfoList->iPossibleStandard = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "RefreshRate" , &iValue, ++line ))
		lpModeInfoList->iRefreshRate = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "Width" , &iValue, ++line ))
		lpModeInfoList->iPelsWidth = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "Height" , &iValue, ++line ))
		lpModeInfoList->iPelsHeight = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "TimingFlags" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sTimingFlags = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HTotal" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHTotal = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HDisplay" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHDisplay = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HSyncStart" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHSyncStart = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HSyncWidth" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHSyncWidth = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VTotal" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVTotal = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VDisplay" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVDisplay = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VSyncStart" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVSyncStart = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VSyncWidth" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVSyncWidth = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "PixelClock" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sPixelClock = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HOverscanRight" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHOverscanRight = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "HOverscanLeft" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sHOverscanLeft = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VOverscanBottom" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVOverscanBottom = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "VOverscanTop" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sVOverscanTop = iValue;
	else
		return line;
/*
	if ( ADL_OK == GetValue( "Overscan8B" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sOverscan8B = iValue;
	else
		return line;

	if ( ADL_OK == GetValue( "OverscanGR" , &iValue, ++line ))
		lpModeInfoList->sDetailedTiming.sOverscanGR = iValue;
	else
		return line;
*/
	return ADL_OK;
}
//获取值
int GetValue(  char * name, int * iValue, int line )
{
	//设置file ld
	char sField[ 256 ];
	//获取文件相关信息
	fscanf_s( file,"%32s %d\n", sField, 33, iValue);
	//如果获取的值和名称不相符合直接退出
	if ( 0 == strcmp( sField, name ) )
		return ADL_OK;
	else
	{
		//输出相关信息
		sprintf_s( err, sErr, "Expected     : %s \nActual       : %s \nIn line      : %d", name, sField, line );
		return ADL_ERR;
	}
}
//获取分辨率
int GetHex(  char * name, int * iValue, int line )
{
	char sField[ 256 ];
	fscanf_s( file,"%32s %X\n", sField, 33, iValue);
	if ( 0 == strcmp( sField, name ) )
		return ADL_OK;
	else
	{
		sprintf_s( err, sErr, "Expected     : %s \nActual       : %s \nIn line      : %d", name, sField, line );
		return ADL_ERR;
	}
}
//获取浮点数
int GetFloat(  char * name, float * fValue, int line )
{
	char sField[ 256 ];
	fscanf_s( file,"%32s %f\n", sField, 33, fValue);
	if ( 0 == strcmp( sField, name ) )
		return ADL_OK;
	else
	{
		sprintf_s( err,  sErr, "Expected     : %s \nActual       : %s \nIn line      : %d", name, sField, line );
		return ADL_ERR;
	}
}

// Initialize ADL
//开启并初始化ADL
int OpenADL()
{
	//句柄创建指针
    ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create;
	int ADL_Err = ADL_ERR;

#if defined (LINUX)
	char sztemp[256];
    sprintf(sztemp,"libatiadlxx.so");
    hDLL = dlopen( sztemp, RTLD_LAZY|RTLD_GLOBAL);
#else
	//加载dll
    hDLL = LoadLibrary( "atiadlxx.dll" );
    if (hDLL == NULL)
    {
        // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        hDLL = LoadLibrary("atiadlxy.dll");
    }
#endif

	if (hDLL != NULL)
    {
		//根据字节对齐查找函数
        ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL,"ADL_Main_Control_Create");
		if ( NULL != ADL_Main_Control_Create)
			//创建控制句柄
            ADL_Err = ADL_Main_Control_Create (ADL_Main_Memory_Alloc, 1);
			// The second parameter is 1, which means:
			// retrieve adapter information only for adapters that are physically present and enabled in the system
		    //接收转接器信息，物理存在和系统可操作的是1
	}
    else
    {
        printf("ADL Library not found!\n" );
    }

	return ADL_Err;
}

// Destroy ADL
//关闭ADL，主要关闭ADL的相关配置文件
void CloseADL()
{
	   ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy;

	   //文件非空则关闭
	if ( NULL != file )
		fclose( file );
	if ( NULL != file2 )
		fclose( file2 );
	    //释放内存
		ADL_Main_Memory_Free ( (void **)&lpAdapterInfo );//销毁转换器信息
		ADL_Main_Memory_Free ( (void **)&lpAdlDisplayInfo );//销毁显示信息
	   //获取结束函数句柄
		ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL,"ADL_Main_Control_Destroy");
		//销毁内存
		if ( NULL != ADL_Main_Control_Destroy )
            ADL_Main_Control_Destroy ();
		
#if defined (LINUX)
    dlclose(hDLL);
#else
    FreeLibrary(hDLL);
#endif
}
//ADLBios 信息到文件中
int AdlBiosInfoToFile( int iAdapterIndex, int iDisplayIndex, ADLBiosInfo * lpAdlBI )
{
	if ( NULL == file || NULL == lpAdlBI )
		return ADL_ERR_NULL_POINTER;

		fprintf( file, "%-17s %d\n", "AdapterIndex", iAdapterIndex );
		fprintf( file, "%-17s %d\n", "DisplayIndex", iDisplayIndex );
		fprintf( file, "%-17s %s\n", "PartNumber", lpAdlBI->strPartNumber );
		fprintf( file, "%-17s %s\n", "Version", lpAdlBI->strVersion );
		fprintf( file, "%-17s %s\n\n", "Date", lpAdlBI->strDate );

	return ADL_OK;//return 0;
}
//显示错误信息
int DisplayErrorAndPause( char * sError )
{
	printf ( "%s",  sError );
	printf ( "\nPress Enter to continue..." );
	getchar();
	return ADL_ERR;
}
//显示帮助信息
void ShowHelp(void)
{
		printf ( "\nADL Utility, Ver 3.0        Copyright(c) 2010 Advanced Micro Devices, Inc.\n\n" );
		printf ( "adlutil get [file]        : Get the display settings of all\n" );//获取 显示设置信息，并输出到文件
		printf ( "                            active adapters [and saves them to file]\n" );
		printf ( "adlutil set file          : Set the display with the parameters from file\n" ); //根据文件设置显示信息
		printf ( "adlutil list [file]       : Get ALL Override Mode settings of all\n" );//获取所有设置模式
		printf ( "                            active adapters [and saves them to file]\n" );
		printf ( "adlutil info file1 [file2]: Find Override Mode, defined in file1\n" );//查找文件中的重载模型
		printf ( "                            [and save the detailed timings in file2]\n" );//存储细节时序到
		printf ( "adlutil over file         : Set Override Display mode from settings in file\n" );//设置重载模型到文件中
		printf ( "adlutil gen [file]        : Generate template to be used by 'over' command\n" );//通用模板
		printf ( "adlutil bios [file]       : Get the Video BIOS information\n" );//获取bios信息
		DisplayErrorAndPause( "" );
}

