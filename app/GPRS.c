/**************************************************************

  Copyright (C),WangJinLong, 2017 深圳市德派森科技发展有限公司

***************************************************************
 * 文 件 名   : GPRS.c
 * 版 本 号   : V1.0
 * 作    者   : WangJinLong
 * 生成日期   : 2017年7月5日
***************************************************************/


#define GPRS_PRIVATE
#include "stm8l15x_conf.h"
#include "atommutex.h"
#include "atomsem.h"
#include "atomqueue.h"
#include "string.h"



#define GON_OFF_PIN    GPIO_Pin_7
#define GON_OFF_GPIO   GPIOG


#define GPON_OFF_PIN    GPIO_Pin_5
#define GPON_OFF_GPIO   GPIOF


static uint16_t RecvCount = 0;
uint8_t AT_Answer = 0x00;//AT命令的回复

static ATOM_SEM Sem_Gprs;

#define QUEUE_USART       10

static uint8_t TrueString[5];
static uint8_t FALSEString[5];


static ATOM_QUEUE Queue_Gprs;
static uint8_t QueueGprsStorage[QUEUE_USART];

static ATOM_TCB GprsUsartTask_Tcb;
NEAR static uint8_t GprsUsartTask_stack[GPRS_USART_TASK_SIZE_BYTES];
static void GprsUsartTask(uint32_t param);
static void SendHeartBeat(void);
static void ConnectFailProcess(void);
static void Gprs_ReStart(void);
static void ReceiveReset(void);

void PrintString(uint8_t *pStr, uint8_t len);
void AnswerServer(uint8_t BufNum);
uint8_t PacketPreprocess(uint8_t *pStr, uint8_t Length, uint8_t PackOrUnpack);


void Gprs_Init(void)
{
    uint8_t status;
    CLK_PeripheralClockConfig(CLK_Peripheral_USART3, ENABLE);
    GPIO_ExternalPullUpConfig(GPIOG, GPIO_Pin_0 | GPIO_Pin_1, ENABLE);
    USART_DeInit(USART3);

    /* USART configuration */
    USART_Init(USART3, 38400,
               USART_WordLength_8b,
               USART_StopBits_1,
               USART_Parity_No,
               (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx));

    GPIO_Init(GON_OFF_GPIO, GON_OFF_PIN, GPIO_Mode_Out_PP_High_Fast);

    GPIO_Init(GPON_OFF_GPIO, GPON_OFF_PIN, GPIO_Mode_Out_PP_High_Fast);

    //DCD管脚
    GPIO_Init(GPRS_DCD_PORT, GPRS_DCD_PIN, GPIO_Mode_In_FL_No_IT);

    GPIO_SetBits(GPON_OFF_GPIO, GPON_OFF_PIN);//关GPRS模块电源

    if (atomSemCreate (&Sem_Gprs, 0) != ATOM_OK)
    {
        printf ("Sem_Gprs creat fail\n");
        return;
    }

    if (atomQueueCreate (&Queue_Gprs, &QueueGprsStorage[0], sizeof(uint8_t), QUEUE_USART) != ATOM_OK)
    {
        return;
    }

//    //打开模块供电
    GPIO_ResetBits(GPON_OFF_GPIO, GPON_OFF_PIN);
//    GPIO_SetBits(GON_OFF_GPIO, GON_OFF_PIN);
//    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);
//    GPIO_ResetBits(GON_OFF_GPIO, GON_OFF_PIN);
//    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);

    status = atomThreadCreate(&GprsUsartTask_Tcb,
                              GPRS_USART_TASK_PRIO, GprsUsartTask, 0,
                              &GprsUsartTask_stack[GPRS_USART_TASK_SIZE_BYTES - 1],
                              GPRS_USART_TASK_SIZE_BYTES);
    if (status != ATOM_OK)
    {
#ifdef TESTS_LOG_STACK_USAGE
        printf ("GPRSTask start fail\n");
#endif
    }
    ReceiveReset();

    USART_Cmd(USART3, ENABLE);

//    ConnectToServer();
    GprsInfo.Status.Status_u8 = 0x00;
//    memset(GprsRxBuf[0].DataBufferChar,sizeof(DATA_FORMAT));
//    memset(GprsRxBuf[1].DataBufferChar,sizeof(DATA_FORMAT));
//    memset(GprsTxBuf.DataBufferChar,sizeof(GprsTxBuf.DataBufferChar));
    GprsInfo.CommandCount = DATAPACKAGE_STARTING;

}
static void ReceiveReset(void)
{
    USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    RecvCount = 0;
    AT_Answer = 1;
    memset(&(GprsRxBuf[0].DataBufferChar[0]), 0, BUF_SIZE);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
}
static uint8_t* SendATCommand(uint8_t *cmd, uint8_t *ack, uint8_t Timeout)
{
    uint8_t* pString = NULL;
    ReceiveReset();
    //atomSemResetCount(&Sem_Gprs,0);
    PrintString(cmd, 0);
    atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * Timeout);
//    USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);

    GprsRxBuf[0].DataBufferChar[RecvCount] = '\0';//添加结束符
    RecvCount = 0;
    pString = strstr(&GprsRxBuf[0].DataBufferChar[0], ack);
    return pString;
}

