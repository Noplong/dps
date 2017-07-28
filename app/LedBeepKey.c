/**************************************************************

  Copyright (C),WangJinLong, 2017 深圳市德派森科技发展有限公司

***************************************************************
 * 文 件 名   : LedBeepKey.c
 * 版 本 号   : V1.0
 * 作    者   : WangJinLong
 * 生成日期   : 2017年7月5日
***************************************************************/

#define LEDBEEPKEY_PRIVATE
#include "stm8l15x_conf.h"



static uint8_t MusicState = MUSIC_NONE;
static ATOM_TCB KeyScanTask_Tcb;
NEAR static uint8_t KeyScanTask_thread_stack[KEYSCAN_TASK_SIZE_BYTES];
static void KeyScanTask(uint32_t param);


void LedBeepKeyInit(void)
{
    int8_t status;
    ATOM_TIMER timer_cb;
    //LED
    GPIO_Init(LED_PORT, LED_ALARM | LED_COMM | LED_FAULT, GPIO_Mode_Out_PP_High_Fast);

    //BEEP
    GPIO_Init(GPIOE, GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Fast);

    //BACKLIGHT
    GPIO_Init(GPIOC, GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Fast);

    //KEY
    GPIO_Init(GPIOG, GPIO_Pin_2 | GPIO_Pin_3, GPIO_Mode_In_PU_No_IT);
    GPIO_Init(GPIOF, GPIO_Pin_7, GPIO_Mode_In_PU_No_IT);

    status = atomThreadCreate(&KeyScanTask_Tcb,
                              KEYSCAN_TASK_PRIO, KeyScanTask, 0,
                              &KeyScanTask_thread_stack[KEYSCAN_TASK_SIZE_BYTES - 1],
                              KEYSCAN_TASK_SIZE_BYTES);
    if (status != ATOM_OK)
    {
#ifdef TESTS_LOG_STACK_USAGE
        printf ("KeyScanTask start fail\n");
#endif
    }

//    timer_cb.cb_func = MusicTimerCallback;
//    timer_cb.cb_data = NULL;
//    timer_cb.cb_ticks = (SYSTEM_TICKS_PER_SEC * 2);
//    /* Request the timer callback to run in one second */
//    if (atomTimerRegister (&timer_cb) != ATOM_OK)
//    {
//#ifdef TESTS_LOG_STACK_USAGE
//        printf ("TimerRegister\n");
//#endif
//    }
}

void LedCtrl(uint8_t GPIO_Pin, uint8_t State)
{
//    GPIO_WriteBit(LED_PORT,GPIO_Pin,State);
//GPIO_WriteBit(LED_PORT,LED_ALARM | LED_COMM | LED_FAULT,State);
    if(State == LED_ON)
    {
        GPIO_ResetBits(LED_PORT, GPIO_Pin);
    }
    else
    {
        GPIO_SetBits(LED_PORT, GPIO_Pin);

    }


}

uint8_t KEY_GetKey(void)
{
    uint8_t key_value = 0;
    uint8_t KEY;

    key_value = ((GPIO_ReadInputData(GPIOG) & 0x0C)
                 | (GPIO_ReadInputData(GPIOF) & 0x80));

    switch(key_value)
    {
        case 0x88:
            KEY = KEY_1;
            break;
        case 0x84:
            KEY = KEY_2;
            break;
        case 0x0C:
            KEY = KEY_3;
            break;
        default:
            KEY = 0xFF;
            break;
    }
    return KEY;

}

static void KeyScanTask(uint32_t param)
{
    uint8_t key = 0, key_old = 0, key_state = KEY_UNPRESS;
    uint8_t count = 0;
    while(1)
    {
        key_old = KEY_GetKey();
        atomTimerDelay(10);
        key = KEY_GetKey();

        if((key == key_old) && (key != 0xff) && (key_state == KEY_UNPRESS))
        {
//            printf ("key:%d\n", key);//打印的话容易死机
            if(ParameterBuffer.ParameterConfig.Music == MUSIC_NORMAL)
            {
                MUSIC_Set(MUSIC_KEY);
            }
            key_state = KEY_PRESS;
            switch(key)
            {
                case KEY_1:
                case KEY_2:
                case KEY_3:
                    ParameterSysStatus.KeyValue = key;
                    GUI_ShowWindow(CurrentWindow, WM_KEY);
                    break;
                default:
                    break;
            }
        }

        if((key == key_old) && (key == 0xff) && (key_state == KEY_PRESS))
        {
            key_state = KEY_UNPRESS;
        }

        if((key == key_old) && (key != 0xff) && (key_state == KEY_PRESS))
        {
            count++;
            if(count > 10)
            {
                count = 0;
            }
        }
        else
        {
            count = 0;
        }
//        LedCtrl(LED_ALARM, RESET);
//        atomTimerDelay(SYSTEM_TICKS_PER_SEC * 10);
//        LedCtrl(LED_ALARM, SET);
//        atomTimerDelay(SYSTEM_TICKS_PER_SEC * 10);
    }

}

