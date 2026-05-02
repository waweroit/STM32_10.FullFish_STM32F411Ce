/*
 * W25Q64.h
 *
 *  Created on: Aug 3, 2025
 *      Author: wawer
 */

#pragma once
#ifndef INC_W25Q64_H_
#define INC_W25Q64_H_

//Strona (Page) = 256 bajtów
//Sektor = 4096 bajtów = 16 stron
//Cała pamięć to 8 MB = 8388608 bajtów = 32768 stron

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define FLASH_MAGIC 0xA5A5BEEF  // dowolna niezerowa, charakterystyczna wartosc

#define W25Q64_PAGE_SIZE       256
#define W25Q64_SECTOR_SIZE     4096
#define W25Q64_BLOCK_SIZE      65536
#define W25Q64_TIMEOUT         1000

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
} W25Q_HandleTypeDef;

void W25Q_Init(W25Q_HandleTypeDef *w25q, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);

bool W25Q_Read(W25Q_HandleTypeDef *w25q, uint32_t addr, uint8_t *buf, uint32_t len);
bool W25Q_Write(W25Q_HandleTypeDef *w25q, uint32_t addr, const uint8_t *buf, uint32_t len);

bool W25Q_EraseSector(W25Q_HandleTypeDef *w25q, uint32_t addr);
bool W25Q_EraseBlock64K(W25Q_HandleTypeDef *w25q, uint32_t addr);
bool W25Q_EraseChip(W25Q_HandleTypeDef *w25q);

uint8_t W25Q_ReadStatusReg1(W25Q_HandleTypeDef *w25q);
void    W25Q_WriteEnable(W25Q_HandleTypeDef *w25q);
void    W25Q_WaitForReady(W25Q_HandleTypeDef *w25q);

#endif /* INC_W25Q64_H_ */
