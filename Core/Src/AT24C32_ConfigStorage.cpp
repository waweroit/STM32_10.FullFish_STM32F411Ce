/*
 * AT24C32_ConfigStorage.cpp
 *
 *  Created on: Jan 17, 2026
 *      Author: wawer
 */



#include "AT24C32_ConfigStorage.hpp"

uint32_t AT24C32_ConfigStorage::Crc32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            uint32_t mask = (crc & 1u) ? 0xFFFFFFFFu : 0u;
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return ~crc;
}

uint32_t AT24C32_ConfigStorage::CalcBlobCrc(const Blob &b)
{
    return Crc32(reinterpret_cast<const uint8_t*>(&b),
                 static_cast<uint32_t>(sizeof(Blob) - sizeof(b.crc32)));
}

bool AT24C32_ConfigStorage::ValidateBlobHeader(const Blob &b)
{
    if (b.magic != kMagic) return false;
    if (b.version != kVersion) return false;
    if (b.size != sizeof(Blob)) return false;
    return true;
}

void AT24C32_ConfigStorage::Pack(Blob &b, const DeviceConfig &dev, const AquaConfig &aqua)
{
    std::memcpy(b.dev_bytes,  &dev,  sizeof(DeviceConfig));
    std::memcpy(b.aqua_bytes, &aqua, sizeof(AquaConfig));
}

void AT24C32_ConfigStorage::Unpack(const Blob &b, DeviceConfig &dev, AquaConfig &aqua)
{
    std::memcpy(&dev,  b.dev_bytes,  sizeof(DeviceConfig));
    std::memcpy(&aqua, b.aqua_bytes, sizeof(AquaConfig));
}

AT24C32_ConfigStorage::AT24C32_ConfigStorage(I2C_HandleTypeDef *hi2c, uint8_t devAddress)
    : hi2c_(hi2c),
      devAddr8_(static_cast<uint16_t>(devAddress) << 1) // 7-bit -> 8-bit
{
}

HAL_StatusTypeDef AT24C32_ConfigStorage::WaitReady(uint32_t timeoutMs)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeoutMs)
    {
        if (HAL_I2C_IsDeviceReady(hi2c_, devAddr8_, 1, 5) == HAL_OK)
            return HAL_OK;
    }
    return HAL_TIMEOUT;
}

HAL_StatusTypeDef AT24C32_ConfigStorage::ReadBytes(uint16_t memAddr, uint8_t *buf, uint16_t len)
{
    if (!buf || len == 0) return HAL_ERROR;
    if ((uint32_t)memAddr + len > kEepromSizeBytes) return HAL_ERROR;

    return HAL_I2C_Mem_Read(hi2c_, devAddr8_,
                            memAddr, I2C_MEMADD_SIZE_16BIT,
                            buf, len, 100);
}

HAL_StatusTypeDef AT24C32_ConfigStorage::WriteBytes(uint16_t memAddr, const uint8_t *buf, uint16_t len)
{
    if (!buf || len == 0) return HAL_ERROR;
    if ((uint32_t)memAddr + len > kEepromSizeBytes) return HAL_ERROR;

    while (len > 0)
    {
        uint16_t pageOff  = memAddr % kPageSizeBytes;
        uint16_t pageLeft = kPageSizeBytes - pageOff;
        uint16_t chunk    = (len < pageLeft) ? len : pageLeft;

        HAL_StatusTypeDef st = HAL_I2C_Mem_Write(hi2c_, devAddr8_,
                                                 memAddr, I2C_MEMADD_SIZE_16BIT,
                                                 (uint8_t*)buf, chunk, 100);
        if (st != HAL_OK) return st;

        st = WaitReady(20);
        if (st != HAL_OK) return st;

        memAddr += chunk;
        buf     += chunk;
        len     -= chunk;
    }
    return HAL_OK;
}

bool AT24C32_ConfigStorage::IsStoredDataValid()
{
    Blob b{};
    if (ReadBytes(kBaseAddr, reinterpret_cast<uint8_t*>(&b), sizeof(Blob)) != HAL_OK)
        return false;

    if (!ValidateBlobHeader(b))
        return false;

    return CalcBlobCrc(b) == b.crc32;
}

HAL_StatusTypeDef AT24C32_ConfigStorage::Save(const DeviceConfig *dev, const AquaConfig *aqua)
{
    if (!dev || !aqua) return HAL_ERROR;

    Blob b{};
    b.magic   = kMagic;
    b.version = kVersion;
    b.size    = static_cast<uint16_t>(sizeof(Blob));

    Pack(b, *dev, *aqua);

    b.crc32 = CalcBlobCrc(b);

    return WriteBytes(kBaseAddr,
                      reinterpret_cast<const uint8_t*>(&b),
                      static_cast<uint16_t>(sizeof(Blob)));
}

HAL_StatusTypeDef AT24C32_ConfigStorage::Load(DeviceConfig *dev, AquaConfig *aqua)
{
    if (!dev || !aqua) return HAL_ERROR;

    Blob b{};
    HAL_StatusTypeDef st = ReadBytes(kBaseAddr,
                                     reinterpret_cast<uint8_t*>(&b),
                                     static_cast<uint16_t>(sizeof(Blob)));
    if (st != HAL_OK) return st;

    if (!ValidateBlobHeader(b)) return HAL_ERROR;
    if (CalcBlobCrc(b) != b.crc32) return HAL_ERROR;

    Unpack(b, *dev, *aqua);
    return HAL_OK;
}
