/**************************************************************

  Copyright (C),WangJinLong, 2017 �����е���ɭ�Ƽ���չ���޹�˾

***************************************************************
 * �� �� ��   : main.h
 * �� �� ��   : V1.0
 * ��    ��   : WangJinLong
 * ��������   : 2017��7��22��
***************************************************************/
   
#ifndef MAIN_H_
#define MAIN_H_
  
#ifdef MAIN_PRIVATE
    #define MAIN_PUBLIC
#else
    #define MAIN_PUBLIC extern
#endif
 
MAIN_PUBLIC void GUI_ShowWindow(WinName_t name, uint8_t Msg);
  
#endif
