/**************************************************************

  Copyright (C),WangJinLong, 2017 深圳市德派森科技发展有限公司

***************************************************************
 * 文 件 名   : GPRS.h
 * 版 本 号   : V1.0
 * 作    者   : WangJinLong
 * 生成日期   : 2017年7月5日
***************************************************************/

#ifndef GPRS_H_
#define GPRS_H_

#ifdef GPRS_PRIVATE
#define GPRS_PUBLIC
#else
#define GPRS_PUBLIC extern
#endif

#define GPRS_DCD_PORT       GPIOD
#define GPRS_DCD_PIN        GPIO_Pin_4

//应用数据单元-类型标志定义
#define SERVER_COMMAND_DEVICE_STATUS    65 //上传设备状态
#define SERVER_COMMAND_SIM_ICCID        66 //查询或者上传SIM卡CCID
#define SERVER_COMMAND_DEVICE_CONFIG    67 //设置阈值
#define SERVER_COMMAND_DEVICE_RESET     128 //复位命令
#define SERVER_COMMAND_DEVICE_MUTE      129 //消声命令

//应用数据单元-系统类型代码
#define SYSTEM_TYPE    10 //电气火灾监控系统

//应用数据单元-设备类型代码
#define DEVICE_TYPE_12_CHANNEL    8 //DDH5267物联网模块


#define GPRS_COMMAND_PREPARE         01 //网络连接准备
#define GPRS_COMMAND_SINGAL         02 //SIM卡有无及信号强度
#define GPRS_COMMAND_CGATT         03 //GPRS附着
#define GPRS_COMMAND_CONNECT_IP        04//使用IP建立TCP连接
#define GPRS_COMMAND_HEARTBEAT        05//发送心跳
#define GPRS_COMMAND_RECVPROCESS        06//接收包处理
#define GPRS_COMMAND_RECV_TIMEOUT        07//接收包超时处理
#define GPRS_COMMAND_CCID        8//读设备CCID


#define GPRS_COMMAND_SPECIAL         20 //特殊字符处理


#define AT_CREG         01 //网络注册信息
#define AT_CGREG        02 //GPRS网络注册状态
#define AT_CGATT        03 //附着和分离GPRS业务
#define AT_CIPMUX       04 //配置TCPIP连接协议方式，单路还是多路方式
#define AT_CIPMODE      05 //配置TCPIP应用模式，透明还是非透明模式
#define AT_CSTT         06 //启动任务并设置移动接入点APN、用户名、密码
#define AT_CIICR        07 //激活移动场景，发起GPRS 或CSD无线连接
#define AT_CIPSHUT      8 //关闭移动场景
#define AT_CIPSCONT     9 //保存TCPIP应用场景
#define AT_CIPSTART     10 //建立TCP或UDP协议数据连接
#define AT_CIPSEND      11 //发送数据
#define AT_CIPCLOSE     12 //关闭TCP或UDP连接
#define AT_CIFSR        13 //查询本地IP地址
#define AT_CIPSTATUS    14 //查询当前连接状态
#define AT_CDNSCFG      15 //配置域名解析服务器
#define AT_CIPSERVER    16 //配置为服务器
#define AT_CIPCCFG      17 //配置透明传输参数
#define AT_CLPORT       18 //设置本地端口
#define AT_CIPHEAD      19 //设置接收数据是否显示IP头
#define AT_CIPATS       20 //设置自动发送数据的时间
#define AT_CIPSPRT      21 //设置发送数据时是否显示‘>’ 和send ok 
#define AT_CIPCSGP      22 //设置为CSD或GPRS连接模式
#define AT_CIPSRIP      23 //设置接收数据时是否显示发送方的IP地址和端口号
#define AT_CIPDPDP      24 //设置是否检查GPRS网络状态
#define AT_CIPSHOWTP    25 //设置是否接收数据时在IP头显示传输协议 
#define AT_RST    26 //设置是否接收数据时在IP头显示传输协议 
#define AT_CPIN         27 //SIM卡状态
#define AT_ATE         28 //回显控制(ATE0-关闭回显，ATE1-开启回显)
#define AT_CSQ         29 //网络信号强度
#define AT_CGCLASS         30 //移动台类别
#define AT_CGDCONT        31 //连接方式
#define AT_IPR        32 //波特率同步

#define AT_CCID      33//读SIM卡串号 


