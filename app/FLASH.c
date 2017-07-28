/**************************************************************

  Copyright (C),WangJinLong, 2017 �����е���ɭ�Ƽ���չ���޹�˾

***************************************************************
 * �� �� ��   : FLASH.c
 * �� �� ��   : V1.0
 * ��    ��   : WangJinLong
 * ��������   : 2017��7��3��
 * ���Լ�¼  : 2017��7��3��FLASH��дʹ�ÿ⺯���������⣬��ַ����������
***************************************************************/

#define FLASH_PRIVATE

#include "stm8l15x_conf.h"

#define FLASH_OPERATION_STARTADDRESS  (uint32_t)0x1000   /* Flash Operation start address */

void ReadFlashParameter(void)
{
    uint32_t Address;
    uint8_t i;

    Address = FLASH_OPERATION_STARTADDRESS;

    for (i = 0; i < PARAMETER_CONFIG_SIZE; i++)
    {
        ParameterBuffer.DataBuffer_8[i] = *(((PointerAttr uint8_t*)(MemoryAddressCast)Address) + i);
    }
//    for(i = 0;i < 12 ;i++)
//    {
//        RealData.ChannelEventCommit[i][0] = CHANNEL_UNKNOW;
//        RealData.ChannelEventCommit[i][1] = CHANNEL_EVENT_NORMAL;
//    }

    if(ParameterBuffer.ParameterConfig.MainSoftWareVersion != CURRENT_VERSION)//����汾У��
    {
        FactoryReset();
        WWDG_SWReset();
    }
}


void WriteFlashParameter(void)
{
    uint8_t i;
    uint32_t Addr;
    uint32_t Data;
    Addr = FLASH_OPERATION_STARTADDRESS;
    sim();
//    FLASH_DeInit();
    FLASH_SetProgrammingTime(FLASH_ProgramTime_Standard);

    FLASH_Unlock(FLASH_MemType_Data);
    while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET)
    {}
    for (i = 0; i < PARAMETER_CONFIG_SIZE; i++)
    {
        *(((PointerAttr uint8_t*)(MemoryAddressCast)Addr) + i) = *((uint8_t*)(&ParameterBuffer.DataBuffer_8[0]) + i);
    }
    FLASH_Lock(FLASH_MemType_Data);
    rim();
}
void FactoryReset(void)
{
    ParameterBuffer.ParameterConfig.Music = MUSIC_NORMAL;

    ParameterBuffer.ParameterConfig.ChannelAlarmValue[0] = 0x11f4;//500mA
    ParameterBuffer.ParameterConfig.ChannelAlarmValue[1] = 0x2046;//500mA
//    ParameterBuffer.ParameterConfig.ChannelAlarmValue[2] = 0x1000;//500mA
//    ParameterBuffer.ParameterConfig.ChannelAlarmValue[3] = 0x1000;//500mA
//    ParameterBuffer.ParameterConfig.ChannelAlarmValue[4] = 0x1000;//500mA
//    
//    ParameterBuffer.ParameterConfig.ChannelAlarmValue[5] = 0x3000;//500mA
//    ParameterBuffer.ParameterConfig.ChannelAlarmValue[6] = 0x3000;
//    ParameterBuffer.ParameterConfig.ChannelAlarmValue[7] = 0x3000;//500mA
    
    ParameterBuffer.ParameterConfig.ChannelAlarmValue[2] = 0x11f4;//500mA
    ParameterBuffer.ParameterConfig.ChannelAlarmValue[3] = 0x11f4;//500mA
    ParameterBuffer.ParameterConfig.ChannelAlarmValue[4] = 0x11f4;//500mA
    
    ParameterBuffer.ParameterConfig.ChannelAlarmValue[5] = 0x3064;//500mA
    ParameterBuffer.ParameterConfig.ChannelAlarmValue[6] = 0x3064;
    ParameterBuffer.ParameterConfig.ChannelAlarmValue[7] = 0x3064;//500mA

    ParameterBuffer.ParameterConfig.ChannelAdParameter[0] = 235;//200mAʱADֵ
    ParameterBuffer.ParameterConfig.ChannelAdParameter[1] = 990;//800mAʱADֵ
    
    ParameterBuffer.ParameterConfig.ChannelAdParameter[2] = 235;//1AʱADֵ
    ParameterBuffer.ParameterConfig.ChannelAdParameter[3] = 2656;//5AʱADֵ
    memcpy(ParameterBuffer.ParameterConfig.ConnectIP, "123.207.71.208,6666\r\n", 23);
//    memcpy(ParameterBuffer.ParameterConfig.ConnectIP, "115.28.103.131,40000\r\n", 23);
    ParameterBuffer.ParameterConfig.PackageHead_From  =1234;
    ParameterBuffer.ParameterConfig.PackageHead_To  =1000;

//    ParameterBuffer.ParameterConfig.ChannelRatioIn[0] = 100;//100:5
//    ParameterBuffer.ParameterConfig.ChannelRatioIn[1] = 100;
//    ParameterBuffer.ParameterConfig.ChannelRatioIn[2] = 100;

//    ParameterBuffer.ParameterConfig.ChannelRatioOut[0] = 5;//100:5
//    ParameterBuffer.ParameterConfig.ChannelRatioOut[1] = 5;
//    ParameterBuffer.ParameterConfig.ChannelRatioOut[2] = 5;    


//    ParameterBuffer.ParameterConfig.ChannelAdParameter[0] = 892;
//    ParameterBuffer.ParameterConfig.ChannelAdParameter[1] = 892;
//    ParameterBuffer.ParameterConfig.ChannelAdParameter[2] = 892;
//    ParameterBuffer.ParameterConfig.ChannelAdParameter[3] = 10;
//    ParameterBuffer.ParameterConfig.ChannelAdParameter[4] = 10;
//    ParameterBuffer.ParameterConfig.ChannelAdParameter[5] = 10;
//    ParameterBuffer.ParameterConfig.ChannelAdParameter[6] = 10;
//    ParameterBuffer.ParameterConfig.ChannelAdParameter[7] = 10;
//    ParameterBuffer.ParameterConfig.ChannelAdParameter[8] = 10;

    ParameterBuffer.ParameterConfig.MainSoftWareVersion = CURRENT_VERSION;

    WriteFlashParameter();
}



