/**************************************************************

  Copyright (C),WangJinLong, 2017 深圳市德派森科技发展有限公司

***************************************************************
 * 文 件 名   : Window.h
 * 版 本 号   : V1.0
 * 作    者   : WangJinLong
 * 生成日期   : 2017年7月12日
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