static void GprsUsartTask(uint32_t param)
{
    uint8_t msg, status;
    uint8_t Commmand;
    uint8_t *pMatchString = NULL;

    while(1)
    {
        status = atomQueueGet(&Queue_Gprs, 3500, &msg);
        if(status == ATOM_OK)
        {
            Commmand = msg;
            switch(Commmand)
            {
                case GPRS_COMMAND_PREPARE://网络连接准备
                    pMatchString = SendATCommand("AT+IPR=38400\r\n", "OK", 4); //波特率同步
                    if(pMatchString == NULL)
                    {
                        pMatchString = SendATCommand("AT+IPR=38400\r\n", "OK", 4); //波特率同步
                    }
                    if(pMatchString == NULL)
                    {
                        Gprs_ReStart();
                        GprsInfo.Status.Status_bit.GPRS_AbleToUse = FALSE;
                    }
                    else
                    {
                        GprsInfo.Status.Status_bit.GPRS_AbleToUse = TRUE;
                        GprsInfo.CommandCount &=  0x0FFF;
                        GprsInfo.CommandCount |= DATAPACKAGE_WAIT_CONNECT;
                        SendATCommand("ATE0\r\n", "OK", 2);
                    }
                    break;
                case GPRS_COMMAND_SINGAL://SIM卡有无及信号强度
                    if(GPIO_ReadInputDataBit(GPRS_DCD_PORT, GPRS_DCD_PIN) == GPRS_DCD_PIN)
                    {
//                        pMatchString = SendATCommand("AT+CIPSTATUS\r\n", "+CPIN: READY", 2);
//                        if(pMatchString)
                        SendATCommand("ATE0\r\n", "OK", 2);
                        pMatchString = SendATCommand("AT+CPIN?\r\n", "ERROR", 2);
                        if(pMatchString == NULL)
                        {
                            GprsInfo.Status.Status_bit.AT_CPIN_Flag = TRUE;
                            pMatchString = SendATCommand("AT+CSQ\r\n", "+CSQ:", 2);
                            status = ((*(pMatchString + 6) - 0x30) * 10) + (*(pMatchString + 7) - 0x30);

                            if(status == 99)//无信号
                            {
                                GprsInfo.AT_CSQ_Data = 0;
                            }
                            else
                            {
                                GprsInfo.AT_CSQ_Data = 113 - (status * 2); //实际dbm
                            }
                        }
                        else
                        {
                            pMatchString = SendATCommand("AT+CPIN?\r\n", "ERROR", 2);
                            if(pMatchString != NULL)
                            {
                                Gprs_ReStart();
                            }
                            GprsInfo.Status.Status_bit.AT_CPIN_Flag = FALSE;
                            GprsInfo.AT_CSQ_Data = 0;
                            GprsInfo.Status.Status_bit.GPRS_AbleToUse = FALSE;
                            GprsInfo.CommandCount &=  0x0FFF;
                            GprsInfo.CommandCount |= DATAPACKAGE_STARTING;

                        }
                    }
                    else
                    {
                        GprsInfo.Status.Status_bit.AT_CPIN_Flag = TRUE;
                        GprsInfo.Status.Status_bit.AT_CGATT_Flag = TRUE;
                        GprsInfo.Status.Status_bit.ServerContectState = TRUE;
                        GprsInfo.CommandCount = DATAPACKAGE_READY_TO_SEND;
                    }
                    break;
                case GPRS_COMMAND_CGATT://GPRS附着状态
//                pMatchString = SendATCommand("AT+CGATT=1\r\n", "+CGATT: 1", 2);
                    pMatchString = SendATCommand("AT+CGATT?\r\n", "+CGATT:", 2);
                    if(pMatchString != NULL)
                    {
                        status = (*(pMatchString + 8) - 0x30);
                        if(status == 0)
                        {
                            pMatchString = SendATCommand("AT+CGATT=1\r\n", "+CGATT: 1", 2);
                            if(pMatchString != NULL)
                            {
                                GprsInfo.Status.Status_bit.AT_CGATT_Flag = TRUE;
                            }
                            else
                            {
                                Gprs_ReStart();

                                GprsInfo.Status.Status_bit.GPRS_AbleToUse = FALSE;
                                GprsInfo.Status.Status_bit.AT_CPIN_Flag = FALSE;
                                GprsInfo.Status.Status_bit.AT_CGATT_Flag = FALSE;
                            }
                        }
                        else
                        {
                            GprsInfo.Status.Status_bit.AT_CGATT_Flag = TRUE;
                        }
                    }
                    else
                    {
                        GprsInfo.Status.Status_bit.AT_CPIN_Flag = FALSE;
                    }
                    break;
                case GPRS_COMMAND_CONNECT_IP://使用IP建立连接
                    pMatchString = SendATCommand("AT+CIPMUX=0\r\n", "OK", 2);
                    pMatchString = SendATCommand("AT+CIPMODE=1\r\n", "OK", 2);
                    pMatchString = SendATCommand("AT+CIPCCFG=3,2,256,1\r\n", "OK", 2);
                    pMatchString = SendATCommand("AT+CSTT=CMNET\r\n", "OK", 5);

                    pMatchString = SendATCommand("AT+CIICR\r\n", "OK", 8); //激活场景
                    pMatchString = SendATCommand("AT+CIFSR\r\n", "ERROR", 10); //获取本地IP
                    if(pMatchString == NULL)
                    {
                        PrintString("AT+CIPSTART=TCP,", 0);
                        pMatchString = SendATCommand(ParameterBuffer.ParameterConfig.ConnectIP, "CONNECT", 20); //连接服务器
                        if(pMatchString != NULL || GPIO_ReadInputDataBit(GPRS_DCD_PORT, GPRS_DCD_PIN) != GPRS_DCD_PIN)
                        {
//                            PrintString("SEND\r\n", 0);//测试

                            RecvCount = 0;
                            GprsInfo.CommandCount = DATAPACKAGE_READY_TO_SEND;
                            GprsInfo.Status.Status_bit.ServerContectState = TRUE;
                        }
                        else
                        {
                            ConnectFailProcess();
                        }
                    }
                    else
                    {
                        ConnectFailProcess();
                    }
                    break;
                case GPRS_COMMAND_HEARTBEAT://发送心跳
                    AT_Answer = 0;
                    SendHeartBeat();

                    break;
                case GPRS_COMMAND_RECVPROCESS://接收包处理
                    if(GprsRxBuf[0].DataBuff.DataPackage_State == DATA_PACKAGE_READY)
                    {
                        status = 0;
                    }
                    else if(GprsRxBuf[1].DataBuff.DataPackage_State == DATA_PACKAGE_READY)
                    {
                        status = 1;
                    }

                    if(GprsRxBuf[status].DataBuff.DataPackage_Command == 0x02)//服务器发送命令
                    {
                        AnswerServer(status);
                    }
                    else if(GprsRxBuf[status].DataBuff.DataPackage_Command == 0x03)//服务器确认命令
                    {
                      memset(&(GprsRxBuf[status].DataBufferChar[0]), 0, BUF_SIZE);
                        GprsRxBuf[status].DataBuff.DataPackage_State = DATA_PACKAGE_EMPTY;
                    }
                    else
                    {
                        memset(&(GprsRxBuf[status].DataBufferChar[0]), 0, BUF_SIZE);
                        GprsRxBuf[status].DataBuff.DataPackage_State = DATA_PACKAGE_EMPTY;
                    }
                    break;
                case GPRS_COMMAND_RECV_TIMEOUT://接收包超时处理
                    memset(&(GprsRxBuf[0].DataBufferChar[0]), 0, BUF_SIZE);
                    memset(&(GprsRxBuf[1].DataBufferChar[0]), 0, BUF_SIZE);
                    GprsRxBuf[0].DataBuff.DataPackage_State = DATA_PACKAGE_EMPTY;
                    GprsRxBuf[1].DataBuff.DataPackage_State = DATA_PACKAGE_EMPTY;
                    RecvCount = 0;
                    break;
                case GPRS_COMMAND_CCID://读设备CCID
                    pMatchString = SendATCommand("AT+CCID\r\n", "OK", 2); //CCID
                    if(pMatchString != NULL)
                    {
                        for(status = 0; status < 20; status++)
                        {
                            GprsInfo.Sim_Iccid[status] = GprsRxBuf[0].DataBufferChar[2 + status];
                        }
                        GprsInfo.Sim_Iccid[19] = '*';
                    }
                    break;
                case GPRS_COMMAND_UPDADTE://更新数据
                    if(GPIO_ReadInputDataBit(GPRS_DCD_PORT, GPRS_DCD_PIN) != GPRS_DCD_PIN)//数据透传模式
                    {
//                        GPIO_Init(GPRS_DTR_PORT, GPRS_DTR_PIN, GPIO_Mode_Out_PP_High_Fast);
//                        GPIO_SetBits(GPRS_DTR_PORT, RELAY_CTRL_IO);
                        pMatchString = SendATCommand("+++", "OK", 15);
                        if(pMatchString != NULL)
                        {
                            pMatchString = SendATCommand("AT+CSQ\r\n", "+CSQ:", 2);
                            if(pMatchString != NULL)
                            {
                                status = ((*(pMatchString + 6) - 0x30) * 10) + (*(pMatchString + 7) - 0x30);

                                if(status == 99)//无信号
                                {
                                    GprsInfo.AT_CSQ_Data = 0;
                                }
                                else
                                {
                                    GprsInfo.AT_CSQ_Data = 113 - (status * 2); //实际dbm
                                }
                            }
                            pMatchString = SendATCommand("ATO\r\n", "CONNECT", 5);
                            AT_Answer = 0;
                        }
                    }
                    break;

            }
        }
        else
        {
            if((GprsInfo.CommandCount & 0xF000) == DATAPACKAGE_STARTING)//正在启动
            {
                if(GprsInfo.Status.Status_bit.GPRS_AbleToUse == FALSE)
                {
                    QueueGprsPut(GPRS_COMMAND_PREPARE);
                }
            }
            else if((GprsInfo.CommandCount & 0xF000) == DATAPACKAGE_WAIT_CONNECT)
            {
                if(GprsInfo.Status.Status_bit.AT_CPIN_Flag == FALSE)
                {
                    QueueGprsPut(GPRS_COMMAND_SINGAL);
                }
                else if(GprsInfo.Status.Status_bit.AT_CGATT_Flag == FALSE)
                {
                    QueueGprsPut(GPRS_COMMAND_CCID);
                    QueueGprsPut(GPRS_COMMAND_CGATT);
                }
                else if(GprsInfo.Status.Status_bit.ServerContectState == FALSE)
                {
                    QueueGprsPut(GPRS_COMMAND_CONNECT_IP);
                }
            }
        }
//        RecvCount = 0;
//        memset(GprsRxBuf, 0, sizeof(GprsRxBuf));

    }
}

