/**************************************************************

  Copyright (C),WangJinLong, 2017 深圳市德派森科技发展有限公司

***************************************************************
 * 文 件 名   : lcd.c
 * 版 本 号   : V1.0
 * 作    者   : WangJinLong
 * 生成日期   : 2017年7月5日
***************************************************************/

#define LCD_PRIVATE

#include "stm8l15x_conf.h"

/*
如果SEG不是0-7的顺序，比如改为0-3，6-9，中间空出两个引脚
修改方法如下
1.更改LCD_GLASS_Init()中的端口LCD_PortMaskRegister
2.更改数组LCD_MAP[8][MAX_LCD_SEG][2]

*/





const uint8_t LCD_CHAR[] =
{

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*0x00-0x0F*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*0x10-0x1F*/
    /* space !    "    #    $    %    &    ￡    (    )   *    +    ,    -    .    /    */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x80, 0x00, /*0x20-0x2F*/
    /*  0     1   2    3    4    5    6    7    8    9    :    ;    <    =    >    ?    */
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, /*0x30-0x3F*/
    /*  @     A    B    C    D    E    F    G    H    I    J    K    L    M    N    O   */
    0x00, 0x77, 0x7f, 0x39, 0x5E, 0x79, 0x71, 0x00, 0x76, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x3f, /*0x40-0x4F*/
    /*    P    Q   R     S    T   U    V     W    X    Y    Z        */
    0x73, 0x67, 0x50, 0x6d, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*0x50-0x5F*/
    /*    '    a    b    c    d    e    f    g   h     i    j    k    l    m    n    o   */
    0x00, 0x5c, 0x7c, 0x58, 0x5e, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x5c, /*0x60-0x6F*/
    /*    p    q    r    s    t    u   v     w    x    y    z        */
    0x73, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*0x70-0x7F*/

};

const char LCD_MAP[8][MAX_LCD_SEG][2] =
{
    {
        // 1
        //{com, seg}
        // A       // B     // C     // D
        {0, 0}, {0, 1}, {2, 1}, {3,  0},
        // E      // F     // G     // P1
        {2, 0}, {1, 0}, {1, 1}, {3,  1},
    },

    {
        // 2
        //{com, seg}
        // A      // B     // C     // D
        {0, 2}, {0, 3}, {2, 3}, {3,  2},
        // E     // F     // G     //P2
        {2, 2}, {1, 2}, {1, 3}, {3,  3},
    },

    {
        // 3
        //{com, seg}
        // A      // B    // C     // D
        {0, 4}, {0, 5}, {2, 5}, {3,  4},
        // E     // F     // G     //P3
        {2, 4}, {1, 4}, {1, 5}, {3,  5},
    },

    {
        // 4
        //{com, seg}
        // A      // B     // C     // D
        {0, 6}, {0, 7}, {2, 7}, {3,  6},
        // E      // F     // G     // P4
        {2, 6}, {1, 6}, {1, 7}, {0,  8},
    },
};
void LCD_GLASS_Init(void)
{
    uint8_t status;
    /*
      The LCD is configured as follow:
       - clock source = LSE (32.768 KHz)
       - Voltage source = Internal
       - Prescaler = 8
       - Divider = 17
       - Mode = 1/4 Duty, 1/3 Bias
       - LCD frequency = (clock source * Duty) / (Prescaler * Divider)
                       = 60 Hz */

    /* Enable LCD clock */
    CLK_PeripheralClockConfig(CLK_Peripheral_LCD, ENABLE);
    CLK_RTCClockConfig(CLK_RTCCLKSource_HSI, CLK_RTCCLKDiv_64);

    /* Initialize the LCD */
    LCD_Init(LCD_Prescaler_8, LCD_Divider_16, LCD_Duty_1_4,
             LCD_Bias_1_3, LCD_VoltageSource_Internal);

    /* Mask register*/
    LCD_PortMaskConfig(LCD_PortMaskRegister_0, 0xFF);//SEG0~SEG7
    LCD_PortMaskConfig(LCD_PortMaskRegister_1, 0x07);//SEG8~SEG10

    LCD_ContrastConfig(LCD_Contrast_Level_7); //配置LCD的对比度0---7
    LCD_DeadTimeConfig(LCD_DeadTime_1);              // 配置LCD液晶的死区时间0--7
    //decrease power consumption
    LCD_PulseOnDurationConfig(LCD_PulseOnDuration_5);

    LCD_Cmd(ENABLE); /*!< Enable LCD peripheral */

}




/*
P1(com,seg)=(4,0)
P2(com,seg)=(4,2)
P3(com,seg)=(4,4)
P4(com,seg)=(4,6)
*/
void LCD_SetPixel(uint8_t Com, uint8_t Seg, LCD_PIXEL_SET_TypeDef On_Off)
{
    uint8_t x, com_offset, Seg_offset;

    if(Com > MAX_LCD_COM - 1)
    {
        return;
    }

    x = Com * 28 + Seg;
    com_offset = x / 8;
    Seg_offset = x % 8;

    LCD->CR4 &= (uint8_t)~LCD_CR4_PAGECOM;
    if(Com > 3)
    {
        LCD->CR4 |= LCD_CR4_PAGECOM;
    }

    if(On_Off == PIXEL_ON)
    {
        LCD->RAM[com_offset] |= 0x01 << Seg_offset;
    }
    else
    {
        LCD->RAM[com_offset] &= (uint8_t)~(0x01 << Seg_offset);
    }

}


