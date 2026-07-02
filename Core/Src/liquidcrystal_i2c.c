/*
 * liquidcrystal_i2c.c
 *
 *  Created on: Jan 28, 2024
 *      Author: wawer
 */
#include "liquidcrystal_i2c.h"

uint8_t dpFunction;
uint8_t dpControl;
uint8_t dpMode;
uint8_t dpRows;
uint8_t dpBacklight;
I2C_HandleTypeDef *i2cPtr;

static uint8_t SendCommand(uint8_t);
static uint8_t SendChar(uint8_t);
static uint8_t Send(uint8_t, uint8_t);
static uint8_t Write4Bits(uint8_t);
static HAL_StatusTypeDef ExpanderWrite(uint8_t);
static uint8_t PulseEnable(uint8_t);


uint8_t special1[8] = {
        0b00000,
        0b11001,
        0b11011,
        0b00110,
        0b01100,
        0b11011,
        0b10011,
        0b00000
};

uint8_t special2[8] = {
        0b11000,
        0b11000,
        0b00110,
        0b01001,
        0b01000,
        0b01001,
        0b00110,
        0b00000
};


uint8_t Initialize_HD44780(I2C_HandleTypeDef *I2Ccomunication)
{
	i2cPtr = I2Ccomunication;

	if(!HD44780_Init(2))
		return 0;
	/* Clear buffer */
	if(!HD44780_Clear())
	return 0;
	/* Hide characters */
	HD44780_NoDisplay();
	HD44780_Cursor();

	if(!HD44780_SetCursor(0,0))
		return 0;

	HD44780_Backlight();

//	HD44780_PrintStr("HELLO STM32!!!");
//	/* Show characters */
//	HD44780_Display();
//	/* Move position */
//	HD44780_SetCursor(0, 1);
//	HD44780_PrintStr("BYE STM32!!!");
//	/* Blink cursor */
//	HD44780_Blink();
	HD44780_Display();
	return 1;
}


uint8_t HD44780_Init(uint8_t rows)
{
  dpRows = rows;

  dpBacklight = LCD_BACKLIGHT;

  dpFunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

  if (dpRows > 1)
  {
    dpFunction |= LCD_2LINE;
  }
  else
  {
    dpFunction |= LCD_5x10DOTS;
  }

  /* Wait for initialization */

  HAL_Delay(50);
  if(ExpanderWrite(dpBacklight))
  {}

  HAL_Delay(10);

  /* 4bit Mode */
  Write4Bits(0x03 << 4);
  HAL_Delay(100);

  Write4Bits(0x03 << 4);
  HAL_Delay(100);

  Write4Bits(0x03 << 4);
  HAL_Delay(100);

  Write4Bits(0x02 << 4);
  HAL_Delay(100);

  /* Display Control */
  if(!SendCommand(LCD_FUNCTIONSET | dpFunction))
	  return 0;

  dpControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  if(!HD44780_Display())
	  return 0;

  if(!HD44780_Clear())
	  return 0;

  /* Display Mode */
  dpMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  if(!SendCommand(LCD_ENTRYMODESET | dpMode))
	  return 0;
  HAL_Delay(450);

  HD44780_CreateSpecialChar(0, special1);
  HD44780_CreateSpecialChar(1, special2);

  if(!HD44780_Home())
	  return 0;
  return 1;
}

uint8_t HD44780_Clear()
{
  uint8_t ret = SendCommand(LCD_CLEARDISPLAY);
  //DelayUS(2000);//default 2000
  return ret;
}

uint8_t HD44780_Home()
{
  uint8_t ret = SendCommand(LCD_RETURNHOME);
  HAL_Delay(100);
  return ret;
}

