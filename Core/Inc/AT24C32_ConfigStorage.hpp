/*
 * AT24C32_ConfigStorage.hpp
 *
 *  Created on: Jan 17, 2026
 *      Author: wawer
 */

#ifndef INC_AT24C32_CONFIGSTORAGE_HPP_
#define INC_AT24C32_CONFIGSTORAGE_HPP_

#include <cstdint>
#include <cstring>

extern "C" {
#include "i2c.h"
}

#include "EnviromentConfiguration.hpp"

class IConfigStorage {
public:
    virtual ~IConfigStorage() = default;

    virtual HAL_StatusTypeDef Save(const DeviceConfig *dev, const AquaConfig *aqua) = 0;
    virtual HAL_StatusTypeDef Load(DeviceConfig *dev, AquaConfig *aqua) = 0;

    virtual bool IsStoredDataValid() = 0;
};

class AT24C32_ConfigStorage : public IConfigStorage {
public:
    // devAddress podaj jako 7-bit (np. 0x57)
    AT24C32_ConfigStorage(I2C_HandleTypeDef *hi2c, uint8_t devAddress);

    HAL_StatusTypeDef Save(const DeviceConfig *dev, const AquaConfig *aqua) override;
    HAL_StatusTypeDef Load(DeviceConfig *dev, AquaConfig *aqua) override;
    bool IsStoredDataValid() override;

private:
    I2C_HandleTypeDef *hi2c_;
    uint16_t devAddr8_;

    static constexpr uint16_t kEepromSizeBytes = 4096;
    static constexpr uint16_t kPageSizeBytes  = 32;
    static constexpr uint16_t kBaseAddr       = 0x0000;

    static constexpr uint32_t kMagic   = 0xA24C32C1u;
    static constexpr uint16_t kVersion = 1;

#if defined(__GNUC__)
    struct __attribute__((packed)) Blob {
        uint32_t magic;
        uint16_t version;
        uint16_t size; // sizeof(Blob)

        // UWAGA: trzymamy dane jako bajty, żeby Blob był POD i packed działał
        uint8_t dev_bytes[sizeof(DeviceConfig)];
        uint8_t aqua_bytes[sizeof(AquaConfig)];

        uint32_t crc32; // CRC po polach: magic..aqua_bytes (bez crc32)
    };
#else
#pragma pack(push, 1)
    struct Blob {
        uint32_t magic;
        uint16_t version;
        uint16_t size;
        uint8_t dev_bytes[sizeof(DeviceConfig)];
        uint8_t aqua_bytes[sizeof(AquaConfig)];
        uint32_t crc32;
    };
#pragma pack(pop)
#endif

    static_assert(sizeof(Blob) <= kEepromSizeBytes, "Config blob too large for AT24C32");

    HAL_StatusTypeDef ReadBytes(uint16_t memAddr, uint8_t *buf, uint16_t len);
    HAL_StatusTypeDef WriteBytes(uint16_t memAddr, const uint8_t *buf, uint16_t len);
    HAL_StatusTypeDef WaitReady(uint32_t timeoutMs);

    static uint32_t Crc32(const uint8_t *data, uint32_t len);
    static uint32_t CalcBlobCrc(const Blob &b);
    static bool ValidateBlobHeader(const Blob &b);

    static void Pack(Blob &b, const DeviceConfig &dev, const AquaConfig &aqua);
    static void Unpack(const Blob &b, DeviceConfig &dev, AquaConfig &aqua);
};

#endif /* INC_AT24C32_CONFIGSTORAGE_HPP_ */
