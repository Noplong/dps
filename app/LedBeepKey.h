/**************************************************************

  Copyright (C),WangJinLong, 2017 深圳市德派森科技发展有限公司

***************************************************************
 * 文 件 名   : LedBeepKey.h
 * 版 本 号   : V1.0
 * 作    者   : WangJinLong
 * 生成日期   : 2017年7月5日
***************************************************************/
   
#ifndef LEDBEEPKEY_H_
#define LEDBEEPKEY_H_
  
#ifdef LEDBEEPKEY_PRIVATE
    #define LEDBEEPKEY_PUBLIC
#else
    #define LEDBEEPKEY_PUBLIC extern
#endif

#define LED_PORT    GPIOG
#define LED_ALARM   GPIO_Pin_4
#define LED_COMM    GPIO_Pin_5
#define LED_FAULT   GPIO_Pin_6

#define BACK_LIGHT   GPIO_Pin_6

#define LED_ON   0x00
#define LED_OFF   0x01


#define KEY_PRESS       0  //按键未按下
#define KEY_UNPRESS   1     //按键按下
 
#define KEY_MENU 0x31
#define KEY_ENTER 0x32
#define KEY_RETURN 0x33



 //声音功能选择
typedef enum tagMusicType{
    MUSIC_NONE,//无声
    MUSIC_FAULT,//故障
    MUSIC_FIRE,//火警
    MUSIC_NONE4,//
    MUSIC_KEY,//按键声
    MUSIC_KEY_OFF,//按键声
}MusicType_t,*pMusicType_t;

LEDBEEPKEY_PUBLIC void LedBeepKeyInit(void);
LEDBEEPKEY_PUBLIC void MUSIC_Set(MusicType_t type);
LEDBEEPKEY_PUBLIC uint8_t MUSIC_State(void);
LEDBEEPKEY_PUBLIC void LedCtrl(uint8_t GPIO_Pin,uint8_t State);
LEDBEEPKEY_PUBLIC void MusicTimerCallback (void);
LEDBEEPKEY_PUBLIC void BackLight (uint8_t Ctrl);

#endif