static void MUSIC_Off(void)
{
    GPIO_ResetBits(GPIOE, GPIO_Pin_7);
}
static void MUSIC_On(void)
{
    GPIO_SetBits(GPIOE, GPIO_Pin_7);
}

uint8_t MUSIC_State(void)
{
    return MusicState;
}

void MUSIC_Set(MusicType_t type)
{
    if(ParameterBuffer.ParameterConfig.Music == MUSIC_MUTE)
    {
        MUSIC_Off();
        return;
    }

    switch(type)
    {
        case MUSIC_KEY:
            MUSIC_On();
            atomTimerDelay(SYSTEM_TICKS_PER_SEC);
            MUSIC_Off();
            MusicState = MUSIC_NONE;//消音
            break;
        case MUSIC_FIRE:
            MusicState = MUSIC_FIRE;
            break;
        case MUSIC_FAULT:
            MusicState = MUSIC_FAULT;
            break;
        case MUSIC_NONE:
            MUSIC_Off();
            MusicState = MUSIC_NONE;
            break;
        default:
            break;
    }
}
void BackLight (uint8_t Ctrl)
{
    if(Ctrl == LED_ON)
    {
        GPIO_SetBits(GPIOC, BACK_LIGHT);
    }
    else
    {

        GPIO_ResetBits(GPIOC, BACK_LIGHT);
    }
}

void LedShow (void)
{
    static uint8_t LedFlag = 0xFF;//高四位代表报警灯
    static uint8_t led_cnt = 0;
    uint8_t MusicType;

    MusicType = MUSIC_State();
    led_cnt ++;

    if(MusicType == MUSIC_FIRE)
    {
        LedFlag |= 0xF0;
    }
    else if(MusicType == MUSIC_FAULT)
    {
        if(ParameterSysStatus.AlarmCount != 0)
        {
            MUSIC_Set(MUSIC_FIRE);
        }
        LedFlag |= 0x0F;
    }
    else
    {
        LedFlag = 0x00;
    }


    if (led_cnt  == 1)
    {
        if(ParameterSysStatus.AlarmCount != 0)
        {
            LedCtrl(LED_ALARM, LED_ON);
        }
        if(ParameterSysStatus.FaultCount != 0)
        {
            LedCtrl(LED_FAULT, LED_ON);
        }
    }
    else if(led_cnt == 4)
    {
        if(ParameterSysStatus.AlarmCount != 0 && ((LedFlag & 0xF0) != 0xF0))
        {
            LedCtrl(LED_ALARM, LED_OFF);
        }
        if(ParameterSysStatus.FaultCount != 0 && ((LedFlag & 0x0F) != 0x0F))
        {
            LedCtrl(LED_FAULT, LED_OFF);
        }
    }
    else if(led_cnt == 7)
    {
        led_cnt = 1;
        if(ParameterSysStatus.AlarmCount != 0)
        {
            LedCtrl(LED_ALARM, LED_ON);
        }
        if(ParameterSysStatus.FaultCount != 0)
        {
            LedCtrl(LED_FAULT, LED_ON);
        }

    }

}
void MusicTimerCallback (void)
{
    uint8_t music_type;
    static uint8_t music_cnt = 0;

    music_cnt++;
    LedShow();
    music_type = MUSIC_State();
    if(ParameterBuffer.ParameterConfig.Music == MUSIC_MUTE)
    {
        MUSIC_Off();
        return;
    }
    switch(music_type)
    {
//        case MUSIC_FAULT:
        case MUSIC_FIRE:
        {
            if (music_cnt == 2)
            {
                music_cnt = 0;
                MUSIC_On();
            }
            else if(music_cnt == 1)
            {
                MUSIC_Off();
            }

        }
        break;
        case MUSIC_FAULT:
        {
            if (music_cnt == 9)
            {
                music_cnt = 0;
                MUSIC_On();
            }
            else if(music_cnt == 2)
            {
                MUSIC_Off();
            }
        }
        break;
        case MUSIC_NONE:
            MUSIC_Off();
            break;
        default:
            break;
    }


}


