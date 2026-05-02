/*
 * W25Q64.c
 *
 *  Created on: Aug 3, 2025
 *      Author: wawer
 */




#include "W25Q64.h"
#include "waweroBasicFunction.h"

#define W25Q_CMD_READ_DATA    0x03
#define W25Q_CMD_PAGE_PROGRAM 0x02
#define W25Q_CMD_SECTOR_ERASE 0x20
#define W25Q_CMD_BLOCK_ERASE  0xD8
#define W25Q_CMD_CHIP_ERASE   0xC7
#define W25Q_CMD_WRITE_ENABLE 0x06
#define W25Q_CMD_READ_STATUS1 0x05

static inline void W25Q_CS_L(W25Q_HandleTypeDef *w25q) { HAL_GPIO_WritePin(w25q->cs_port, w25q->cs_pin, GPIO_PIN_RESET); }
static inline void W25Q_CS_H(W25Q_HandleTypeDef *w25q) { HAL_GPIO_WritePin(w25q->cs_port, w25q->cs_pin, GPIO_PIN_SET); }

void W25Q_Init(W25Q_HandleTypeDef *w25q, SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    w25q->hspi = hspi;
    w25q->cs_port = cs_port;
    w25q->cs_pin = cs_pin;
    W25Q_CS_H(w25q);
}

// --- Niskopoziomowe funkcje pomocnicze ---

void W25Q_WriteEnable(W25Q_HandleTypeDef *w25q)
{
    uint8_t cmd = W25Q_CMD_WRITE_ENABLE;
    W25Q_CS_L(w25q);
    HAL_SPI_Transmit(w25q->hspi, &cmd, 1, W25Q64_TIMEOUT);
    W25Q_CS_H(w25q);
}

uint8_t W25Q_ReadStatusReg1(W25Q_HandleTypeDef *w25q)
{
    uint8_t cmd = W25Q_CMD_READ_STATUS1;
    uint8_t status = 0;
    W25Q_CS_L(w25q);
    HAL_SPI_Transmit(w25q->hspi, &cmd, 1, W25Q64_TIMEOUT);
    HAL_SPI_Receive(w25q->hspi, &status, 1, W25Q64_TIMEOUT);
    W25Q_CS_H(w25q);
    return status;
}

void W25Q_WaitForReady(W25Q_HandleTypeDef *w25q)
{
    while (W25Q_ReadStatusReg1(w25q) & 0x01) {
    	DelayUS(100);
        //HAL_Delay(1);
    }
}

// --- Kasowanie (erase) ---

bool W25Q_EraseSector(W25Q_HandleTypeDef *w25q, uint32_t addr)
{
    W25Q_WriteEnable(w25q);
    uint8_t cmd[4] = { W25Q_CMD_SECTOR_ERASE, (addr>>16) & 0xFF, (addr>>8) & 0xFF, addr & 0xFF };
    W25Q_CS_L(w25q);
    bool ok = (HAL_SPI_Transmit(w25q->hspi, cmd, 4, W25Q64_TIMEOUT) == HAL_OK);
    W25Q_CS_H(w25q);
    W25Q_WaitForReady(w25q);
    return ok;
}

bool W25Q_EraseBlock64K(W25Q_HandleTypeDef *w25q, uint32_t addr)
{
    W25Q_WriteEnable(w25q);
    uint8_t cmd[4] = { W25Q_CMD_BLOCK_ERASE, (addr>>16) & 0xFF, (addr>>8) & 0xFF, addr & 0xFF };
    W25Q_CS_L(w25q);
    bool ok = (HAL_SPI_Transmit(w25q->hspi, cmd, 4, W25Q64_TIMEOUT) == HAL_OK);
    W25Q_CS_H(w25q);
    W25Q_WaitForReady(w25q);
    return ok;
}

bool W25Q_EraseChip(W25Q_HandleTypeDef *w25q)
{
    W25Q_WriteEnable(w25q);
    uint8_t cmd = W25Q_CMD_CHIP_ERASE;
    W25Q_CS_L(w25q);
    bool ok = (HAL_SPI_Transmit(w25q->hspi, &cmd, 1, W25Q64_TIMEOUT) == HAL_OK);
    W25Q_CS_H(w25q);
    W25Q_WaitForReady(w25q);
    return ok;
}

// --- Odczyt ---

bool W25Q_Read(W25Q_HandleTypeDef *w25q, uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint8_t cmd[4] = { W25Q_CMD_READ_DATA, (addr>>16) & 0xFF, (addr>>8) & 0xFF, addr & 0xFF };
    W25Q_CS_L(w25q);
    if (HAL_SPI_Transmit(w25q->hspi, cmd, 4, W25Q64_TIMEOUT) != HAL_OK) { W25Q_CS_H(w25q); return false; }
    bool ok = (HAL_SPI_Receive(w25q->hspi, buf, len, W25Q64_TIMEOUT) == HAL_OK);
    W25Q_CS_H(w25q);
    return ok;
}

// --- Zapis z podziałem na strony i automatycznym erase ---

bool W25Q_Write(W25Q_HandleTypeDef *w25q, uint32_t addr, const uint8_t *buf, uint32_t len)
{
    while (len > 0)
    {
        uint32_t page_offset = addr % W25Q64_PAGE_SIZE;
        uint32_t page_left = W25Q64_PAGE_SIZE - page_offset;
        uint32_t to_write = (len < page_left) ? len : page_left;

        // Przed zapisem należy wykonać erase sektora (4kB)
        uint32_t sector_addr = addr & ~(W25Q64_SECTOR_SIZE - 1);
        W25Q_EraseSector(w25q, sector_addr);

        W25Q_WriteEnable(w25q);

        uint8_t cmd[4] = { W25Q_CMD_PAGE_PROGRAM, (addr>>16)&0xFF, (addr>>8)&0xFF, addr&0xFF };
        W25Q_CS_L(w25q);
        if (HAL_SPI_Transmit(w25q->hspi, cmd, 4, W25Q64_TIMEOUT) != HAL_OK) { W25Q_CS_H(w25q); return false; }
        if (HAL_SPI_Transmit(w25q->hspi, (uint8_t*)buf, to_write, W25Q64_TIMEOUT) != HAL_OK) { W25Q_CS_H(w25q); return false; }
        W25Q_CS_H(w25q);

        W25Q_WaitForReady(w25q);

        addr += to_write;
        buf += to_write;
        len -= to_write;
    }
    return true;
}
