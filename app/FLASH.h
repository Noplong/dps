/**************************************************************

  Copyright (C),WangJinLong, 2017 深圳市德派森科技发展有限公司

***************************************************************
 * 文 件 名   : FLASH.h
 * 版 本 号   : V1.0
 * 作    者   : WangJinLong
 * 生成日期   : 2017年7月3日
***************************************************************/

#ifndef FLASH_H_
#define FLASH_H_

#ifdef FLASH_PRIVATE
#define FLASH_PUBLIC
#else
#define FLASH_PUBLIC extern
#endif

#define CHANNEL_NUM 8

//事件类型
#define CHANNEL_NORMAL      0x00
#define CHANNEL_ALARM       (0x01 << 3)
#define CHANNEL_PREALARM    (0x01 << 2)
#define CHANNEL_OPEN        (0x01 << 1)
#define CHANNEL_SHORT       (0x01 << 0)

#define CHANNEL_UNKNOW                  (0x0F)
#define CHANNEL_FAULT                   (0x03 << 0)

typedef struct
{
    unsigned char SysWorkMode;                      //1BYTES  系统工作模式
    unsigned char SysRunState;                      //1BYTES  系统运行状态
    unsigned char SysDebug;                      //1BYTES  系统调试-打印调试信息
    unsigned char SysAutoCalibrate;                      //1BYTES  系统自动调参
    unsigned char AdcConvertState;                      //1BYTES  系统工作模式
    unsigned char AdcConvertCount;
    unsigned char KeyValue;
    unsigned char AlarmCount;
    unsigned char FaultCount;

    unsigned char InputCheck[3];//输入检测:[0]有源输入1  [1]有源输入2 [2]无源输入3

    uint16_t ChannelValue[CHANNEL_NUM]; //通道数据
    uint8_t ChannelState[CHANNEL_NUM]; //通道状态
    uint16_t ChannelEvent[CHANNEL_NUM];//通道事件
} PARAMETER_STATUS;
FLASH_PUBLIC PARAMETER_STATUS ParameterSysStatus;

//配置存储空间大小
#define PARAMETER_CONFIG_SIZE  80


//软件版本
#define CURRENT_VERSION   01


#define MUSIC_MUTE          0x01
#define MUSIC_NORMAL        0x02




typedef struct
{
    unsigned char Music;                        //音响模式
    
    uint16_t PackageHead_From;//数据包头_源地址
    uint16_t PackageHead_To;//数据包头_目的地址
    uint8_t ConnectIP[23];//通讯主站IP及端口

    uint16_t ChannelRatioIn[3];//电流通道变比 ChannelRatioIn[] 比 ChannelRatioOut
    unsigned char ChannelRatioOut[3];
    uint16_t ChannelAlarmValue[CHANNEL_NUM]; //报警阈值
    uint16_t ChannelAdParameter[CHANNEL_NUM];//通道标定值:电压通道220V标定，电流通道1A标定

    unsigned int   ProductID;                       //4BYTES产品ID
    unsigned short MainSoftWareVersion;             //2BYTESCPU软件版本

} PARAMETER_SYSCONFIG;

typedef union _DataBuffer
{
    PARAMETER_SYSCONFIG ParameterConfig;
    unsigned char DataBuffer_8[PARAMETER_CONFIG_SIZE];
} PARAMETER_DATA_BUFFER;


FLASH_PUBLIC PARAMETER_DATA_BUFFER ParameterBuffer;



FLASH_PUBLIC void ReadFlashParameter(void);
FLASH_PUBLIC void WriteFlashParameter(void);
FLASH_PUBLIC void FactoryReset(void);

#endif

