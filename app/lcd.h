/**************************************************************

  Copyright (C),WangJinLong, 2017 �����е���ɭ�Ƽ���չ���޹�˾

***************************************************************
 * �� �� ��   : lcd.h
 * �� �� ��   : V1.0
 * ��    ��   : WangJinLong
 * ��������   : 2017��7��7��
***************************************************************/
   
#ifndef LCD_H_
#define LCD_H_
  
#ifdef LCD_PRIVATE
    #define LCD_PUBLIC
#else
    #define LCD_PUBLIC extern
#endif
 

#include <string.h>
#include <stdio.h>

#define WINDOW_MAX        20

#define WM_CREATE   0x01
#define WM_SHOW     0x02
#define WM_REFRESH     0x03
#define WM_HIDE       0x04
#define WM_KEY        0x05
#define WM_TIMER    0X06
#define WM_USER      0x07

#define SPECIALCHAR_MA      0x01
#define SPECIALCHAR_T       0x02
#define SPECIALCHAR_A       0x03
#define SPECIALCHAR_SINGAL  0x04
typedef enum tagWinName
{
    Window_None,//0
    Window_Desktop,// 1 ����
    Window_Menu,// �˵�
    Window_Password,//�û���������
    Window_ChannelSet,//ͨ������
    Window_AddressSet,//��ַ����
    Window_SelfCheck,//�Լ�
    Window_ReStart//��λ   
} WinName_t, *pWinName_t;

typedef struct tagWinMsg
{
    uint8_t type;
    uint8_t state;
    uint8_t data;
} WinMsg_t, *pWinMsg_t;

typedef void (*WinProc_t)(WinMsg_t *pMsg);

typedef struct tagWindow_t
{
    WinName_t name;
    WinProc_t  pWinProcFunc;
} Window_t, *pWindow_t;

#define MAX_LCD_COM     4
#define MAX_LCD_SEG     8

typedef enum {
  PIXEL_OFF=0,
  PIXEL_ON=1
} LCD_PIXEL_SET_TypeDef;

LCD_PUBLIC WinName_t CurrentWindow;

#define KEY_TIMEOUT     40
#define LCD_TIMEOUT     80

LCD_PUBLIC uint16_t gKeyUnpressedCount;//������ʱ

LCD_PUBLIC void LCD_GLASS_Init(void);
LCD_PUBLIC void LCD_SetPixel(uint8_t Com, uint8_t Seg, LCD_PIXEL_SET_TypeDef On_Off);
LCD_PUBLIC void LCD_DisplayChar(uint8_t ch,uint8_t position,uint8_t signbit);
LCD_PUBLIC void LCD_Printf(char *string);
LCD_PUBLIC void LCD_DesktopDisp(uint8_t Ch, uint16_t Data);
LCD_PUBLIC void LCD_AdressDisp(void);
LCD_PUBLIC void LCD_MenuDisp(uint8_t MenuIndex);
LCD_PUBLIC void LCD_BlinkDisop(void);
LCD_PUBLIC void LCD_ChannelSetDisp(uint8_t Ch);

LCD_PUBLIC void LCD_PrintfSpecialChar(unsigned char Pos, unsigned char PixelOnoff);
#endif
