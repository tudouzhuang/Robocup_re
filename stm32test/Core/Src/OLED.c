#include "main.h"
#include "OLED.h"
#include "OLED_Font.h"

/* 引脚配置：锁定 PB6 (SCL) 和 PB7 (SDA) */
#define OLED_W_SCL(x) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (GPIO_PinState)(x))
#define OLED_W_SDA(x) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (GPIO_PinState)(x))

/* 软件IIC 引脚初始化 */
void OLED_I2C_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // 关键：复用推挽改为开漏输出
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; 
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        OLED_W_SDA(!!(Byte & (0x80 >> i)));
        OLED_W_SCL(1);
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1); // 额外的一个时钟，不处理应答信号
    OLED_W_SCL(0);
}

void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78); // 从机地址
    OLED_I2C_SendByte(0x00); // 写命令
    OLED_I2C_SendByte(Command); 
    OLED_I2C_Stop();
}

void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78); // 从机地址
    OLED_I2C_SendByte(0x40); // 写数据
    OLED_I2C_SendByte(Data);
    OLED_I2C_Stop();
}

void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);                  // 设置Y位置
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));  // 设置X位置高4位
    OLED_WriteCommand(0x00 | (X & 0x0F));         // 设置X位置低4位
}

void OLED_Clear(void)
{  
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for(i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{       
    uint8_t i;
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);       
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]);          
    }
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);   
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);      
    }
}

void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)                            
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

void OLED_Init(void)
{
    uint32_t i, j;
    for (i = 0; i < 1000; i++)          //上电延时
    {
        for (j = 0; j < 1000; j++);
    }
    
    OLED_I2C_Init();            //端口初始化
    
    OLED_WriteCommand(0xAE);    //关闭显示
    OLED_WriteCommand(0xD5);    //设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8);    //设置多路复用率
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3);    //设置显示偏移
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);    //设置显示开始行
    OLED_WriteCommand(0xA1);    //设置左右方向
    OLED_WriteCommand(0xC8);    //设置上下方向
    OLED_WriteCommand(0xDA);    //设置COM引脚硬件配置
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81);    //设置对比度控制
    OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9);    //设置预充电周期
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB);    //设置VCOMH取消选择级别
    OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4);    //设置整个显示打开/关闭
    OLED_WriteCommand(0xA6);    //设置正常/倒转显示
    OLED_WriteCommand(0x8D);    //设置充电泵
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF);    //开启显示
        
    OLED_Clear();               //OLED清屏
}