#ifndef TwoWireSlave_h
#define TwoWireSlave_h
#ifdef ARDUINO_ARCH_ESP32

#include <stdint.h>
#include <driver/i2c.h>

#define I2C_AH_BUFFER_LENGTH 128

class TwoWireSlave
{
  public:
    /**
     * @brief Constructor for TwoWireSlave class.
     * @param bus_num The I2C bus number to use (0 or 1).
     */
    TwoWireSlave(uint8_t bus_num);

    /**
     * @brief Destructor for TwoWireSlave class.
     * Cleans up the I2C driver and flushes buffers.
     */
    ~TwoWireSlave();

    /**
     * @brief Initializes the I2C slave driver with specified pins and address.
     * @param sda GPIO number for SDA.
     * @param scl GPIO number for SCL.
     * @param address I2C slave address (7-bit).
     * @return true if initialization was successful, false otherwise.
     */
    bool begin(int sda, int scl, int address);

    /**
     * @brief Writes data to the I2C slave transmission buffer.
     * @param data Pointer to the data to write.
     * @param size Number of bytes to write.
     * @return The number of bytes written, or -1 on error.
     */
    int write_buff(uint8_t *data, size_t size);

    /**
     * @brief Reads data from the I2C slave reception buffer.
     * @param data Pointer to the buffer to store the read data.
     * @param size Maximum number of bytes to read.
     * @return The number of bytes read, or -1 on error.
     */
    int read_buff(uint8_t *data, size_t size);

    /**
     * @brief Resets/flushes both RX and TX hardware FIFOs.
     */
    void flush(void);

  private:
    uint8_t num;
    i2c_port_t portNum;
    int8_t sda;
    int8_t scl;
};


extern TwoWireSlave WireSlave;
extern TwoWireSlave WireSlave1;

#endif      // ifdef ARDUINO_ARCH_ESP32
#endif      // ifndef TwoWireSlave_h
