/**************************************************************

  Copyright (C),WangJinLong, 2017 深圳市德派森科技发展有限公司

***************************************************************
 * 文 件 名   : MixChannel.c
 * 版 本 号   : V1.0
 * 作    者   : WangJinLong
 * 生成日期   : 2017年7月5日
***************************************************************/

#define MIXCHANNEL_PRIVATE

#include "stm8l15x_conf.h"
#include "atomsem.h"
#include "atomqueue.h"
#include "temp.h"

#define DMA_BUF_SIZE 11
#define ADC1_DR_Address    ((uint16_t)(0x5344) )//ADC1外设地址
unsigned short ADC_ConvertedValue[DMA_BUF_SIZE];
uint16_t ADC[9] = {ADC_Channel_17, ADC_Channel_16, ADC_Channel_15,
                   ADC_Channel_14, ADC_Channel_13, ADC_Channel_12,
                   ADC_Channel_11, ADC_Channel_10, ADC_Channel_9
                  };

static ATOM_TCB AdcTask_Tcb;
NEAR static uint8_t Adc_thread_stack[ADC_TASK_SIZE_BYTES];

static ATOM_SEM Sem_Adc;


static void EventProcess(uint8_t Channel, uint8_t Event);
static void ADCTask(uint32_t param);
static void AdcFunChange(void);
static void ADC_TimerStart(void);
static void ADC_TimerStop(void);


#define ARR_LEN     10
#define ARR_HEAD    0 //(ArrayCount+ARR_LEN+1)%ARR_LEN
#define ARR_END    (ARR_LEN-1) //
static uint8_t      ArrayCount;
volatile uint8_t    TempArray[8][ARR_LEN];


uint8_t AdChannel[10] = {4, 3, 2, 8, 9 //混合通道C0-C4
                         , 7, 6, 5 //电流通道I1-I3
                         , 0, 1
                        };//无缘输入通道Singal1-Singal2
float Leakage_K;
float Leakage_B;
float Current_K;
float Current_B;

