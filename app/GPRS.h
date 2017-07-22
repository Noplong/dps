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

GPRS_PUBLIC uint8_t GprsRxBuf[512];


GPRS_PUBLIC void Gprs_Init(void);  
GPRS_PUBLIC void ConnectToServer(void);

#endif

