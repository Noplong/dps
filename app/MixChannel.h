/**************************************************************

  Copyright (C),WangJinLong, 2017 �����е���ɭ�Ƽ���չ���޹�˾

***************************************************************
 * �� �� ��   : MixChannel.h
 * �� �� ��   : V1.0
 * ��    ��   : WangJinLong
 * ��������   : 2017��7��5��
***************************************************************/
   
#ifndef MIXCHANNEL_H_
#define MIXCHANNEL_H_
  
#ifdef MIXCHANNEL_PRIVATE
    #define MIXCHANNEL_PUBLIC
#else
    #define MIXCHANNEL_PUBLIC extern
#endif

#define RELAY_CTRL_IO        GPIO_Pin_4

#define TEMP_CTRL_IO        GPIO_Pin_0
#define CURRENT_CTRL_IO     GPIO_Pin_5
#define CHECK_CURRENT_IO    GPIO_Pin_1

#define CHANNEL_TYPE_LEAKAGE    0x1000
#define CHANNEL_TYPE_TEMP       0x2000  
#define CHANNEL_TYPE_CURRENT    0x3000
#define CHANNEL_TYPE_IN         0x4000
#define CHANNEL_TYPE_OUT        0x5000

#define CHANNEL_ALARM_VALUE     0x0FFF


#define ADC_STEP_CURRENT                0x01
#define ADC_STEP_TEMPERATURE            0x02
#define ADC_STEP_CHECK_CURRENT          0x03
#define ADC_STEP_CHECK_TEMPERATURE      0x04

#define ADC_CHANGE_START            0x00
#define ADC_CHANGE_DONE             0x01
#define ADC_CHANGE_DOING            0x02

#define CHECK_TIMES            0x03


typedef struct
{
    unsigned char Step;                 //ADC������
    unsigned char InChange;             //ADC��·�����л�    
    unsigned char InPutState[2];
    uint16_t ConvertCount;                //ת����������

} ADC_STATE;

MIXCHANNEL_PUBLIC ADC_STATE AdcState;
MIXCHANNEL_PUBLIC void AdcInit(void); 
MIXCHANNEL_PUBLIC void RelayCtrl(uint8_t Ctrl);

#endif