void AnswerServer(uint8_t BufNum)
{
    uint8_t i;
    uint8_t TeV, Ch;
    uint16_t Thv;
    GprsTxBuf.DataBuff.DataPackage_Start = 0x7E;
    GprsTxBuf.DataBuff.DataPackage_MsgID_L = GprsRxBuf[BufNum].DataBuff.DataPackage_MsgID_L;
    GprsTxBuf.DataBuff.DataPackage_MsgID_H = GprsRxBuf[BufNum].DataBuff.DataPackage_MsgID_H;
    GprsTxBuf.DataBuff.DataPackage_Version_L = GprsRxBuf[BufNum].DataBuff.DataPackage_Version_L;
    GprsTxBuf.DataBuff.DataPackage_Version_H = GprsRxBuf[BufNum].DataBuff.DataPackage_Version_H;

    memcpy(GprsTxBuf.DataBuff.DataPackage_MsgFrom, GprsInfo.Sim_Iccid, 20);

    GprsTxBuf.DataBuff.DataPackage_MsgTo_Low_L = 0x00;
    GprsTxBuf.DataBuff.DataPackage_MsgTo_Low_H = 0x00;
    GprsTxBuf.DataBuff.DataPackage_MsgTo_Mid_L = 0x00;
    GprsTxBuf.DataBuff.DataPackage_MsgTo_Mid_H = 0x00;
    GprsTxBuf.DataBuff.DataPackage_MsgTo_High_L = 0x00;
    GprsTxBuf.DataBuff.DataPackage_MsgTo_High_H = 0x00;

    GprsTxBuf.DataBuff.DataPackage_Command = 0x03;//命令字节-确认
    GprsTxBuf.DataBuff.DataPackage_Data[0] = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[0];//类型标志

    if(GPIO_ReadInputDataBit(GPRS_DCD_PORT, GPRS_DCD_PIN) == GPRS_DCD_PIN)
    {
        GprsRxBuf[BufNum].DataBuff.DataPackage_State = DATA_PACKAGE_EMPTY;
        memset(&(GprsTxBuf.DataBufferChar[0]), 0, BUF_SIZE);
        ConnectFailProcess();
        return;
    }

    GprsTxBuf.DataBuff.DataPackage_DataLength_L = (2+12);//数据长度
    GprsTxBuf.DataBuff.DataPackage_DataLength_H = 0x00;//数据长度
    
    //信息体数目
    GprsTxBuf.DataBuff.DataPackage_Data[1] = 1;
    GprsTxBuf.DataBuff.DataPackage_Data[2] = SYSTEM_TYPE;//系统类型
    GprsTxBuf.DataBuff.DataPackage_Data[3] = DEVICE_TYPE_11_CHANNEL;//设备类型
    GprsTxBuf.DataBuff.DataPackage_Data[4] = 1;//设备地址-控制器号
    GprsTxBuf.DataBuff.DataPackage_Data[5] = 1;//设备地址-回路号
    GprsTxBuf.DataBuff.DataPackage_Data[6] = 1;//设备地址-地址号
    GprsTxBuf.DataBuff.DataPackage_Data[7] = 0; //设备地址-通道号
    
    GprsTxBuf.DataBuff.DataPackage_Data[8] = 0x00; //心跳间隔
    
    GprsTxBuf.DataBuff.DataPackage_Data[9] = 0x00;//音响模式
    GprsTxBuf.DataBuff.DataPackage_Data[10] = 0x00;
    GprsTxBuf.DataBuff.DataPackage_Data[11] = 0x00;
    GprsTxBuf.DataBuff.DataPackage_Data[12] = 0x00;
    GprsTxBuf.DataBuff.DataPackage_Data[13] = 0x00;

    switch(GprsRxBuf[BufNum].DataBuff.DataPackage_Data[0])
    {
        case SERVER_COMMAND_DEVICE_CONFIG:
            //设备类型正确
            if(GprsRxBuf[BufNum].DataBuff.DataPackage_Data[3] == DEVICE_TYPE_11_CHANNEL)
            {
                //需配置通道数
                for(i = 0; i < GprsRxBuf[BufNum].DataBuff.DataPackage_Data[1]; i++)
                {
                    Ch = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 5];
                    if(Ch == 0)//系统配置
                    {
                        TeV = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 6];
                        if(TeV >= 30 && TeV <= 240)
                        {
                            ParameterBuffer.ParameterConfig.HeartBeatTime = TeV;
                        }
                        TeV = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 7];
                        if(TeV == 0)
                        {
                            ParameterBuffer.ParameterConfig.Music = MUSIC_MUTE;
                        }
                        else
                        {
                            ParameterBuffer.ParameterConfig.Music = MUSIC_NORMAL;
                        }
                    }
                    else if(Ch == 1) //剩余电流1
                    {
                        TeV = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 7]; //参量类型
                        Thv = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 10]; //阈值
                        Thv |= (GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 11] << 8);
                        if((TeV == 15) && ((Thv == 0) || (Thv >= 200 && Thv <= 999)))
                        {
                            ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch - 1] = CHANNEL_TYPE_LEAKAGE;
                            ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch - 1] |= Thv;
                        }
                    }
                    else if(Ch > 1 && Ch < 6)//温度通道2-5
                    {
                        TeV = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 7]; //参量类型
                        Thv = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 10]; //阈值
                        Thv |= (GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 11] << 8);
                        Thv = Thv/10;//缩小10倍
                        if((TeV == 2) && ((Thv == 0) || (Thv >= 45 && Thv <= 140)))
                        {
                            ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch - 1] = CHANNEL_TYPE_TEMP;
                            ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch - 1] |= Thv;
                        }

                    }
                    else if(Ch >= 6 && Ch < 9)//电流通道6-8
                    {
                        TeV = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 7]; //参量类型
                        Thv = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 10]; //阈值
                        Thv |= (GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 11] << 8);
                        Thv = Thv/10;//缩小10倍
                        if((TeV == 11) && ((Thv == 0) || (Thv >= 10 && Thv <= 100)))
                        {
                            ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch - 1] = CHANNEL_TYPE_CURRENT;
                            ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch - 1] |= Thv;
                        }
                    }
                    else if(Ch >= 9 && Ch < 11)//输入通道9 10
                    {
                        TeV = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 7]; //参量类型
                        Thv = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 10]; //阈值
                        Thv |= (GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 11] << 8);
                        Thv = Thv/10;//缩小10倍
                        if((TeV == 128) && ((Thv == 0) || (Thv == 1) || (Thv == 2)))
                        {
                            ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch - 1] = CHANNEL_TYPE_IN;
                            ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch - 1] |= Thv;
                        }
                    }
                    else if(Ch == 11)//输出通道11
                    {
                        TeV = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 7]; //参量类型
                        Thv = GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 10]; //阈值
                        Thv |= (GprsRxBuf[BufNum].DataBuff.DataPackage_Data[2 + (12 * i) + 11] << 8);
                        Thv = Thv/10;//缩小10倍
                        if((TeV == 129) && ((Thv == 0) || (Thv == 1) || (Thv == 2)))
                        {
                            ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch - 1] = CHANNEL_TYPE_OUT;
                            ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch - 1] |= Thv;
                        }
                    }


                }
                WriteFlashParameter();
                i = PacketPreprocess(&GprsTxBuf.DataBufferChar[1], 47, 0);//[0]为起始符
                PrintString(GprsTxBuf.DataBufferChar, i + 1);
                USART_SendData8(USART3, 0x7E);
       
            }
            break;
        case SERVER_COMMAND_DEVICE_RESET:
            i = PacketPreprocess(&GprsTxBuf.DataBufferChar[1], 47, 0);//[0]为起始符
            PrintString(GprsTxBuf.DataBufferChar, i + 1);
            USART_SendData8(USART3, 0x7E);
            atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);
            WWDG_SWReset();
            break;
        case SERVER_COMMAND_DEVICE_MUTE:
            MUSIC_Set(MUSIC_NONE);
            i = PacketPreprocess(&GprsTxBuf.DataBufferChar[1], 47, 0);//[0]为起始符
            PrintString(GprsTxBuf.DataBufferChar, i + 1);
            USART_SendData8(USART3, 0x7E);
            break;
        default:
            break;
    }
    GprsRxBuf[BufNum].DataBuff.DataPackage_State = DATA_PACKAGE_EMPTY;
    memset(&(GprsTxBuf.DataBufferChar[0]), 0, BUF_SIZE);
}
void HeartBeat_Timer(void)
{
    GprsInfo.HeartBeat ++;
//    if(GprsInfo.HeartBeat >= 30)//26时是10秒
    if(GprsInfo.HeartBeat >= (26 * ParameterBuffer.ParameterConfig.HeartBeatTime / 10)) //26时是10秒
    {
        GprsInfo.HeartBeat = 0;
        QueueGprsPut(GPRS_COMMAND_HEARTBEAT);
    }
//    else if(GprsInfo.HeartBeat == 26)
//    {
//        QueueGprsPut(GPRS_COMMAND_UPDADTE);
//    }
}
static void Gprs_ReStart(void)
{
//    MUSIC_Off();
    GPIO_ResetBits(GPIOE, GPIO_Pin_7);

    GPIO_SetBits(GPON_OFF_GPIO, GPON_OFF_PIN);
    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);
    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);
    GPIO_ResetBits(GPON_OFF_GPIO, GPON_OFF_PIN);
    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);
    GPIO_SetBits(GON_OFF_GPIO, GON_OFF_PIN);
    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);
    GPIO_ResetBits(GON_OFF_GPIO, GON_OFF_PIN);
    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);
}
static void ConnectFailProcess(void)
{
    uint8_t *pMatchString = NULL;
    GprsInfo.Status.Status_bit.AT_CPIN_Flag = FALSE;
    GprsInfo.Status.Status_bit.AT_CSTT_Flag = FALSE;
    GprsInfo.Status.Status_bit.ServerContectState = FALSE;
    if(GprsInfo.CommandCount != DATAPACKAGE_STARTING)
    {
        pMatchString = SendATCommand("AT+CIPSTATUS\r\n", "PDP DEACT", 3);
        if(pMatchString != NULL)
        {
            GPIO_SetBits(GPON_OFF_GPIO, GPON_OFF_PIN);
            atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);
            GPIO_ResetBits(GPON_OFF_GPIO, GPON_OFF_PIN);
            GPIO_SetBits(GON_OFF_GPIO, GON_OFF_PIN);
            atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);
            GPIO_ResetBits(GON_OFF_GPIO, GON_OFF_PIN);
            atomTimerDelay(SYSTEM_TICKS_PER_SEC * 30);
            GprsInfo.CommandCount = DATAPACKAGE_STARTING;

        }
        SendATCommand("AT+CIPSHUT\r\n", "OK", 1);
        GprsInfo.CommandCount = DATAPACKAGE_WAIT_CONNECT;
    }
}

