/**************************************************************

  Copyright (C),WangJinLong, 2017 �����е���ɭ�Ƽ���չ���޹�˾

***************************************************************
 * �� �� ��   : Window.h
 * �� �� ��   : V1.0
 * ��    ��   : WangJinLong
 * ��������   : 2017��7��12��
***************************************************************/

#ifndef WINDOW_H_
#define WINDOW_H_

#ifdef WINDOW_PRIVATE
#define WINDOW_PUBLIC
#else
#define WINDOW_PUBLIC extern
#endif


WINDOW_PUBLIC void WinDesktopProc(uint8_t Msg);
WINDOW_PUBLIC void WinMenuProc(uint8_t Msg);
WINDOW_PUBLIC void WinPasswordProc(uint8_t Msg);
WINDOW_PUBLIC void WinChannelSetProc(uint8_t Msg);
WINDOW_PUBLIC void WinAddressSetProc(uint8_t Msg);
WINDOW_PUBLIC void WinSelfCheckProc(uint8_t Msg);
WINDOW_PUBLIC void WinReStartProc(uint8_t Msg);

#endif