void AdcInit(void)
{
    uint8_t status;
    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);//使能ADC时钟
    CLK_PeripheralClockConfig(CLK_Peripheral_DMA1, ENABLE);//使能DMA1时钟
    ADC_DeInit(ADC1);
    ADC_VrefintCmd(ENABLE);//使能内部参考电压

    ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_12Bit, ADC_Prescaler_1);
    ADC_SamplingTimeConfig(ADC1, ADC_Group_SlowChannels, ADC_SamplingTime_24Cycles);
    ADC_SamplingTimeConfig(ADC1, ADC_Group_FastChannels, ADC_SamplingTime_24Cycles);

    ADC_ChannelCmd(ADC1, ADC_Channel_Vrefint, ENABLE);
    ADC_SchmittTriggerConfig(ADC1, ADC_Channel_13, DISABLE);
    ADC_SchmittTriggerConfig(ADC1, ADC_Channel_12, DISABLE);
    ADC_SchmittTriggerConfig(ADC1, ADC_Channel_11, DISABLE);
    ADC_SchmittTriggerConfig(ADC1, ADC_Channel_24, DISABLE);
    ADC_SchmittTriggerConfig(ADC1, ADC_Channel_25, DISABLE);
    ADC_SchmittTriggerConfig(ADC1, ADC_Channel_16, DISABLE);
    ADC_SchmittTriggerConfig(ADC1, ADC_Channel_15, DISABLE);
    ADC_SchmittTriggerConfig(ADC1, ADC_Channel_14, DISABLE);
    ADC_SchmittTriggerConfig(ADC1, ADC_Channel_7, DISABLE);
    ADC_SchmittTriggerConfig(ADC1, ADC_Channel_8, DISABLE);

    ADC_ChannelCmd(ADC1, ADC_Channel_13, ENABLE);
    ADC_ChannelCmd(ADC1, ADC_Channel_12, ENABLE);
    ADC_ChannelCmd(ADC1, ADC_Channel_11, ENABLE);
    ADC_ChannelCmd(ADC1, ADC_Channel_24, ENABLE);
    ADC_ChannelCmd(ADC1, ADC_Channel_25, ENABLE);
    ADC_ChannelCmd(ADC1, ADC_Channel_16, ENABLE);
    ADC_ChannelCmd(ADC1, ADC_Channel_15, ENABLE);
    ADC_ChannelCmd(ADC1, ADC_Channel_14, ENABLE);
    ADC_ChannelCmd(ADC1, ADC_Channel_7, ENABLE);
    ADC_ChannelCmd(ADC1, ADC_Channel_8, ENABLE);

    ADC_Cmd(ADC1, ENABLE); //打开ADC，10各通道全部转换完成一次耗时50微秒

    //DMA配置
    SYSCFG_REMAPDMAChannelConfig(REMAP_DMA1Channel_ADC1ToChannel0);//ADC通道要remap
    DMA_Init(DMA1_Channel0, (uint32_t)&ADC_ConvertedValue[0],
             ADC1_DR_Address,
             DMA_BUF_SIZE,
             DMA_DIR_PeripheralToMemory,
             DMA_Mode_Circular,
             DMA_MemoryIncMode_Inc,
             DMA_Priority_High,
             DMA_MemoryDataSize_HalfWord);
    DMA_Cmd(DMA1_Channel0, ENABLE);
    DMA_ITConfig(DMA1_Channel0, DMA_ITx_TC, ENABLE);
    /* DMA enable */
    DMA_GlobalCmd(ENABLE);
    ADC_DMACmd(ADC1, ENABLE);

    if (atomSemCreate (&Sem_Adc, 0) != ATOM_OK)
    {
        printf ("Sem_Adc creat fail\n");
        return;
    }

    status = atomThreadCreate(&AdcTask_Tcb,
                              ADC_TASK_PRIO, ADCTask, 0,
                              &Adc_thread_stack[ADC_TASK_SIZE_BYTES - 1],
                              ADC_TASK_SIZE_BYTES);

    GPIO_Init(GPIOB, GPIO_Pin_1, GPIO_Mode_Out_PP_High_Slow); //测试点

    Leakage_K = (float)(800 - 200) / (float)(ParameterBuffer.ParameterConfig.ChannelAdParameter[1] - ParameterBuffer.ParameterConfig.ChannelAdParameter[0]);
    Leakage_B = (float)(200 - (Leakage_K * ParameterBuffer.ParameterConfig.ChannelAdParameter[0]));

    Current_K = (float)(5 - 1) / (float)(ParameterBuffer.ParameterConfig.ChannelAdParameter[3] - ParameterBuffer.ParameterConfig.ChannelAdParameter[2]);
    Current_B = (float)(5 - (Current_K * ParameterBuffer.ParameterConfig.ChannelAdParameter[3]));

    AdcState.ConvertCount = 0;
    AdcState.InChange = ADC_CHANGE_START;
    AdcState.Step = ADC_STEP_CHECK_CURRENT;
    AdcFunChange();
    ADC_TimerStart();

}