static void SendHeartBeat(void)
{
    uint8_t Ch, j, UsedChannel = 0;
    uint16_t ChannelType;
    uint16_t AlarmValue;
    //判断是否还处在透传模式，不是则DATAPACKAGE_WAIT_CONNECT
    if(((GprsInfo.CommandCount & 0xF000) == DATAPACKAGE_WAIT_CONNECT)
       || ((GprsInfo.CommandCount & 0xF000) == DATAPACKAGE_STARTING))
    {
        return;//服务器未连接，不上传心跳包
    }

    if(GPIO_ReadInputDataBit(GPRS_DCD_PORT, GPRS_DCD_PIN) == GPRS_DCD_PIN)
    {
        ConnectFailProcess();
    }
    ParameterBuffer.ParameterConfig.HeartBeatTime = 10;

    memset(&(GprsTxBuf.DataBufferChar[0]), 0, BUF_SIZE);
    GprsTxBuf.DataBuff.DataPackage_Start = 0x7E;
    GprsTxBuf.DataBuff.DataPackage_MsgID_L = GprsInfo.CommandCount;
    GprsTxBuf.DataBuff.DataPackage_MsgID_H = (GprsInfo.CommandCount >> 8);
    GprsInfo.CommandCount++;
    GprsTxBuf.DataBuff.DataPackage_Version_L = 0x02;
    GprsTxBuf.DataBuff.DataPackage_Version_H = CURRENT_VERSION;

    memcpy(GprsTxBuf.DataBuff.DataPackage_MsgFrom, GprsInfo.Sim_Iccid, 20);

    GprsTxBuf.DataBuff.DataPackage_MsgTo_Low_L = 0x00;
    GprsTxBuf.DataBuff.DataPackage_MsgTo_Low_H = 0x00;
    GprsTxBuf.DataBuff.DataPackage_MsgTo_Mid_L = 0x00;
    GprsTxBuf.DataBuff.DataPackage_MsgTo_Mid_H = 0x00;
    GprsTxBuf.DataBuff.DataPackage_MsgTo_High_L = 0x00;
    GprsTxBuf.DataBuff.DataPackage_MsgTo_High_H = 0x00;

    GprsTxBuf.DataBuff.DataPackage_Command = 0x02;//命令字节-发送
    GprsTxBuf.DataBuff.DataPackage_Data[0] = 65;//类型标志-上传设备状态
    j = 2;
    //通道号为0-上传系统配置信息
    GprsTxBuf.DataBuff.DataPackage_Data[j++] = SYSTEM_TYPE;//系统类型
    GprsTxBuf.DataBuff.DataPackage_Data[j++] = DEVICE_TYPE_11_CHANNEL;//设备类型
    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 1;//设备地址-控制器号
    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 1;//设备地址-回路号
    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 1;//设备地址-地址号
    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 0; //设备地址-通道号

    GprsTxBuf.DataBuff.DataPackage_Data[j++] = ParameterBuffer.ParameterConfig.HeartBeatTime; //心跳间隔

    GprsTxBuf.DataBuff.DataPackage_Data[j++] = ParameterBuffer.ParameterConfig.Music;//音响模式
    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 0x00;
    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 0x00;
    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 0x00;
    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 0x00;

    for(Ch = 0; Ch < CHANNEL_NUM; Ch++)//CHANNEL_NUM更改
    {
        ChannelType = (ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch] & 0xF000);
        AlarmValue  = (ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch] & 0x0FFF);
