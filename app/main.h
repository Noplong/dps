/**************************************************************

  Copyright (C),WangJinLong, 2017 深圳市德派森科技发展有限公司

***************************************************************
 * 文 件 名   : main.h
 * 版 本 号   : V1.0
 * 作    者   : WangJinLong
 * 生成日期   : 2017年7月22日
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