uint16_t GetTemperature(uint32_t *CurrentValue)
{
    uint16_t TempValue;
    uint8_t i;
    TempValue = *CurrentValue;
    if((TempValue < 140) || (TempValue > 3080))
    {
        return 0;
    }
    for(i = 0; i < 180; i++)
    {
        if(TempValue <= temperature[i][1] && TempValue >= temperature[i + 1][1])
        {
            break;
        }
    }
    TempValue = temperature[i + 1][0];
    if(i < 20)                                          //温度在0-150之间
    {
        TempValue = 0;
    }
    else if(i >= 170)
    {
        TempValue = 150;
    }
    return TempValue;

}
/*---------------------------------------------------------------------------------------------------------*/
/*差温算法                                                                                                 */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t isForwardAlarm(uint8_t ChannelCount, uint16_t AlarmThreshold)
{
    uint8_t i;
    uint8_t Count = 0;

    for (i = 0; i < ARR_LEN; i++)
    {
        if((TempArray[ChannelCount][i] + 1) > AlarmThreshold)//十组数据全部大于阈值，则报警
        {
            Count++;
            if(Count > 8)
            {
                return(1);
            }
        }
    }
    if (   TempArray[ChannelCount][ARR_END] > 55                            //最晚点
           || TempArray[ChannelCount][ARR_END]  == 0
           || TempArray[ChannelCount][ARR_HEAD]  < 38)     //最早点
    {
        return(0);
    }

    for (i = 1; i < (ARR_LEN - 1); i++)
    {
        if ( TempArray[ChannelCount][i] <= TempArray[ChannelCount][ARR_END])
        {
            return(0);
        }
    }

    ArrayCount = 20;
    for(i = 0; i < (ARR_LEN - 2); i++)
    {
        if(TempArray[ChannelCount][i] > TempArray[ChannelCount][i + 1])
        {
            ArrayCount ++;
        }
        else if(TempArray[ChannelCount][i] < TempArray[ChannelCount][i + 1])
        {
            ArrayCount --;
        }
    }

    if ((ArrayCount > 27) && (TempArray[ChannelCount][ARR_HEAD] - TempArray[ChannelCount][ARR_END])
        > TemputeratureDifferenceTable[AlarmThreshold - 45])
    {
        return (1);
    }
    return(0);
}

