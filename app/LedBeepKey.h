/**************************************************************

  Copyright (C),WangJinLong, 2017 �����е���ɭ�Ƽ���չ���޹�˾

***************************************************************
 * �� �� ��   : LedBeepKey.h
 * �� �� ��   : V1.0
 * ��    ��   : WangJinLong
 * ��������   : 2017��7��5��
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


#define KEY_PRESS       0  //����δ����
#define KEY_UNPRESS   1     //��������
 
#define KEY_MENU 0x31
#define KEY_ENTER 0x32
#define KEY_RETURN 0x33



 //��������ѡ��
typedef enum tagMusicType{
    MUSIC_NONE,//����
    MUSIC_FAULT,//����
    MUSIC_FIRE,//��
    MUSIC_NONE4,//
    MUSIC_KEY,//������
    MUSIC_KEY_OFF,//������
}MusicType_t,*pMusicType_t;

LEDBEEPKEY_PUBLIC void LedBeepKeyInit(void);
LEDBEEPKEY_PUBLIC void MUSIC_Set(MusicType_t type);
LEDBEEPKEY_PUBLIC uint8_t MUSIC_State(void);
LEDBEEPKEY_PUBLIC void LedCtrl(uint8_t GPIO_Pin,uint8_t State);
LEDBEEPKEY_PUBLIC void MusicTimerCallback (void);
LEDBEEPKEY_PUBLIC void BackLight (uint8_t Ctrl);

#endif