//        if(AlarmValue != 0)//通道在线---->所以通道都有上传
        {
            UsedChannel ++;
            if((ParameterSysStatus.ChannelEvent[Ch] & 0xFF00) != 0)//开路转短路或短路转开路-需先报恢复
            {
                UsedChannel++;
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = SYSTEM_TYPE;//系统类型
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = DEVICE_TYPE_11_CHANNEL;//设备类型
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 1;//设备地址-控制器号
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 1;//设备地址-回路号
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 1;//设备地址-地址号
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = Ch + 1; //设备地址-通道号
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = (ParameterSysStatus.ChannelEvent[Ch] >> 8); //设备状态

                if(ChannelType == CHANNEL_TYPE_LEAKAGE)
                {
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 15;//设备参量-参量类型
                    //实时数据
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = ParameterSysStatus.ChannelValue[Ch];
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = (ParameterSysStatus.ChannelValue[Ch] >> 8);

                    //报警阈值
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = AlarmValue;
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = (AlarmValue >> 8);
                }
                else if(ChannelType == CHANNEL_TYPE_TEMP)
                {
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 2;//设备参量-参量类型
                    //实时数据
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = ParameterSysStatus.ChannelValue[Ch];
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = (ParameterSysStatus.ChannelValue[Ch] >> 8);
                    //报警阈值
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = (AlarmValue * 10);
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = ((AlarmValue * 10) >> 8);

                }
                else if(ChannelType == CHANNEL_TYPE_CURRENT)
                {
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = 11;//设备参量-参量类型
                    //实时数据
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = ParameterSysStatus.ChannelValue[Ch];
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = (ParameterSysStatus.ChannelValue[Ch] >> 8);
                    //报警阈值
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = (AlarmValue * 10);
                    GprsTxBuf.DataBuff.DataPackage_Data[j++] = ((AlarmValue * 10) >> 8);

                }


            }
            GprsTxBuf.DataBuff.DataPackage_Data[j++] = SYSTEM_TYPE;//系统类型
            GprsTxBuf.DataBuff.DataPackage_Data[j++] = DEVICE_TYPE_11_CHANNEL;//设备类型
            GprsTxBuf.DataBuff.DataPackage_Data[j++] = 1;//设备地址-控制器号
            GprsTxBuf.DataBuff.DataPackage_Data[j++] = 1;//设备地址-回路号
            GprsTxBuf.DataBuff.DataPackage_Data[j++] = 1;//设备地址-地址号
            GprsTxBuf.DataBuff.DataPackage_Data[j++] = Ch + 1; //设备地址-通道号
            GprsTxBuf.DataBuff.DataPackage_Data[j++] = ParameterSysStatus.ChannelEvent[Ch]; //设备状态


            if(ChannelType == CHANNEL_TYPE_LEAKAGE)
            {
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 15;//设备参量-参量类型
                //实时数据
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = ParameterSysStatus.ChannelValue[Ch];
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = (ParameterSysStatus.ChannelValue[Ch] >> 8);
                //报警阈值
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = AlarmValue;
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = (AlarmValue >> 8);
            }
            else if(ChannelType == CHANNEL_TYPE_TEMP)
            {
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 2;//设备参量-参量类型
                //实时数据
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = ParameterSysStatus.ChannelValue[Ch];
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = (ParameterSysStatus.ChannelValue[Ch] >> 8);

                //报警阈值
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = (AlarmValue * 10);
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = ((AlarmValue * 10) >> 8);

            }
            else if(ChannelType == CHANNEL_TYPE_CURRENT)
            {
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 11;//设备参量-参量类型
                //实时数据
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = ParameterSysStatus.ChannelValue[Ch];
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = (ParameterSysStatus.ChannelValue[Ch] >> 8);

                //报警阈值
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = (AlarmValue * 10);
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = ((AlarmValue * 10) >> 8);

            }
            else if(ChannelType == CHANNEL_TYPE_IN)
            {
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 128;//设备参量-输入通道
                //实时数据
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 0x00;
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 0x00;

                //报警阈值->检测方式(常开检测、常闭检测)
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = AlarmValue;
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = (AlarmValue >> 8);

            }
            else if(ChannelType == CHANNEL_TYPE_OUT)
            {
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 129;//设备参量-输出通道
                //实时数据
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 0x00;
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = 0x00;

                //报警阈值->输出方式(报警检测、故障输出)
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = AlarmValue;
                GprsTxBuf.DataBuff.DataPackage_Data[j++] = (AlarmValue >> 8);

            }


        }


    }
    GprsTxBuf.DataBuff.DataPackage_Data[1] = (UsedChannel + 1); //信息对象数目

    GprsTxBuf.DataBuff.DataPackage_DataLength_L = ((UsedChannel + 1) * 12 + 2); //应用单元数据长度
    GprsTxBuf.DataBuff.DataPackage_DataLength_H = (((UsedChannel + 1) * 12 + 2) >> 8); //应用单元数据长度

    j = PacketPreprocess(&GprsTxBuf.DataBufferChar[1], ((UsedChannel + 1) * 12 + 2 + 33), 0); //[0]为起始符
    PrintString(GprsTxBuf.DataBufferChar, j + 1);
    USART_SendData8(USART3, 0x7E);
}

