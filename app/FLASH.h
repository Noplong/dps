/**************************************************************

  Copyright (C),WangJinLong, 2017 �����е���ɭ�Ƽ���չ���޹�˾

***************************************************************
 * �� �� ��   : FLASH.h
 * �� �� ��   : V1.0
 * ��    ��   : WangJinLong
 * ��������   : 2017��7��3��
***************************************************************/

#ifndef FLASH_H_
#define FLASH_H_

#ifdef FLASH_PRIVATE
#define FLASH_PUBLIC
#else
#define FLASH_PUBLIC extern
#endif

#define CHANNEL_NUM 8

//�¼�����
#define CHANNEL_NORMAL      0x00
#define CHANNEL_ALARM       (0x01 << 3)
#define CHANNEL_PREALARM    (0x01 << 2)
#define CHANNEL_OPEN        (0x01 << 1)
#define CHANNEL_SHORT       (0x01 << 0)

#define CHANNEL_UNKNOW                  (0x0F)
#define CHANNEL_FAULT                   (0x03 << 0)

typedef struct
{
    unsigned char SysWorkMode;                      //1BYTES  ϵͳ����ģʽ
    unsigned char SysRunState;                      //1BYTES  ϵͳ����״̬
    unsigned char SysDebug;                      //1BYTES  ϵͳ����-��ӡ������Ϣ
    unsigned char SysAutoCalibrate;                      //1BYTES  ϵͳ�Զ�����
    unsigned char AdcConvertState;                      //1BYTES  ϵͳ����ģʽ
    unsigned char AdcConvertCount;
    unsigned char KeyValue;
    unsigned char AlarmCount;
    unsigned char FaultCount;

    unsigned char InputCheck[3];//������:[0]��Դ����1  [1]��Դ����2 [2]��Դ����3

    uint16_t ChannelValue[CHANNEL_NUM]; //ͨ������
    uint8_t ChannelState[CHANNEL_NUM]; //ͨ��״̬
    uint16_t ChannelEvent[CHANNEL_NUM];//ͨ���¼�
} PARAMETER_STATUS;
FLASH_PUBLIC PARAMETER_STATUS ParameterSysStatus;

//���ô洢�ռ��С
#define PARAMETER_CONFIG_SIZE  80


//����汾
#define CURRENT_VERSION   01


#define MUSIC_MUTE          0x01
#define MUSIC_NORMAL        0x02




typedef struct
{
    unsigned char Music;                        //����ģʽ
    
    uint16_t PackageHead_From;//���ݰ�ͷ_Դ��ַ
    uint16_t PackageHead_To;//���ݰ�ͷ_Ŀ�ĵ�ַ
    uint8_t ConnectIP[23];//ͨѶ��վIP���˿�

    uint16_t ChannelRatioIn[3];//����ͨ����� ChannelRatioIn[] �� ChannelRatioOut
    unsigned char ChannelRatioOut[3];
    uint16_t ChannelAlarmValue[CHANNEL_NUM]; //������ֵ
    uint16_t ChannelAdParameter[CHANNEL_NUM];//ͨ���궨ֵ:��ѹͨ��220V�궨������ͨ��1A�궨

    unsigned int   ProductID;                       //4BYTES��ƷID
    unsigned short MainSoftWareVersion;             //2BYTESCPU����汾

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

