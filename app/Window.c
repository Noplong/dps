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
    static uint8_t Channel, Timer = 0;
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
            if(Key == KEY_1)
            {
                GUI_ShowWindow(Window_Menu, WM_SHOW);
            }
            break;
        case WM_TIMER:
            Timer++;
            if(Timer >= 4)
            {
                Timer = 0;
                Channel++;
                //            if(Channel >= 1)
                if(Channel >= CHANNEL_NUM)
                {
                    Channel = 0;
                }
                LCD_DesktopDisp(Channel, ParameterSysStatus.ChannelValue[Channel]);
            }
            break;
        default:
            break;
    }
}
void WinMenuProc(uint8_t Msg)
{
    static uint8_t MenuIndex = 0;
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
            LCD_MenuDisp(MenuIndex);
            break;
        case WM_HIDE:

            break;
        case WM_KEY:
            Key = ParameterSysStatus.KeyValue;
            ParameterSysStatus.KeyValue = NULL;
            switch(Key)
            {
                case KEY_1:
                    MenuIndex ++;
                    if(MenuIndex >= 3)
                    {
                        MenuIndex = 0;
                    }
                    LCD_MenuDisp(MenuIndex);
                    break;
                case KEY_2:
                    if(MenuIndex == 0)
                    {
                        GUI_ShowWindow(Window_AddressSet, WM_SHOW);
                    }
                    else if(MenuIndex == 1)
                    {
                        GUI_ShowWindow(Window_ChannelSet, WM_SHOW);
                    }
                    else if(MenuIndex == 2)
                    {
                        GUI_ShowWindow(Window_SelfCheck, WM_SHOW);
                    }
                    break;
                case KEY_3:
                    MenuIndex = 0;
                    GUI_ShowWindow(Window_Desktop, WM_SHOW);
                    break;

            }
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
    static uint8_t Channel = 0;
    uint8_t Key, BitValue;
    if(Msg == 0x00)
    {
        return;
    }
    switch(Msg)
    {
        case WM_CREATE:
            break;
        case WM_SHOW:
            BlinkFlag = 0;
            OptionBit = 0;
            IntoEdit = 0;
            LCD_ChannelSetDisp(Channel);

            break;
        case WM_HIDE:

            break;
        case WM_KEY:
            Key = ParameterSysStatus.KeyValue;
            ParameterSysStatus.KeyValue = NULL;
            switch(Key)
            {
                case KEY_1:
                    if(IntoEdit == 0)//δ����༭״̬-�л�ͨ��
                    {
                        Channel ++;
                        if(Channel >= CHANNEL_NUM)
                        {
                            Channel = 0;
                        }
                        LCD_ChannelSetDisp(Channel);
                    }
                    else//����༭״̬-��ǰOptionBit��ֵ�ۼ�
                    {
                        BitValue = (BitRam[OptionBit] - 0x30);
                        BitValue ++;
                        if(BitValue >= 10)
                        {
                            BitValue = 0;
                        }
                        BitRam[OptionBit] = (BitValue + 0x30);
                    }
                    break;
                case KEY_2:
                    if(IntoEdit == 0)//δ����༭״̬-����༭״̬
                    {
                        BlinkFlag = 1;
                        OptionBit = 2;//��0λͨ���ţ���һλΪ��.�������ɱ༭
                        IntoEdit = 1;
                    }
                    else//����༭״̬-��ǰOptionBit��ֵ�ۼ�
                    {
                        BlinkFlag = 1;
                        OptionBit ++;
                        if(OptionBit >= 5)
                        {
                            OptionBit = 2;
                            //�������
                        }
                    }
                    break;
                case KEY_3:
                    if(IntoEdit == 0)//δ����༭״̬-���ز˵�
                    {
                        GUI_ShowWindow(Window_Menu, WM_SHOW);
                    }
                    else//����༭״̬-�˳��༭
                    {
                        IntoEdit = 0;
                        LCD_ChannelSetDisp(Channel);//������ֵ���༭����������
                    }

                    break;

            }


            break;
        case WM_TIMER:
            break;
        default:
            break;
    }

}
void WinAddressSetProc(uint8_t Msg)
{
    static uint8_t BitValue = 0;
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
            BlinkFlag = 1;
            OptionBit = 0;
            LCD_AdressDisp();
            break;
        case WM_HIDE:

            break;
        case WM_KEY:
            Key = ParameterSysStatus.KeyValue;
            ParameterSysStatus.KeyValue = NULL;
            switch(Key)
            {
                case KEY_1://����
                    BitValue = (BitRam[OptionBit] - 0x30);
                    BitValue ++;
                    if(BitValue >= 10)
                    {
                        BitValue = 0;
                    }
                    BitRam[OptionBit] = (BitValue + 0x30);
                    break;
                case KEY_2://ȷ��
                    OptionBit ++;
                    if(OptionBit > 3)
                    {
                        OptionBit = 0;

                        //�������
                        ParameterBuffer.ParameterConfig.PackageHead_From = ((BitRam[0] - 0x30)*1000 + (BitRam[1] - 0x30)*100 + (BitRam[2] - 0x30)*10 + (BitRam[3] - 0x30));
                        WriteFlashParameter();
                    }
                    break;
                case KEY_3:
                    BitValue = 0;
                    OptionBit = 0;
                    GUI_ShowWindow(Window_Menu, WM_SHOW);
                    break;

            }
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