void QueueGprsPut(uint8_t command)
{
    atomQueuePut(&Queue_Gprs, 0, &command);
}

void PrintString(uint8_t *pStr, uint8_t len)
{
    if(len == 0)
    {
        while(*pStr)
        {
            USART_ClearFlag(USART3, USART_FLAG_TC);//清除标志位，否则第1位数据会丢失
            if(USART_GetFlagStatus(USART3, USART_FLAG_TXE) != RESET)
            {
                USART_SendData8(USART3, *pStr);
                while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
            }

            pStr++;
        }

    }
    else
    {
        while(len--)
        {
            USART_ClearFlag(USART3, USART_FLAG_TC);//清除标志位，否则第1位数据会丢失
            if(USART_GetFlagStatus(USART3, USART_FLAG_TXE) != RESET)
            {
                USART_SendData8(USART3, *pStr);
                while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
            }

            pStr++;
        }
    }


}

INTERRUPT_HANDLER(TIM3_CC_USART3_RX_IRQHandler, 22)
{
    /* In order to detect unexpected events during development,
       it is recommended to set a breakpoint on the following instruction.
    */
    uint8_t err;
    uint16_t RecvData = 0;
    static uint8_t RxBufChoose = 0;
    static uint8_t SpecialCharCount = 0;
    atomIntEnter();

    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);

        RecvData = USART_ReceiveData8(USART3);
        if(AT_Answer != 1)//透传模式下
        {
            if(RecvData == 0x7E && (RecvCount < 2))//接收到起始符
            {
                RecvCount = 0;
//                LedCtrl(LED_ALARM, LED_ON);
                if(GprsRxBuf[0].DataBuff.DataPackage_State == DATA_PACKAGE_EMPTY)
                {
                    RxBufChoose = 0;
                    GprsRxBuf[RxBufChoose].DataBuff.DataPackage_State = DATA_PACKAGE_RECEVING;
                }
                else if(GprsRxBuf[1].DataBuff.DataPackage_State == DATA_PACKAGE_EMPTY)
                {
                    RxBufChoose = 1;
                    GprsRxBuf[RxBufChoose].DataBuff.DataPackage_State = DATA_PACKAGE_RECEVING;
                }
                GprsRxBuf[RxBufChoose].DataBufferChar[RecvCount ++] = RecvData;
            }
            //正在接收
            else if(GprsRxBuf[RxBufChoose].DataBuff.DataPackage_State == DATA_PACKAGE_RECEVING)
            {
                if((RecvData == 94 || RecvData == 93) && (GprsRxBuf[RxBufChoose].DataBufferChar[RecvCount - 1] == 125))
                {
                    GprsRxBuf[RxBufChoose].DataBufferChar[RecvCount - 1] = (125 + RecvData - 93);
                }
                else
                {
                    GprsRxBuf[RxBufChoose].DataBufferChar[RecvCount ++] = RecvData;
                }

                if(RecvData == 0x7E)//结束符
                {
//                    LedCtrl(LED_ALARM, LED_OFF);
                    GprsRxBuf[RxBufChoose].DataBuff.DataPackage_Stop = 0x7E;
                    GprsRxBuf[RxBufChoose].DataBuff.DataPackage_State = DATA_PACKAGE_READY;
                    RecvCount = 0;
                    QueueGprsPut(GPRS_COMMAND_RECVPROCESS);

                }
            }
            if(RecvCount >= BUF_SIZE)
            {
                QueueGprsPut(GPRS_COMMAND_RECVPROCESS);
            }
        }
        else
        {
            GprsRxBuf[0].DataBufferChar[RecvCount++] = RecvData;
            if(RecvCount >= BUF_SIZE)
            {
                RecvCount = 0;
            }
        }


    }
    atomIntExit(TRUE);