#define BUF_SIZE 257



GPRS_PUBLIC void Gprs_Init(void);
GPRS_PUBLIC void ConnectToServer(void);

typedef struct tagStatus
{
    uint8_t AT_CPIN_Flag: 1; //SIM卡状态
    uint8_t AT_CGATT_Flag: 1; //GPRS附着状态
    uint8_t GPRS_AbleToUse: 1; //GPRS模块是否可以正常通讯
    uint8_t UartContectState: 1; //串口连接状态
    uint8_t AT_CSTT_Flag: 1; //APN设置
    uint8_t GRST_Flag: 1; //GPRS模块复位
    uint8_t ServerContectState: 2;//服务器链接状态

} Status_t, *pStatus_t;

typedef union
{
    Status_t Status_bit;
    uint8_t Status_u8;

} STATUS_BUFF;

//数据包上传状态定义
#define DATAPACKAGE_STARTING        0x0000  //GPRS模块正在启动
#define DATAPACKAGE_WAIT_CONNECT    0x1000  //等待服务器连接成功
#define DATAPACKAGE_READY_TO_SEND   0x2000  //可以上传数据
#define DATAPACKAGE_WAIT_ACK        0x3000  //等待服务器应答
#define DATAPACKAGE_UPLOAD_DONE     0x4000  //数据上传完成



typedef struct tagGprsInfo
{
    STATUS_BUFF Status;
    uint8_t AT_CSQ_Data;//信号强度
    uint8_t Sim_Iccid[10];//设备ICCID  
    uint16_t HeartBeat;//心跳计时
    uint16_t CommandCount;//业务流水号，高四位代表数据传输状态

} GprsInfo_t, *pGprsInfo_t;
GPRS_PUBLIC GprsInfo_t GprsInfo;

#define DATA_PACKAGE_EMPTY      0x00//数据包为空
#define DATA_PACKAGE_RECEVING   0x01//数据包正在接收
#define DATA_PACKAGE_READY      0x02//数据包接收完成


typedef struct
{
    uint8_t DataPackage_Start;          //1字节启动符0x7E
    uint8_t DataPackage_MsgID_L;         //2字节业务流水号(低字节在前)
    uint8_t DataPackage_MsgID_H;
    
    uint8_t DataPackage_Version_L;       //2字节协议版本号(低字节在前)
    uint8_t DataPackage_Version_H;

    uint8_t DataPackage_MsgFrom_Low_L;   ////6字节源地址(低字节在前)
    uint8_t DataPackage_MsgFrom_Low_H;
    uint8_t DataPackage_MsgFrom_Mid_L;   //
    uint8_t DataPackage_MsgFrom_Mid_H;   //    
    uint8_t DataPackage_MsgFrom_High_L;  //
    uint8_t DataPackage_MsgFrom_High_H;  //

    uint8_t DataPackage_MsgTo_Low_L;     ////6字节目的地址(低字节在前)
    uint8_t DataPackage_MsgTo_Low_H;
    uint8_t DataPackage_MsgTo_Mid_L;     //
    uint8_t DataPackage_MsgTo_Mid_H;     //
    uint8_t DataPackage_MsgTo_High_L;    //
    uint8_t DataPackage_MsgTo_High_H;    //

    uint8_t DataPackage_DataLength_L;    //2字节应用单元数据长度
    uint8_t DataPackage_DataLength_H;
    
    uint8_t  DataPackage_Command;       //1字节命令字节

    uint8_t DataPackage_Data[233];      //233字节应用单元数据

    uint8_t DataPackage_CRCL;           //1字节CRC校验低字节
    uint8_t DataPackage_CRCH;           //1字节CRC校验高字节
    uint8_t DataPackage_Stop;           //1字节结束符0x7E
    uint8_t DataPackage_State;//数据包状态
} DATA_FORMAT;

typedef union _Buffer
{
    DATA_FORMAT DataBuff;
    unsigned char DataBufferChar[BUF_SIZE];
} USART_DATA_BUFFER;

GPRS_PUBLIC USART_DATA_BUFFER GprsRxBuf[2];//接收缓冲区
GPRS_PUBLIC USART_DATA_BUFFER GprsTxBuf;


GPRS_PUBLIC void QueueGprsPut(uint8_t command);
GPRS_PUBLIC void HeartBeat_Timer(void);

#endif

