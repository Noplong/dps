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
    GPIO_Init(GPIOF, GPIO_Pin_6, GPIO_Mode_Out_PP_Low_Fast);

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
//GPIO_WriteBit(LED_PORT,GPIO_Pin,State);
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
void KeySetSound(uint8_t key)
{
    static uint8_t KeyBuf[6];
    static uint8_t KeyIndex = 0;
    static const uint8_t SetSound[] = {KEY_ENTER, KEY_ENTER, KEY_ENTER, KEY_ENTER, KEY_ENTER, KEY_MENU};
    switch(KeyIndex)
    {
        case 0:
            if(key == KEY_ENTER)
            {
                KeyBuf[KeyIndex++] = key;
            }
            else
            {
                KeyIndex = 0;
            }
            break;
        case 1:
            if(key == KEY_ENTER)
            {
                KeyBuf[KeyIndex++] = key;
            }
            else
            {
                KeyIndex = 0;
            }
            break;
        case 2:
            if(key == KEY_ENTER)
            {
                KeyBuf[KeyIndex++] = key;
            }
            else
            {
                KeyIndex = 0;
            }
            break;
        case 3:
            if(key == KEY_ENTER)
            {
                KeyBuf[KeyIndex++] = key;
            }
            else
            {
                KeyIndex = 0;
            }
            break;
        case 4:
            if(key == KEY_ENTER)
            {
                KeyBuf[KeyIndex++] = key;
            }
            else
            {
                KeyIndex = 0;
            }
            break;
        case 5:
            if(key == KEY_MENU)
            {
                KeyBuf[KeyIndex++] = key;
                if(!memcmp(KeyBuf, SetSound, sizeof(SetSound)))
                {
//                    GPIO_ResetBits(GPIOE, GPIO_Pin_7);
                    GPIO_SetBits(GPIOE, GPIO_Pin_7);
                    if(ParameterBuffer.ParameterConfig.Music == MUSIC_NORMAL)
                    {
                        ParameterBuffer.ParameterConfig.Music = MUSIC_MUTE;
                    }
                    else
                    {
                        ParameterBuffer.ParameterConfig.Music = MUSIC_NORMAL;
                    }
                    WriteFlashParameter();
                    GPIO_ResetBits(GPIOE, GPIO_Pin_7);
                }
            }
            else
            {
                KeyIndex = 0;
            }
            break;
        default:
            KeyIndex = 0;
            break;
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
            KEY = KEY_MENU;
            break;
        case 0x84:
            KEY = KEY_ENTER;
            break;
        case 0x0C:
            KEY = KEY_RETURN;
            break;
        default:
            KEY = 0xFF;
            break;
    }
    return KEY;

}
#define LONG_PRESS 10
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
            gKeyUnpressedCount = 0;
            BackLight(LED_ON);
            KeySetSound(key);//设置音响模式

            switch(key)
            {
                case KEY_MENU:
                case KEY_ENTER:
                case KEY_RETURN:
                    ParameterSysStatus.KeyValue = key;
                    GUI_ShowWindow(CurrentWindow, WM_KEY);
                    break;
                default:
                    break;
            }
        }
        if((key == key_old) && (key == 0xff) && (key_state == KEY_UNPRESS))//无按键操作
        {
            if(gKeyUnpressedCount >= KEY_TIMEOUT)
            {
                if(CurrentWindow != Window_Desktop)
                {
                    ParameterSysStatus.KeyValue = KEY_RETURN;
                    GUI_ShowWindow(CurrentWindow, WM_KEY);
                }
            }

            if(gKeyUnpressedCount >= LCD_TIMEOUT)
            {
                gKeyUnpressedCount = LCD_TIMEOUT;
                BackLight(LED_OFF);
            }

        }

        if((key == key_old) && (key == 0xff) && (key_state == KEY_PRESS))
        {
            key_state = KEY_UNPRESS;
        }

        if((key == key_old) && (key != 0xff) && (key_state == KEY_PRESS))
        {
            count++;
            if(count > 100 && key == KEY_ENTER)//长按确认键复位
            {
                MUSIC_Set(MUSIC_KEY);
                WWDG_SWReset();
                count = 0;
            }
            if(count > 100 && key == KEY_ENTER)//长按返回键复位
            {
                MUSIC_Set(MUSIC_KEY);
                WWDG_SWReset();
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
//    GPIO_ResetBits(GPIOE, GPIO_Pin_7);
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
        GPIO_SetBits(GPIOF, BACK_LIGHT);
    }
    else
    {

        GPIO_ResetBits(GPIOF, BACK_LIGHT);
    }
}

void LedShow (void)
{
    static uint8_t LedFlag = 0xFF;//高四位代表报警灯
    static uint8_t led_cnt = 0;
    uint8_t MusicType;

    MusicType = MUSIC_State();
    led_cnt ++;
    if(ParameterSysStatus.AlarmCount != 0 && (ParameterBuffer.ParameterConfig.ChannelAlarmValue[10] & 0x0FFF) == 0x0001)
    {
        RelayCtrl(LED_ON);
    }
    else if(ParameterSysStatus.FaultCount != 0 && (ParameterBuffer.ParameterConfig.ChannelAlarmValue[10] & 0x0FFF) == 0x0002)
    {
        RelayCtrl(LED_ON);
    }

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
    if(CurrentWindow != Window_SelfCheck)
    {
        LedShow();
    }
    music_type = MUSIC_State();
    if(ParameterBuffer.ParameterConfig.Music == MUSIC_MUTE && CurrentWindow != Window_SelfCheck)
    {
        MUSIC_Off();
        return;
    }
    switch(music_type)
    {
//        case MUSIC_FAULT:
        case MUSIC_FIRE:
        {
            if (music_cnt >= 2)
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
            if (music_cnt >= 9)
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