//    if(RecvCount == 1)
//    {
//        OSTmrStart(GPRSTimer, &err);
//    }
}
static unsigned char auchCRCHi[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;

//Low-Order Byte Table
/* Table of CRC values for low¤Corder byte */
static char auchCRCLo[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,
    0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
    0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
    0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,
    0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
    0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,
    0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,
    0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
    0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
    0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,
    0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,
    0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
    0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,
    0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
    0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
    0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,
    0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,
    0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;
unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen)
{
    unsigned char uchCRCHi = 0xFF ;     /* high byte of CRC initialized*/
    unsigned char uchCRCLo = 0xFF ; /* low byte of CRC initialized*/
    unsigned uIndex ;                   /* will index into CRC lookup table*/
    while (usDataLen--)             /* pass through message buffer*/
    {
        uIndex   = uchCRCHi ^ *puchMsg++ ;      /* calculate the CRC*/
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex];
    }
    return (uchCRCHi << 8 | uchCRCLo);
}

/*-----------------------------------------------------------------*/
/* 命令数据包预处理 */
/*-----------------------------------------------------------------*/
uint8_t PacketPreprocess(uint8_t *pStr, uint8_t Length, uint8_t PackOrUnpack)
{
    uint8_t i, j, NewLength = 0;
    uint16_t CRC_Value;
    if(PackOrUnpack)//DATA_UNPACK数据解包
    {
        NewLength = Length;
        if(*pStr == 0x7E)
        {
            NewLength -= 1;
            for(i = 0; i < (Length - 1); i++)
            {
                *(pStr + i) = *(pStr + i + 1);
            }
        }
        for(i = 0; i < Length; i++)
        {
            if(*(pStr + i) == 125 &&  (i < (Length - 1)) &&  (*(pStr + i + 1) == 94 || *(pStr + i + 1) == 93))
            {
                *(pStr + i) = (*(pStr + i + 1) + 32);
                NewLength --;
                for(j = i + 1; j < Length; j++)
                {
                    if((j + 1) == Length)
                    {
                        *(pStr + j) = 0;
                    }
                    else
                    {
                        *(pStr + j) = *(pStr + j + 1);
                    }
                }
            }
        }
    }
    else if(Length >= 2)//数据封包+ 加入CRC校验位
    {
        NewLength = Length + 2;     //加2字节CRC校验
        CRC_Value = CRC16(pStr, Length);
        *(pStr + Length) = (uint8_t)CRC_Value;
        *(pStr + Length + 1) = CRC_Value >> 8;

        for(i = 0; i < NewLength; i++)
        {
            if(*(pStr + i) == 126 || *(pStr + i) == 125)
            {
                NewLength ++;
                for(j = NewLength; j > i; j--)
                {
                    *(pStr + j) = *(pStr + j - 1);
                }
                *(pStr + i)     = 125;
                *(pStr + i + 1) = (*(pStr + i) - 32);
            }
        }

    }
    return NewLength;
}


