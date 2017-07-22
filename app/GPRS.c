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

#define GON_OFF_PIN    GPIO_Pin_7
#define GON_OFF_GPIO   GPIOG


#define GPON_OFF_PIN    GPIO_Pin_5
#define GPON_OFF_GPIO   GPIOF


static uint8_t RecvCount = 0;
uint8_t ReadDataFlag = 0x00;

static ATOM_SEM Sem_Gprs;

#define QUEUE_USART       10

static ATOM_QUEUE Queue_Gprs;
static uint8_t QueueGprsStorage[QUEUE_USART];

static ATOM_TCB GprsUsartTask_Tcb;
NEAR static uint8_t GprsUsartTask_stack[GPRS_USART_TASK_SIZE_BYTES];
static void GprsUsartTask(uint32_t param);
void PrintString(uint8_t *pStr, uint8_t len);


void Gprs_Init(void)
{
    uint8_t status;
    CLK_PeripheralClockConfig(CLK_Peripheral_USART3, ENABLE);
    GPIO_ExternalPullUpConfig(GPIOG, GPIO_Pin_0 | GPIO_Pin_1, ENABLE);
    USART_DeInit(USART3);

    /* USART configuration */
    USART_Init(USART3, 9600,
               USART_WordLength_8b,
               USART_StopBits_1,
               USART_Parity_No,
               (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx));

    GPIO_Init(GON_OFF_GPIO, GON_OFF_PIN, GPIO_Mode_Out_PP_High_Fast);

    GPIO_Init(GPON_OFF_GPIO, GPON_OFF_PIN, GPIO_Mode_Out_PP_High_Fast);
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

    USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
    USART_Cmd(USART3, DISABLE);

//    //打开模块供电,关复位脚
    GPIO_ResetBits(GPON_OFF_GPIO, GPON_OFF_PIN);
    GPIO_SetBits(GON_OFF_GPIO, GON_OFF_PIN);
    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 15);
    GPIO_ResetBits(GON_OFF_GPIO, GON_OFF_PIN);
    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 30);