uint8_t HD44780_SetCursor(uint8_t col, uint8_t row)
{
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if (row >= dpRows)
  {
    row = dpRows-1;
  }
  return SendCommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void HD44780_NoDisplay()
{
  dpControl &= ~LCD_DISPLAYON;
  SendCommand(LCD_DISPLAYCONTROL | dpControl);
}

uint8_t HD44780_Display()
{
  dpControl |= LCD_DISPLAYON;
  return SendCommand(LCD_DISPLAYCONTROL | dpControl);
}

void HD44780_NoCursor()
{
  dpControl &= ~LCD_CURSORON;
  SendCommand(LCD_DISPLAYCONTROL | dpControl);
}

void HD44780_Cursor()
{
  dpControl |= LCD_CURSORON;
  SendCommand(LCD_DISPLAYCONTROL | dpControl);
}

void HD44780_NoBlink()
{
  dpControl &= ~LCD_BLINKON;
  SendCommand(LCD_DISPLAYCONTROL | dpControl);
}

void HD44780_Blink()
{
  dpControl |= LCD_BLINKON;
  SendCommand(LCD_DISPLAYCONTROL | dpControl);
}

void HD44780_ScrollDisplayLeft(void)
{
  SendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void HD44780_ScrollDisplayRight(void)
{
  SendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void HD44780_LeftToRight(void)
{
  dpMode |= LCD_ENTRYLEFT;
  SendCommand(LCD_ENTRYMODESET | dpMode);
}

void HD44780_RightToLeft(void)
{
  dpMode &= ~LCD_ENTRYLEFT;
  SendCommand(LCD_ENTRYMODESET | dpMode);
}

void HD44780_AutoScroll(void)
{
  dpMode |= LCD_ENTRYSHIFTINCREMENT;
  SendCommand(LCD_ENTRYMODESET | dpMode);
}

void HD44780_NoAutoScroll(void)
{
  dpMode &= ~LCD_ENTRYSHIFTINCREMENT;
  SendCommand(LCD_ENTRYMODESET | dpMode);
}

uint8_t HD44780_CreateSpecialChar(uint8_t location, uint8_t charmap[])
{
  location &= 0x7;
  if(!SendCommand(LCD_SETCGRAMADDR | (location << 3)))
	  return 0;

  for (int i=0; i<8; i++)
  {
    if(!SendChar(charmap[i]))
    	return 0;
  }
  return 1;
}

/*void HD44780_PrintSpecialChar(uint8_t index)
{
  SendChar(index);
}*/

uint8_t HD44780_LoadCustomCharacter(uint8_t char_num, uint8_t *rows)
{
  return HD44780_CreateSpecialChar(char_num, rows);
}

uint8_t HD44780_PrintStr(char c[])
{
  while(*c)
	  {
	  	  if(!SendChar(*c++))
	  		  return 0;
	  }
  return 1;
}

void HD44780_SetBacklight(uint8_t new_val)
{
  if(new_val) HD44780_Backlight();
  else HD44780_NoBacklight();
}

void HD44780_NoBacklight(void)
{
  dpBacklight=LCD_NOBACKLIGHT;
  ExpanderWrite(0);
}

void HD44780_Backlight(void)
{
  dpBacklight=LCD_BACKLIGHT;
  ExpanderWrite(0);
}

static uint8_t SendCommand(uint8_t cmd)
{
  return Send(cmd, 0);
}

static uint8_t SendChar(uint8_t ch)
{
	return Send(ch, RS);
}

static uint8_t Send(uint8_t value, uint8_t mode)
{
  uint8_t highnib = value & 0xF0;
  uint8_t lownib = (value<<4) & 0xF0;
  uint8_t firstEx = Write4Bits((highnib)|mode);
  uint8_t secEx = Write4Bits((lownib)|mode);
  if(firstEx && secEx)
  		return 1;
  	else
  		return 0;
}

static uint8_t Write4Bits(uint8_t value)
{
	uint8_t firstEx 	= ExpanderWrite(value)== HAL_OK;
	uint8_t secEx 		= PulseEnable(value);
	if(firstEx && secEx)
		return 1;
	else
		return 0;
}

static HAL_StatusTypeDef ExpanderWrite(uint8_t _data)
{
  uint8_t data = _data | dpBacklight;
  return HAL_I2C_Master_Transmit(i2cPtr, DEVICE_ADDR, (uint8_t*)&data, 1, 10);
}

uint8_t PulseEnable(uint8_t _data)
{
  //ExpanderWrite(_data | ENABLE);
  uint8_t firstEx = ExpanderWrite(_data | _ENABLE) == HAL_OK;
  //DelayUS(20);//default 20

  //ExpanderWrite(_data & ~ENABLE);
  uint8_t secEx = ExpanderWrite(_data & ~_ENABLE) == HAL_OK;
  //DelayUS(20);//default 20
  if(firstEx && secEx)
	  return 1;
  else
	  return 0;
}

void TextByLine(const char* line1, const char* line2)
{
    // Opcjonalnie: ustaw tekst z powrotem na początku
    HD44780_Clear();
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr((char *)line1);
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr((char *)line2);
}

void ScrollTextOnce(const char* line1, const char* line2, int scrollDelay)
{
    int len1 = strlen(line1);
    int len2 = strlen(line2);
    int maxLen = len1 > len2 ? len1 : len2;  // Długość najdłuższego tekstu
    char displayLine1[17];
    char displayLine2[17];

    // Przewijaj tekst o jeden znak, aż do momentu, gdy wyświetlony będzie cały tekst
    for (int i = 0; i <= maxLen - 16; i++) {
        // Pobranie fragmentu linii do wyświetlenia
        strncpy(displayLine1, line1 + i, 16);
        strncpy(displayLine2, line2 + i, 16);
        displayLine1[16] = '\0';
        displayLine2[16] = '\0';

        // Wyczyszczenie ekranu i ustawienie kursora na początku
        HD44780_Clear();
        HD44780_SetCursor(0, 0);
        HD44780_PrintStr(displayLine1);
        HD44780_SetCursor(0, 1);
        HD44780_PrintStr(displayLine2);

        // Czas oczekiwania przed kolejnym przesunięciem
        HAL_Delay(scrollDelay*1000);
    }

    // Opcjonalnie: ustaw tekst z powrotem na początku
    HD44780_Clear();
    HD44780_SetCursor(0, 0);
    HD44780_PrintStr((char *)line1);
    HD44780_SetCursor(0, 1);
    HD44780_PrintStr((char *)line2);
    TextByLine(line1,line2);
}

uint8_t I2C_FindDevices(I2C_HandleTypeDef *hi2c, uint8_t *devAddresses, uint8_t maxLen)
{
    uint8_t found = 0;

    for (uint8_t addr7 = 1; addr7 < 127; addr7++)
    {
        if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(addr7 << 1), 1, 5) == HAL_OK)
        {
            if (found < maxLen)
            {
                devAddresses[found++] = addr7;   // zapisuj 7-bitowy adres
            }
            else
            {
                // brak miejsca w buforze - kończymy albo ignorujemy kolejne
                break;
            }
        }
    }

    return found;
}