void LCD_DisplayChar(uint8_t ch, uint8_t position, uint8_t signbit)
{
    uint8_t data, i;
    char com, seg;


    data = LCD_CHAR[ch];

    if(signbit)
    {
        data |= 0x80;
    }

    for(i = 0; i < MAX_LCD_SEG; i++)
    {
        //        数组首地址     +   COM位码偏移 + 段码偏移
        com = *((char*)(LCD_MAP) + position * 16 + i * 2);
        seg = *((char*)(LCD_MAP) + position * 16 + i * 2 + 1);
        LCD_SetPixel(com, seg, PIXEL_OFF);
        if (data & (1 << i))
        {
            /* Turn on segment */
            LCD_SetPixel(com, seg, PIXEL_ON);
        }
    }

}

//支持 “12:40”,"1.2.3.4"," 123","123","123 "," 1.23","1.2:.3.4","1.2.:3.4"
void LCD_Printf(char *string)
{
    uint8_t length, i, position, signbit;

    length = strlen(string);
    position = 0;
    signbit = 0;
    for(i = 0; i < length; i++)
    {
        if((*string) != '.')
        {
            LCD_DisplayChar(*(string++), position++, signbit);
            signbit = 0;
            /* 如果字符串长度大于4个字符，则退出（不含符号位）*/
            if(position == MAX_LCD_COM)
            {
                return;
            }

        }
        else if((*string) == '.')
        {
            signbit = 1;
            LCD_DisplayChar(*(string - 1), position - 1, signbit);
            signbit = 0;
            string++;
        }
    }

}
//显示特殊字段
void LCD_PrintfSpecialChar(unsigned char Pos, unsigned char PixelOnoff)
{
    switch(Pos)
    {
        case SPECIALCHAR_MA:
        {
            LCD_SetPixel(3, 7, PIXEL_ON);
            LCD_SetPixel(2, 8, PIXEL_OFF);
        }
        break;
        case SPECIALCHAR_T:
        {
            LCD_SetPixel(3, 7, PIXEL_OFF);
            LCD_SetPixel(2, 8, PIXEL_ON);
        }
        break;
        case SPECIALCHAR_A:
        {
            LCD_SetPixel(3, 7, PIXEL_OFF);
            LCD_SetPixel(2, 8, PIXEL_OFF);
        }
        break;
        case SPECIALCHAR_SINGAL:
        {
            LCD_SetPixel(3, 10, PixelOnoff);
        }
        break;
        default:
            break;
    }

}
//显示通道数据(包括阈值和实时数据)
//#define CHANNEL_TYPE_LEAKAGE    0x1000
//#define CHANNEL_TYPE_TEMP       0x2000
//#define CHANNEL_TYPE_CURRENT    0x3000
//#define CHANNEL_ALARM_VALUE     0x0FFF
//Ch通道号
//Data通道值
void LCD_DesktopDisp(uint8_t Ch, uint16_t Data)
{
    char BitChar[8] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
    uint8_t Fun;
    uint16_t RealData;
    if(Ch > CHANNEL_NUM)
    {
        return;
    }
    Fun = (ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch] >> 12);
    if(Fun == SPECIALCHAR_A)//电流显示1位小数,电流值比实际值扩大了10倍
    {
        RealData = Data;
        BitChar[0] = (Ch + 1 + '0');
        BitChar[1] = '.';
        BitChar[2] = RealData / 1000 + '0';
        BitChar[3] = RealData % 1000 / 100 + '0';
        BitChar[4] = (RealData % 100) / 10 + '0';
        BitChar[5] = (RealData % 10) + '0';

        if(BitChar[2] != '0')//电流值超过1000A(即实际值超过100A)
        {
            BitChar[5] = '.';//小数点显示在末尾
            BitChar[6] = '\0';
        }
        else//BitChar[2] == '0'
        {
            BitChar[2] = ' ';
            if(BitChar[3] == '0')
            {
                BitChar[3] = ' ';
            }
            BitChar[6] = BitChar[5];
            BitChar[5] = '.';
        }
    }
    else
    {
        RealData = Data;
        BitChar[0] = (Ch + 1 + '0');
        BitChar[1] = '.';
        BitChar[2] = RealData / 100 + '0';
        BitChar[3] = (RealData % 100) / 10 + '0';
        BitChar[4] = (RealData % 10) + '0';

        if(BitChar[2] != '0')
        {
            BitChar[5] = '\0';
        }
        else//BitChar[2] == '0'
        {
            BitChar[2] = ' ';
            if(BitChar[3] == '0')
            {
                BitChar[3] = ' ';
            }
            BitChar[5] = '\0';
        }
    }
    if((ParameterBuffer.ParameterConfig.ChannelAlarmValue[Ch] & 0x0FFF) == 0x0000)//通道关闭
    {
        BitChar[2] = 'O';
        BitChar[3] = 'F';
        BitChar[4] = 'F';
        BitChar[5] = '\0';
    }
    else if((ParameterSysStatus.ChannelState[Ch] & CHANNEL_SHORT) == CHANNEL_SHORT)
    {
        BitChar[2] = 'E';
        BitChar[3] = 'R';
        BitChar[4] = '1';
        BitChar[5] = '\0';
    }
    else if((ParameterSysStatus.ChannelState[Ch] & CHANNEL_OPEN) == CHANNEL_OPEN)
    {
        BitChar[2] = 'E';
        BitChar[3] = 'R';
        BitChar[4] = '2';
        BitChar[5] = '\0';
    }
    LCD_Printf(BitChar);
    LCD_PrintfSpecialChar(Fun, PIXEL_ON); //显示通道单位
}