//    ConnectToServer();

}
static void GprsUsartTask(uint32_t param)
{
    uint8_t msg, status;
    uint8_t Commmand;

    while(1)
    {
        status = atomQueueGet(&Queue_Gprs, 0, &msg);
//        if(status == ATOM_OK)
//        {
//            Commmand = msg;
          
//            switch(Commmand)
//            {
//                case AT_ATE:
//                    PrintString("ATE0\r\n", 0);
//                    //等待1.5s
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("ATE0\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    break;
//                case AT_CPIN:
//                    PrintString("AT+CPIN?\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+CPIN?\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {

//                    }
//                    break;
//                case AT_CSQ:
//                    PrintString("AT+CSQ\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+CSQ\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {

//                    }
//                    break;
//                case AT_CGATT:
//                    PrintString("AT+CGATT=1\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+CGATT=1\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {

//                    }
//                    break;
//                case AT_CIPMUX:
//                    PrintString("AT+CIPMUX=0\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+CIPMUX=0\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {

//                    }
//                    break;
//                case AT_CIPMODE:
//                    PrintString("AT+CIPMODE=0\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+CIPMODE=0\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {

//                    }
//                    break;
//                case AT_CSTT:
//                    PrintString("AT+CSTT=CMNET\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+CSTT=CMNET\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {

//                    }
//                    break;
//                case AT_CIICR:
//                    PrintString("AT+CIICR\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+CIICR\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {

//                    }
//                    break;
//                case AT_CIPSTART:
//                    PrintString("AT+CIPSTART=UDP,", 0);
//                    PrintString("115.28.103.131,8089\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                    }
//                    else
//                    {

//                    }
//                    break;
//                case AT_CIPSEND:
//                    PrintString("AT+CIPSEND\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+CIPSEND\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {
//                        for(status = 0; status <= 23; status++)//查找接收的字符中有没有"\r\n> ",有表示可以发送
//                        {
//                            if(GprsRxBuf[status] == 0x0D
//                               && GprsRxBuf[status + 1] == 0x0A
//                               && GprsRxBuf[status + 2] == 0x3E)
//                            {
//                                USART_SendData8(USART3, (ParameterBuffer.ParameterConfig.PackageHead_From >> 8));
//                                USART_SendData8(USART3, (ParameterBuffer.ParameterConfig.PackageHead_From & 0x00FF));
//                                USART_SendData8(USART3, (ParameterBuffer.ParameterConfig.PackageHead_To >> 8));
//                                USART_SendData8(USART3, (ParameterBuffer.ParameterConfig.PackageHead_To & 0x00FF));

                            
//                                USART_SendData8(USART3, 0x1A);
//                                break;
//                            }
//                        }
//                    }
//                    break;
//                case AT_CIPSRIP:
//                    PrintString("AT+CIPSRIP=1\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+CIPSRIP=1\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {

//                    }
//                    break;
//                case AT_IPR:
//                    PrintString("AT+IPR=9600\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+IPR=9600\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {

//                    }
//                    break;
//                case AT_CCID:
//                    PrintString("AT+CCID\r\n", 0);
//                    status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    if(status == ATOM_TIMEOUT)
//                    {
//                        PrintString("AT+CCID\r\n", 0);
//                        //等待1.5s
//                        status = atomSemGet(&Sem_Gprs, SYSTEM_TICKS_PER_SEC * 15);
//                    }
//                    else
//                    {

//                    }
//                    break;
//            }
//        }
//        RecvCount = 0;
//        memset(GprsRxBuf, 0, sizeof(GprsRxBuf));

    }
}
void QueueGprsPut(uint8_t command)
{
    atomQueuePut(&Queue_Gprs, 0, &command);
}
void ConnectToServer(void)
{
    QueueGprsPut(AT_IPR);//串口波特率同步
    QueueGprsPut(AT_ATE);//判断是否可以与GPRS模块通讯，并关闭回显
    QueueGprsPut(AT_CPIN);//检测是否有手机卡接入
    QueueGprsPut(AT_CSQ);//检查网络信号强度
    QueueGprsPut(AT_CGATT);//设置附着GPRS
    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 20);

    QueueGprsPut(AT_CIPMUX);
    QueueGprsPut(AT_CSTT);
    QueueGprsPut(AT_CIICR);
    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 60);

    QueueGprsPut(AT_CIFSR);
    atomTimerDelay(SYSTEM_TICKS_PER_SEC * 30);
    QueueGprsPut(AT_CCID);

    QueueGprsPut(AT_CIPSTART);
    QueueGprsPut(AT_CIPSRIP);



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

    atomIntEnter();

    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);

        RecvData = USART_ReceiveData8(USART3);

        GprsRxBuf[RecvCount++] = RecvData;
        if(RecvCount >= 10 && GprsRxBuf[RecvCount - 1] == '\n')//接收到服务器数据
        {
//            if(GprsRxBuf[RecvCount - 1] == (ParameterBuffer.ParameterConfig.PackageHead_From & 0x00FF)
//               && GprsRxBuf[RecvCount - 2] == (ParameterBuffer.ParameterConfig.PackageHead_From >> 8)
//               && GprsRxBuf[RecvCount - 3] == (ParameterBuffer.ParameterConfig.PackageHead_To & 0x00FF)
//               && GprsRxBuf[RecvCount - 4] == (ParameterBuffer.ParameterConfig.PackageHead_To >> 8))
            {
                ReadDataFlag = 0xFF; //接收的是服务器端命令
//                if(gSysInfo.Status.Status_bit.UartContectState == UART_STATE_SYSSET)
//                {
//                    GprsRxBuf[6] = (ParameterBuffer.gSysConfig.PackageHead_From & 0x00FF);
//                    GprsRxBuf[5] = (ParameterBuffer.gSysConfig.PackageHead_From >> 8);
//                    GprsRxBuf[4] = (ParameterBuffer.gSysConfig.PackageHead_To & 0x00FF);
//                    GprsRxBuf[3] = (ParameterBuffer.gSysConfig.PackageHead_To >> 8);
//                    GprsRxBuf[2] = 0x06;
//                    GprsRxBuf[1] = 0xBB;
//                    GprsRxBuf[0] = 0xAA;
//                    RecvCount = 7;
//                }
//                else
                {
                    RecvCount = 0;
                    atomSemPut(&Sem_Gprs);
                }
            }
        }

        if(RecvCount >= 250)
        {
            RecvCount = 0;
        }

    }
    atomIntExit(TRUE);
//    if(RecvCount == 1)
//    {
//        OSTmrStart(GPRSTimer, &err);
//    }
}