static void ADCTask(uint32_t param)
{
    uint8_t status, Channel, i;
    uint16_t AlarmValue;
    uint32_t TempTotal[CHANNEL_NUM];
    static uint8_t GetTemp = 0;
    static uint16_t ChannelADMark[CHANNEL_NUM];//高四位代表报警次数，低12为代表ADMax
    static uint16_t ChannelADNow[CHANNEL_NUM];//此次AD最大值
    while(1)
    {
//        GPIO_SetBits(GPIOB, GPIO_Pin_1);
        status = atomSemGet(&Sem_Adc, 1000);//等待1S
        if(status == ATOM_TIMEOUT)
        {
            atomSemResetCount(&Sem_Adc, 0);
            ADC_TimerStop();
            DMA_ClearITPendingBit(DMA1_IT_TC0);
            TIM3_ClearFlag(TIM3_FLAG_Update);
            ADC_TimerStart();
            continue;
        }
//        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        atomSemResetCount(&Sem_Adc, 0);

        if(AdcState.InChange == ADC_CHANGE_START)
        {
            AdcFunChange();
            ADC_TimerStart();
        }
        else
        {
            if(AdcState.InChange == ADC_CHANGE_DOING)
            {
                AdcState.ConvertCount = 0;
                AdcState.InChange = ADC_CHANGE_DONE;
            }
            switch(AdcState.Step)
            {
                case ADC_STEP_TEMPERATURE:
                case ADC_STEP_CHECK_TEMPERATURE:
                    if(AdcState.ConvertCount == 200)
                    {
                        if(GetTemp == 0)
                        {
                            GetTemp = 1;
                            break;
                        }
                        GetTemp = 0;
                    }

                    for(Channel = 0; Channel < 5; Channel++)
                    {
                        AlarmValue = ParameterBuffer.ParameterConfig.ChannelAlarmValue[Channel];
                        status = ParameterSysStatus.ChannelState[Channel];
                        ADC_ConvertedValue[AdChannel[Channel]] = (ADC_ConvertedValue[AdChannel[Channel]] % 4095);
                        if(((AlarmValue & CHANNEL_TYPE_TEMP) == CHANNEL_TYPE_TEMP) //是温度通道
                           && ((AlarmValue & CHANNEL_ALARM_VALUE) != 0) //通道在线
                           && ((status & CHANNEL_UNKNOW) != CHANNEL_UNKNOW))//通道状态确定
                        {
                            if(AdcState.ConvertCount <= 2)
                            {
                                TempTotal[Channel] = 0;
                                ChannelADNow[Channel] = ADC_ConvertedValue[AdChannel[Channel]];
                            }
                            else if(AdcState.ConvertCount <= 199) //
                            {
                                if(ADC_ConvertedValue[AdChannel[Channel]] > ChannelADNow[Channel])
                                {
                                    ChannelADNow[Channel] = ADC_ConvertedValue[AdChannel[Channel]];
                                }
                                TempTotal[Channel] += ADC_ConvertedValue[AdChannel[Channel]];
                            }
                            else
                            {
//                                AdcState.ConvertCount = 200;
//                                GPIO_SetBits(GPIOB, GPIO_Pin_1);
                                TempTotal[Channel] = ((TempTotal[Channel] - ChannelADNow[Channel]) / 196);
                                status = GetTemperature(&TempTotal[Channel]);
                                ParameterSysStatus.ChannelValue[Channel] = status;
                                if((ChannelADMark[Channel] & 0x0FFF) == 0 && status == 0)
                                {
                                    ChannelADMark[Channel] += 0x1000;

                                    if((ChannelADMark[Channel] >> 12) >= CHECK_TIMES)
                                    {
                                        if(TempTotal[Channel] >= 2600)
                                        {
                                            EventProcess(Channel, CHANNEL_OPEN);

                                        }
                                        else
                                        {
                                            EventProcess(Channel, CHANNEL_SHORT);
                                        }
                                        ChannelADMark[Channel] = ((ChannelADMark[Channel] & 0x0FFF) | status);
                                        continue;
                                    }
                                }

                                if((ParameterSysStatus.ChannelState[Channel] & CHANNEL_FAULT) != 0x00)//有故障
                                {
                                    if((ChannelADMark[Channel] & 0x0FFF) == 0 && status != 0)
                                    {
                                        ChannelADMark[Channel] = ((ChannelADMark[Channel] & 0x0FFF) | status);
                                        continue;
                                    }
                                    else if((ChannelADMark[Channel] & 0x0FFF) != 0 && status != 0)
                                    {
                                        ChannelADMark[Channel] += 0x1000;
                                        if((ChannelADMark[Channel] >> 12) >= CHECK_TIMES)
                                        {
                                            ChannelADMark[Channel] &= 0x0FFF;
                                            EventProcess(Channel, CHANNEL_NORMAL);
                                        }
                                        ChannelADMark[Channel] = ((ChannelADMark[Channel] & 0xF000) | status);
                                        continue;
                                    }
                                }

                                ChannelADMark[Channel] = status;

                                for(i = 0; i < ARR_LEN - 1; i++)
                                {
                                    TempArray[Channel][ARR_LEN - 1 - i] = TempArray[Channel][ARR_LEN - 1 - i - 1];
                                }
                                TempArray[Channel][0] = status;

                                if(isForwardAlarm(Channel, AlarmValue & 0x0fff))
                                {
                                    EventProcess(Channel, CHANNEL_ALARM);
                                }
//                                GPIO_ResetBits(GPIOB, GPIO_Pin_1);
                            }
                        }
                    }

                    break;
                case ADC_STEP_CURRENT:

                    for(Channel = 0; Channel < 8; Channel++)
                    {
                        AlarmValue = ParameterBuffer.ParameterConfig.ChannelAlarmValue[Channel];
                        status = ParameterSysStatus.ChannelState[Channel];
                        ADC_ConvertedValue[AdChannel[Channel]] = (ADC_ConvertedValue[AdChannel[Channel]] % 4095);

                        if((AlarmValue & CHANNEL_TYPE_TEMP) != CHANNEL_TYPE_TEMP //不是温度通道
                           && (AlarmValue & CHANNEL_ALARM_VALUE) != 0 //通道在线
                           && (status & CHANNEL_UNKNOW) != CHANNEL_UNKNOW//通道状态确定
                           && (status & CHANNEL_FAULT) == 0x00 ) //通道无故障
                        {
                            if(AdcState.ConvertCount <= 2)
                            {

                                ChannelADNow[Channel] = ADC_ConvertedValue[AdChannel[Channel]];
                            }
                            else if(AdcState.ConvertCount <= 199)//
                            {
                                if(ADC_ConvertedValue[AdChannel[Channel]] > ChannelADNow[Channel])
                                {
                                    ChannelADNow[Channel] = ADC_ConvertedValue[AdChannel[Channel]];
                                }
                            }
                            else
                            {
                                if((ChannelADMark[Channel] & 0xF000) == 0xF000)//重新找最大值
                                {
                                    ChannelADMark[Channel] = (0x0FFF & ChannelADNow[Channel]);
                                }
                                else
                                {
                                    if((((ChannelADMark[Channel] & 0x0FFF) <= ChannelADNow[Channel]) && ((ChannelADNow[Channel] - (ChannelADMark[Channel] & 0x0FFF)) < 100))
                                       || ((ChannelADNow[Channel] <= (ChannelADMark[Channel] & 0x0FFF)) && (((ChannelADMark[Channel] & 0x0FFF) - ChannelADNow[Channel]) < 100)))
                                    {
                                        ChannelADMark[Channel] &= 0xF000;
                                        ChannelADMark[Channel] |= (0x0FFF & ChannelADNow[Channel]);
                                        if((AlarmValue & CHANNEL_TYPE_LEAKAGE) == CHANNEL_TYPE_LEAKAGE)
                                        {
                                            ParameterSysStatus.ChannelValue[Channel] = (ChannelADNow[Channel] * Leakage_K + Leakage_B);
                                            if(ParameterSysStatus.ChannelValue[Channel] > 999)
                                            {
                                                ParameterSysStatus.ChannelValue[Channel] = 999;
                                            }
                                            if(ParameterSysStatus.ChannelValue[Channel] >= ((AlarmValue & 0x0fff) * 99 / 100))
                                            {
                                                ChannelADMark[Channel] += 0x1000;
                                                if((ChannelADMark[Channel] >> 12) >= CHECK_TIMES)
                                                {
                                                    EventProcess(Channel, CHANNEL_ALARM);
                                                    ChannelADMark[Channel] &= 0x0FFF;
                                                }
                                            }
                                        }
                                        else if((AlarmValue & CHANNEL_TYPE_CURRENT) == CHANNEL_TYPE_CURRENT)
                                        {
                                            ParameterSysStatus.ChannelValue[Channel] = (ChannelADNow[Channel] * Current_K + Current_B) * 10;
                                            if(ParameterSysStatus.ChannelValue[Channel] > 999)
                                            {
                                                ParameterSysStatus.ChannelValue[Channel] = 999;
                                            }

                                            if(ParameterSysStatus.ChannelValue[Channel] >= ((AlarmValue & 0x0fff) * 990 / 100))
                                            {
                                                ChannelADMark[Channel] += 0x1000;
                                                if((ChannelADMark[Channel] >> 12) >= CHECK_TIMES)
                                                {
                                                    EventProcess(Channel, CHANNEL_ALARM);
                                                    ChannelADMark[Channel] &= 0x0FFF;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        ChannelADMark[Channel] = 0xF000;
                                    }
                                }

                            }
                        }
                    }
                    break;
                case ADC_STEP_CHECK_CURRENT:

                    for(Channel = 0; Channel < 8; Channel++)
                    {
                        if(ParameterSysStatus.ChannelValue[Channel] < 50)//漏电流小于50mA时才开始检测故障
                        {
                            AlarmValue = ParameterBuffer.ParameterConfig.ChannelAlarmValue[Channel];
                            status = ParameterSysStatus.ChannelState[Channel];
                            ADC_ConvertedValue[AdChannel[Channel]] = (ADC_ConvertedValue[AdChannel[Channel]] % 4095);

                            if((AlarmValue & CHANNEL_TYPE_TEMP) != CHANNEL_TYPE_TEMP //不是温度通道
                               && (AlarmValue & CHANNEL_ALARM_VALUE) != 0) //通道在线
                            {
                                if(AdcState.ConvertCount <= 10)
                                {
                                    TempTotal[Channel] = 0;
                                    ChannelADNow[Channel] = ADC_ConvertedValue[AdChannel[Channel]];
                                }
                                else if(AdcState.ConvertCount <= 51) //
                                {
                                    if(ADC_ConvertedValue[AdChannel[Channel]] > ChannelADNow[Channel])
                                    {
                                        ChannelADNow[Channel] = ADC_ConvertedValue[AdChannel[Channel]];
                                    }
                                    TempTotal[Channel] += ADC_ConvertedValue[AdChannel[Channel]];
                                }
                                else
                                {
                                    AdcState.ConvertCount = 200;
                                    TempTotal[Channel] = ((TempTotal[Channel] - ChannelADNow[Channel]) / 40);

                                    if(TempTotal[Channel] < 100)//短路
                                    {
                                        TempArray[Channel][0]++;//短路计数
                                        TempArray[Channel][1] = 0; //开路计数
                                        TempArray[Channel][2] = 0; //正常计数
                                        if(TempArray[Channel][0] >= CHECK_TIMES)
                                        {
                                            TempArray[Channel][0] = CHECK_TIMES;
                                            EventProcess(Channel, CHANNEL_SHORT);
                                        }
                                    }
                                    else if(TempTotal[Channel] > 2000)//开路
                                    {
                                        TempArray[Channel][0] = 0; //短路计数
                                        TempArray[Channel][1]++;//开路计数
                                        TempArray[Channel][2] = 0; //正常计数
                                        if(TempArray[Channel][1] >= CHECK_TIMES)
                                        {
                                            TempArray[Channel][1] = CHECK_TIMES;
                                            EventProcess(Channel, CHANNEL_OPEN);
                                        }
                                    }
                                    else //正常
                                    {
                                        TempArray[Channel][0] = 0; //短路计数
                                        TempArray[Channel][1] = 0; //开路计数
                                        TempArray[Channel][2]++; //正常计数
                                        if(TempArray[Channel][2] >= CHECK_TIMES)
                                        {
                                            TempArray[Channel][2] = CHECK_TIMES;
                                            EventProcess(Channel, CHANNEL_NORMAL);
                                        }
                                    }

                                }
                            }
                            else
                            {
                                TempTotal[Channel] = 0;
                            }
                        }
                    }


                    break;
                default:
                    break;
            }
            ADC_TimerStart();
        }



    }

}

static void AdcFunChange(void)
{
    GPIO_Init(GPIOC, TEMP_CTRL_IO | CURRENT_CTRL_IO | CHECK_CURRENT_IO, GPIO_Mode_Out_PP_High_Fast); //上拉输出
    switch(AdcState.Step)
    {
        case ADC_STEP_CURRENT:
            GPIO_SetBits(GPIOC, TEMP_CTRL_IO);
            GPIO_SetBits(GPIOC, CURRENT_CTRL_IO);
            GPIO_SetBits(GPIOC, CHECK_CURRENT_IO);
            break;
        case ADC_STEP_CHECK_TEMPERATURE:
        case ADC_STEP_TEMPERATURE:
            GPIO_ResetBits(GPIOC, TEMP_CTRL_IO);
            GPIO_ResetBits(GPIOC, CURRENT_CTRL_IO);
            GPIO_SetBits(GPIOC, CHECK_CURRENT_IO);
            break;
        case ADC_STEP_CHECK_CURRENT:
            GPIO_SetBits(GPIOC, TEMP_CTRL_IO);
            GPIO_SetBits(GPIOC, CURRENT_CTRL_IO);
            GPIO_ResetBits(GPIOC, CHECK_CURRENT_IO);
            break;
    }
    AdcState.InChange = ADC_CHANGE_DOING;
}
static void EventProcess(uint8_t Channel, uint8_t Event)
{
    uint16_t ChannelType;
    ChannelType = (ParameterBuffer.ParameterConfig.ChannelAlarmValue[Channel]&0xF000);
    switch(Event)
    {
        case CHANNEL_NORMAL:
            if((ParameterSysStatus.ChannelState[Channel] & CHANNEL_OPEN) == CHANNEL_OPEN)
            {
                ParameterSysStatus.ChannelState[Channel] &= ~(CHANNEL_OPEN);
                if(ChannelType == CHANNEL_TYPE_LEAKAGE)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 139;
                }
                else if(ChannelType == CHANNEL_TYPE_TEMP)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 135;
                }
                else if(ChannelType == CHANNEL_TYPE_CURRENT)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 139;
                }
                ParameterSysStatus.FaultCount --;
            }
            else if((ParameterSysStatus.ChannelState[Channel] & CHANNEL_SHORT) == CHANNEL_SHORT)
            {
                ParameterSysStatus.ChannelState[Channel] &= ~(CHANNEL_SHORT);
                if(ChannelType == CHANNEL_TYPE_LEAKAGE)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 137;
                }
                else if(ChannelType == CHANNEL_TYPE_TEMP)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 133;
                }
                else if(ChannelType == CHANNEL_TYPE_CURRENT)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 137;
                }
                ParameterSysStatus.FaultCount --;
            }
            if(ParameterSysStatus.FaultCount == 0)
            {
                LedCtrl(LED_FAULT, LED_OFF);
                if(MUSIC_State() == MUSIC_FAULT)
                {
                    MUSIC_Set(MUSIC_NONE);
                }
            }
            break;
        case CHANNEL_ALARM:
            if((ParameterSysStatus.ChannelState[Channel] & CHANNEL_ALARM) != CHANNEL_ALARM)
            {
                ParameterSysStatus.ChannelState[Channel] = CHANNEL_ALARM;
                if(ChannelType == CHANNEL_TYPE_LEAKAGE)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 143;
                }
                else if(ChannelType == CHANNEL_TYPE_TEMP)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 144;
                }
                else if(ChannelType == CHANNEL_TYPE_CURRENT)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 145;
                }
                ParameterSysStatus.AlarmCount++;
                LedCtrl(LED_ALARM, LED_ON);
                MUSIC_Set(MUSIC_FIRE);
            }
            else
            {}

            break;
        case CHANNEL_PREALARM:
            break;
        case CHANNEL_OPEN:
            if((ParameterSysStatus.ChannelState[Channel] & CHANNEL_OPEN) != CHANNEL_OPEN)
            {
                ParameterSysStatus.FaultCount++;
                if((ParameterSysStatus.ChannelState[Channel] & CHANNEL_SHORT) == CHANNEL_SHORT)
                {
                    if(ChannelType == CHANNEL_TYPE_LEAKAGE)
                    {
                        ParameterSysStatus.ChannelEvent[Channel] = (137 << 8);
                    }
                    else if(ChannelType == CHANNEL_TYPE_TEMP)
                    {
                        ParameterSysStatus.ChannelEvent[Channel] = (133 << 8);
                    }
                    else if(ChannelType == CHANNEL_TYPE_CURRENT)
                    {
                        ParameterSysStatus.ChannelEvent[Channel] = (137 << 8);
                    }
                    ParameterSysStatus.FaultCount--;
                }
                if(ChannelType == CHANNEL_TYPE_LEAKAGE)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 138;
                }
                else if(ChannelType == CHANNEL_TYPE_TEMP)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 134;
                }
                else if(ChannelType == CHANNEL_TYPE_CURRENT)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 138;
                }
                ParameterSysStatus.ChannelState[Channel] = CHANNEL_OPEN;
                LedCtrl(LED_FAULT, LED_ON);
                MUSIC_Set(MUSIC_FAULT);
            }

            break;
        case CHANNEL_SHORT:
            if((ParameterSysStatus.ChannelState[Channel] & CHANNEL_SHORT) != CHANNEL_SHORT)
            {
                ParameterSysStatus.FaultCount++;
                if((ParameterSysStatus.ChannelState[Channel] & CHANNEL_OPEN) == CHANNEL_OPEN)
                {
                    if(ChannelType == CHANNEL_TYPE_LEAKAGE)
                    {
                        ParameterSysStatus.ChannelEvent[Channel] = (139 << 8);
                    }
                    else if(ChannelType == CHANNEL_TYPE_TEMP)
                    {
                        ParameterSysStatus.ChannelEvent[Channel] = (135 << 8);
                    }
                    else if(ChannelType == CHANNEL_TYPE_CURRENT)
                    {
                        ParameterSysStatus.ChannelEvent[Channel] = (139 << 8);
                    }
                    ParameterSysStatus.FaultCount--;
                }
                if(ChannelType == CHANNEL_TYPE_LEAKAGE)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 136;
                }
                else if(ChannelType == CHANNEL_TYPE_TEMP)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 132;
                }
                else if(ChannelType == CHANNEL_TYPE_CURRENT)
                {
                    ParameterSysStatus.ChannelEvent[Channel] = 136;
                }
                ParameterSysStatus.ChannelState[Channel] = CHANNEL_SHORT;
                LedCtrl(LED_FAULT, LED_ON);
                MUSIC_Set(MUSIC_FAULT);
            }
            break;
        default:
            break;

    }

}

