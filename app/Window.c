/**************************************************************

  Copyright (C),WangJinLong, 2017 �����е���ɭ�Ƽ���չ���޹�˾

***************************************************************
 * �� �� ��   : Window.c
 * �� �� ��   : V1.0
 * ��    ��   : WangJinLong
 * ��������   : 2017��7��12��
***************************************************************/

#define WINDOW_PRIVATE
#include "stm8l15x_conf.h"




void WinDesktopProc(uint8_t Msg)
{
    static uint8_t Channel;
    uint8_t Key;
    if(Msg == 0x00)
    {
        return;
    }
    switch(Msg)
    {
        case WM_CREATE:
            break;
        case WM_SHOW:
            LCD_DesktopDisp(Channel, ParameterSysStatus.ChannelValue[Channel]);
            break;
        case WM_HIDE:

            break;
        case WM_KEY:
            Key = ParameterSysStatus.KeyValue;
            ParameterSysStatus.KeyValue = NULL;

            break;
        case WM_TIMER:
            Channel++;
//            if(Channel >= 1)
            if(Channel >= CHANNEL_NUM)
            {
                Channel = 0;
            }
            LCD_DesktopDisp(Channel, ParameterSysStatus.ChannelValue[Channel]);
            break;
        default:
            break;
    }
}
void WinMenuProc(uint8_t Msg)
{
    if(Msg == 0x00)
    {
        return;
    }
    switch(Msg)
    {
        case WM_CREATE:
            break;
        case WM_SHOW:

            break;
        case WM_HIDE:

            break;
        case WM_KEY:

            break;
        case WM_TIMER:
            break;
        default:
            break;
    }


}
void WinPasswordProc(uint8_t Msg)
{
    if(Msg == 0x00)
    {
        return;
    }
    switch(Msg)
    {
        case WM_CREATE:
            break;
        case WM_SHOW:

            break;
        case WM_HIDE:

            break;
        case WM_KEY:

            break;
        case WM_TIMER:
            break;
        default:
            break;
    }


}
void WinChannelSetProc(uint8_t Msg)
{
    uint8_t Key;
    if(Msg == 0x00)
    {
        return;
    }
    switch(Msg)
    {
        case WM_CREATE:
            break;
        case WM_SHOW:


            break;
        case WM_HIDE:

            break;
        case WM_KEY:



            break;
        case WM_TIMER:
            break;
        default:
            break;
    }

}
void WinAddressSetProc(uint8_t Msg)
{

    if(Msg == 0x00)
    {
        return;
    }
    switch(Msg)
    {
        case WM_CREATE:
            break;
        case WM_SHOW:

            break;
        case WM_HIDE:

            break;
        case WM_KEY:

            break;
        case WM_TIMER:
            break;
        default:
            break;
    }

}
void WinSelfCheckProc(uint8_t Msg)
{
    if(Msg == 0x00)
    {
        return;
    }
    switch(Msg)
    {
        case WM_CREATE:
            break;
        case WM_SHOW:

            break;
        case WM_HIDE:

            break;
        case WM_KEY:

            break;
        case WM_TIMER:
            break;
        default:
            break;
    }


}
void WinReStartProc(uint8_t Msg)
{
    if(Msg == 0x00)
    {
        return;
    }
    switch(Msg)
    {
        case WM_CREATE:
            break;
        case WM_SHOW:

            break;
        case WM_HIDE:

            break;
        case WM_KEY:

            break;
        case WM_TIMER:
            break;
        default:
            break;
    }


}

