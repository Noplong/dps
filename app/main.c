/**************************************************************

  Copyright (C),WangJinLong, 2017 �����е���ɭ�Ƽ���չ���޹�˾

***************************************************************
 * �� �� ��   : main.c
 * �� �� ��   : V1.0
 * ��    ��   : WangJinLong
 * ��������   : 2017��7��5��
***************************************************************/

#define MAIN_PRIVATE
#include "stm8l15x_conf.h"
#include "atommutex.h"
#include "atomqueue.h"
static ATOM_TCB SysInit_Tcb;
static void SysInit_Task (uint32_t param);


NEAR static uint8_t main_thread_stack[SYSINIT_TASK_SIZE_BYTES];
NEAR static uint8_t idle_thread_stack[IDLE_STACK_SIZE_BYTES];


#define ATOM_STACK_CHECKING         //�������пռ����
#define TESTS_LOG_STACK_USAGE       //���м�¼




static ATOM_MUTEX Mutex_Lcd;

#define QUEUE_ENTRIES       20

static ATOM_QUEUE Queue_Lcd;
static uint8_t QueueLcdStorage[QUEUE_ENTRIES];

//static ATOM_TCB GuiTask_Tcb;
//NEAR static uint8_t GuiTask_thread_stack[GUI_TASK_SIZE_BYTES];
//static void GuiTask(uint32_t param);

static uint8_t WinMsg;


void SYSCLK_Init()
{
    //CLK�Ĵ���ȫ���ָ�Ĭ��ֵ
    CLK_DeInit();

    //�����л�ʱ��Դ�������������ʱ��Դ��Զ��HSI�ڲ�����ʱ��Դ��
    CLK_SYSCLKSourceSwitchCmd(ENABLE);

    //ѡ��ʱ��Դ
    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);

    //ѡ���Ƶ��16MHZ/1
    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);

    //ʱ������
    CLK_HSICmd(ENABLE);
}

void BSP_Init(void)
{
    LCD_GLASS_Init();//����Һ����ʼ��
//    CurrentAdInit();//�����ɼ�ͨ����ʼ��
//    MixChannelInit();//���ͨ����ʼ��
//    IOCheck();//���������ʼ��
    ReadFlashParameter();//����ϵͳ����
}
void GUI_ShowWindow(WinName_t name, uint8_t Msg)
{

    atomMutexGet(&Mutex_Lcd, 0);
    CurrentWindow = name;
    WinMsg = Msg;
    atomQueuePut(&Queue_Lcd, 0, (uint8_t*)WinMsg);
    atomMutexPut(&Mutex_Lcd);
}
int main(void)
{
    int8_t status;
    SYSCLK_Init( );//ʱ�ӳ�ʼ��->16MHZ
    sim();

    /* Initialise the OS before creating our threads */
    status = atomOSInit(&idle_thread_stack[IDLE_STACK_SIZE_BYTES - 1], IDLE_STACK_SIZE_BYTES);
    if (status == ATOM_OK)
    {
        /* Enable the system tick timer */
        archInitSystemTickTimer();
        if (atomMutexCreate (&Mutex_Lcd) != ATOM_OK)
        {
            printf ("Mutex_Lcd creat fail\n");
        }

        if (atomQueueCreate (&Queue_Lcd, &QueueLcdStorage[0], sizeof(uint8_t), QUEUE_ENTRIES) != ATOM_OK)
        {
            printf ("Queue_Lcd creat fail\n");
        }
        /* Create an application thread */
        status = atomThreadCreate(&SysInit_Tcb,
                                  SYSINIT_TASK_PRIO, SysInit_Task, 0,
                                  &main_thread_stack[SYSINIT_TASK_SIZE_BYTES - 1],
                                  SYSINIT_TASK_SIZE_BYTES);
        if (status == ATOM_OK)
        {
            /**
             * First application thread successfully created. It is
             * now possible to start the OS. Execution will not return
             * from atomOSStart(), which will restore the context of
             * our application thread and start executing it.
             *
             * Note that interrupts are still disabled at this point.
             * They will be enabled as we restore and execute our first
             * thread in archFirstThreadRestore().
             */
            atomOSStart();
        }
    }
    rim();

}

static void SysInit_Task (uint32_t param)
{
    uint32_t test_status;
    uint8_t msg;
    param = param;

    if (uart_init(115200) != 0)
    {
        /* Error initialising UART */
    }
    printf("SYSTEM LOADING..........\n");

    LCD_GLASS_Init();//����Һ����ʼ��
//    printf("Lcd Initialize..........\n");
    ReadFlashParameter();//����ϵͳ����
    Gprs_Init();//GPRSͨѶ��ʼ��
    LedBeepKeyInit();//LED���������ڳ�ʼ��
    AdcInit();//ADCͨ���ɼ���ʼ��
    GUI_ShowWindow(Window_Desktop, WM_SHOW);
//    printf("LedBeepKey Initialize..........\n");
    BackLight(LED_ON);
    while (1)
    {
        test_status = atomQueueGet(&Queue_Lcd, 500, &msg);
        MusicTimerCallback();
        if(test_status == ATOM_TIMEOUT)
        {
            msg = WM_TIMER;
        }
        else
        {
            msg = WinMsg;
            WinMsg = NULL;
        }
        switch(CurrentWindow)
        {
            case Window_Desktop:
                WinDesktopProc(msg);
                break;
            case Window_Menu:
                WinMenuProc(msg);
                break;
            case Window_Password:
                WinPasswordProc(msg);
                break;
            case Window_ChannelSet:
                WinChannelSetProc(msg);
                break;
            case Window_AddressSet:
                WinAddressSetProc(msg);
                break;
            case Window_SelfCheck:
                WinSelfCheckProc(msg);
                break;
            case Window_ReStart:
                WinReStartProc(msg);
                break;
            default:
                break;
        }
//        GUI_ShowWindow(Window_Desktop, WM_TIMER);
//        MusicTimerCallback();
//        atomTimerDelay(SYSTEM_TICKS_PER_SEC * 2);
//        MusicTimerCallback();
//        atomTimerDelay(SYSTEM_TICKS_PER_SEC * 2);
//        MusicTimerCallback();
//        atomTimerDelay(SYSTEM_TICKS_PER_SEC * 2);
//        MusicTimerCallback();
//        atomTimerDelay(SYSTEM_TICKS_PER_SEC * 2);
//        MusicTimerCallback();
//        atomTimerDelay(SYSTEM_TICKS_PER_SEC * 2);
    }
}