INTERRUPT_HANDLER(TIM3_UPD_OVF_TRG_BRK_USART3_TX_IRQHandler, 21)
{
    TIM3_ClearFlag(TIM3_FLAG_Update);
    AdcState.ConvertCount++;
    ADC_TimerStop();
    ADC_SoftwareStartConv(ADC1);
}
INTERRUPT_HANDLER(DMA1_CHANNEL0_1_IRQHandler, 2)
{
    if(DMA_GetITStatus(DMA1_IT_TC0) != RESET)
    {
        atomSemPut(&Sem_Adc);
        DMA_ClearITPendingBit(DMA1_IT_TC0);
    }
}

static void ADC_TimerStart(void)
{
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE);//
    TIM3_DeInit();

    if(AdcState.ConvertCount >= 200)//40ms
    {
        AdcState.ConvertCount = 0;
        AdcState.Step ++;
        if(AdcState.Step >= 5)
        {
            AdcState.Step = 1;
        }


        AdcState.InChange = ADC_CHANGE_START;
    }

    if(AdcState.InChange == ADC_CHANGE_START)
    {
        TIM3_TimeBaseInit(TIM3_Prescaler_1, TIM3_CounterMode_Up, 16 * 66);//实测66为100us
    }
    else if(AdcState.InChange == ADC_CHANGE_DOING)
    {
        TIM3_TimeBaseInit(TIM3_Prescaler_128, TIM3_CounterMode_Up, 35000);//280Ms
    }
    else if(AdcState.InChange == ADC_CHANGE_DONE)
    {
        TIM3_TimeBaseInit(TIM3_Prescaler_1, TIM3_CounterMode_Up, 16 * 115);//实测115为150us
    }

    sim();

    TIM3_ClearFlag(TIM3_FLAG_Update);
    TIM3_ITConfig(TIM3_IT_Update, ENABLE);
    TIM3_ARRPreloadConfig(ENABLE);
    TIM3_SetCounter(0);

    TIM3_GenerateEvent(TIM3_EventSource_Update);
    TIM3_Cmd(ENABLE);//计数器使能，开始计数

    //很重要，缺少以下内容会导致定时器反复开关过程中定时器定时不准
    while(TIM3_GetITStatus(TIM3_IT_Update) == RESET );// 清除计数器第一次无效中断
    TIM3_ClearFlag(TIM3_FLAG_Update);
    rim();


}
static void ADC_TimerStop(void)
{
    TIM3_SetCounter(0);
    TIM3_ITConfig(TIM3_IT_Update, DISABLE);
    TIM3_Cmd(DISABLE);
    TIM3_DeInit();
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, DISABLE);//

}
