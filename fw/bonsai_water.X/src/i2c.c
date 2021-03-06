/**
 * @file i2c.c
 * @brief 
 * @author Adrian Alvarez
 * @version 1.0
 * @date 2015-08-31
 */

/* Copyright (C) 
 * 2015 - Adrian Alvarez
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include <xc.h>

#include <stdint.h>
#include "user.h"
#include "log.h"
#include "i2c.h"

#define I2C_BR_100KHz 60 /*  I2CxBRG value for 100 KHz baudrate @16 MIPs */

/**
* \brief Configure I2C peripheral to communicate with other IC in the board.
*/
void configureI2C()
{
    /* Set desired baudrate of the bus */
    I2C3BRG = I2C_BR_100KHz;
    I2C3CON = 0;
    I2C3CONbits.I2CEN = 1;
}


/**
 * @brief 
 *
 * @param device_add
 * @param address
 * @param dlen
 * @param din
 *
 * @return 
 */
int readSeqRegisters(uint8_t device_add, int address, int dlen, uint8_t *dout)
{
    uint8_t nBytes;
    I2C3CONbits.SEN = 1;
    while(I2C3CONbits.SEN != 0)  continue;
    I2C3TRN = device_add;

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received from device");
        return 0;
    }
    I2C3TRN = (address & 0xFF);

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received writting I2C dev address");
        return 0;
    }

    I2C3CONbits.RSEN = 1; 
    while(I2C3CONbits.RSEN != 0)  continue;
    I2C3TRN = device_add + 1; /* I2C device address in read mode */

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received after restart and writting"\
            "register address");
        return 0;
    }

    for( nBytes = 0; nBytes<dlen; nBytes++){
        I2C3CONbits.RCEN = 1;
        while(I2C3CONbits.RCEN != 0)  continue;
        *(dout+nBytes) = (I2C3RCV & 0xFF) ;
        /*  Send nack when last byte was already sent */
        I2C3CONbits.ACKDT = ( nBytes!=dlen-1 ) ? 0 : 1;
        I2C3CONbits.ACKEN = 1;
        while( I2C3CONbits.ACKEN != 0 ) continue;
    }

    I2C3CONbits.PEN = 1;
    while(I2C3CONbits.PEN != 0)  continue;
    for(int i=0;i<1000; i++)
        continue;
    return 1;
}


/**
 * @brief 
 * @brief Read from I2C bus the register addres from a i2c device.
 * |START|I2Cdev(write)|REG_ADDSS|REG_ADDSS|RESTART|i2cdev(read)|value|
 * @param device_add This is the i2c device address (last bit should be 0)
 * @param address Address of the register that will be read.
 * @param value Point where value are going to be written.
 */
int readSeqRegisters16add(uint8_t device_add, uint16_t address, int dlen, uint8_t *dout)
{
    uint8_t nBytes;
    I2C3CONbits.SEN = 1;
    while(I2C3CONbits.SEN != 0)  continue;
    I2C3TRN = device_add;

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received from device");
        return 0;
    }
    I2C3TRN =((address >> ADD_HIGH_WORD) & ADD_8_BITS_MASK);

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received from device");
        return 0;
    }
    I2C3TRN = (address & ADD_8_BITS_MASK);

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received writting I2C dev address");
        return 0;
    }

    I2C3CONbits.RSEN = 1; 
    while(I2C3CONbits.RSEN != 0)  continue;
    I2C3TRN = device_add + 1; /* I2C device address in read mode */

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received after restart and writting"\
            "register address");
        return 0;
    }

    for( nBytes = 0; nBytes<dlen; nBytes++){
        I2C3CONbits.RCEN = 1;
        while(I2C3CONbits.RCEN != 0)  continue;
        *(dout+nBytes) = (I2C3RCV & 0xFF) ;
        /*  Send nack when last byte was already sent */
        I2C3CONbits.ACKDT = ( nBytes!=dlen-1 ) ? 0 : 1;
        I2C3CONbits.ACKEN = 1;
        while( I2C3CONbits.ACKEN != 0 ) continue;
    }

    I2C3CONbits.PEN = 1;
    while(I2C3CONbits.PEN != 0)  continue;
    for(int i=0;i<1000; i++)
        continue;
    return 1;
}

/**
 * \brief Read from I2C bus the register addres from a i2c device.
 * |START|I2Cdev(write)|REG_ADDSS|RESTART|i2cdev(read)|value|
 * \param device_add This is the i2c device address (last bit should be 0)
 * \param address Address of the register that will be read.
 * \param value Point where value are going to be written.
 */
int readRegister(uint8_t device_add, int address, int *value)
{
    I2C3CONbits.SEN = 1;
    while(I2C3CONbits.SEN != 0)  continue;
    I2C3TRN = device_add;

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received from device");
        return 0;
    }
    I2C3TRN = (address & 0xFF);

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received writting I2C dev address");
        return 0;
    }

    I2C3CONbits.RSEN = 1; 
    while(I2C3CONbits.RSEN != 0)  continue;
    I2C3TRN = device_add + 1; /* I2C device address in write mode */

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received after restart and writting"\
            "register address");
        return 0;
    }

    I2C3CONbits.RCEN = 1;
    while(I2C3CONbits.RCEN != 0)  continue;
    *value = I2C3RCV ;
    I2C3CONbits.ACKDT = 1;
    I2C3CONbits.ACKEN = 1;
    I2C3CONbits.PEN = 1;
    while(I2C3CONbits.PEN != 0)  continue;
    for(int i=0;i<1000; i++)
        continue;
    return 1;
}



/**
 * @brief Read one value from device address
 *
 * @param device_add
 * @param value
 *
 * @return 
 */
int read(uint8_t device_add, int *value)
{
    I2C3CONbits.SEN = 1;
    while(I2C3CONbits.SEN != 0)  continue;
    I2C3TRN = device_add + 1;

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received reading");
        return 0;
    }

    I2C3CONbits.RCEN = 1;
    while(I2C3CONbits.RCEN != 0)  continue;
    *value = I2C3RCV ;
    I2C3CONbits.ACKDT = 1;
    I2C3CONbits.ACKEN = 1;
    I2C3CONbits.PEN = 1;
    while(I2C3CONbits.PEN != 0)  continue;
    for(int i=0;i<1000; i++)
        continue;
    return 1;
}


/**
 * @brief  Write register of a I2C device sequentially.
 *
 * @param device_add This i the i2c device address (last bit should be 0).
 * @param address Address of the first register that will be written
 * @param dlen Number of registers that will be updated
 * @param din Pointer to a buffer than contains the values of the registers\
 * thar will be updated
 *
 * @return Number of bytes written
 */
int writeSeqRegisters(uint8_t device_add, int address, int dlen, uint8_t *din)
{
    unsigned int n_bytes;
    I2C3CONbits.SEN = 1;
    while(I2C3CONbits.SEN != 0)  continue;
    I2C3TRN = device_add; /* I2C device address in write mode */

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received from device");
        I2C3CONbits.PEN = 1;
        while(I2C3CONbits.PEN != 0)  continue;
        return 0;
    }
    I2C3TRN = address;

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        I2C3CONbits.PEN = 1;
        while(I2C3CONbits.PEN != 0)  continue;
        LOG_WARN("I2C. Acknowledgement not received after writting"\
            "register address");
        return 0;
    }
    for(n_bytes = 0; n_bytes < dlen; n_bytes++){
        I2C3TRN = *(din+n_bytes);

        while(I2C3STATbits.TBF) continue;
        while(I2C3STATbits.TRSTAT) continue;
        if(I2C3STATbits.ACKSTAT){
            I2C3CONbits.PEN = 1;
            while(I2C3CONbits.PEN != 0)  continue;
            LOG_WARN("I2C. Acknowledgement not received after writting"\
                "register address");
            return n_bytes;
        }
    }
    I2C3CONbits.PEN = 1;
    while(I2C3CONbits.PEN != 0)  continue;
    return n_bytes;
}
/**
 * @brief  Write register of 16 bytes I2C device addres width sequentially.
 *
 * @param device_add This i the i2c device address (last bit should be 0).
 * @param address 16 bit address of the first register that will be written
 * @param dlen Number of registers that will be updated
 * @param din Pointer to a buffer than contains the values of the registers\
 * thar will be updated
 *
 * @return Number of bytes written
 */
int writeSeqRegisters16add(uint8_t device_add, uint16_t address, int dlen, uint8_t *din)
{
    unsigned int n_bytes;
    I2C3CONbits.SEN = 1;
    while(I2C3CONbits.SEN != 0)  continue;
    I2C3TRN = device_add; /* I2C device address in write mode */

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received from device");
        I2C3CONbits.PEN = 1;
        while(I2C3CONbits.PEN != 0)  continue;
        return 0;
    }
    I2C3TRN = ((address) >> ADD_HIGH_WORD) & ADD_8_BITS_MASK;

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received from device");
        I2C3CONbits.PEN = 1;
        while(I2C3CONbits.PEN != 0)  continue;
        return 0;
    }
    I2C3TRN = (address) & ADD_8_BITS_MASK;

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        I2C3CONbits.PEN = 1;
        while(I2C3CONbits.PEN != 0)  continue;
        LOG_WARN("I2C. Acknowledgement not received after writting"\
            "register address");
        return 0;
    }
    for(n_bytes = 0; n_bytes < dlen; n_bytes++){
        I2C3TRN = *(din+n_bytes);

        while(I2C3STATbits.TBF) continue;
        while(I2C3STATbits.TRSTAT) continue;
        if(I2C3STATbits.ACKSTAT){
            I2C3CONbits.PEN = 1;
            while(I2C3CONbits.PEN != 0)  continue;
            LOG_WARN("I2C. Acknowledgement not received after writting"\
                "register address");
            return n_bytes;
        }
    }
    I2C3CONbits.PEN = 1;
    while(I2C3CONbits.PEN != 0)  continue;
    return n_bytes;
}


/**
 * \brief Write to I2C bus the register address of a i2c device.
 * |START|I2Cdev(write)|value|
 * \param device_add This is the i2c device address (last bit should be 0)
 * \param address Address of the register that will be written.
 * \param value Value that are going to be written in the register.
 */
int writeRegister(uint8_t device_add, int address, int value)
{
    I2C3CONbits.SEN = 1;
    while(I2C3CONbits.SEN != 0)  continue;
    I2C3TRN = device_add; /* I2C device address in write mode */

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received from device");
        I2C3CONbits.PEN = 1;
        while(I2C3CONbits.PEN != 0)  continue;
        return 0;
    }
    I2C3TRN = address;

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        I2C3CONbits.PEN = 1;
        while(I2C3CONbits.PEN != 0)  continue;
        LOG_WARN("I2C. Acknowledgement not received after writting register address");
        return 0;
    }
    I2C3TRN = value;

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        I2C3CONbits.PEN = 1;
        while(I2C3CONbits.PEN != 0)  continue;
        LOG_WARN("I2C. Acknowledgement not received after writting register address");
        return 0;
    }
    I2C3CONbits.PEN = 1;
    while(I2C3CONbits.PEN != 0)  continue;
    return 1;
}



/**
 * @brief Write in i2c bus the number of bytes indicated
 *
 * @param device_add Address finished in zero of the I2C device.
 * @param data Pointer to a buffer that contain the data to be transmited
 * @param nData Number of bytes that are going to be send
 *
 * @return 
 */
int writeI2Cdata( uint8_t device_add, uint8_t *data, uint8_t nData)
{
    unsigned int n_bytes;
    I2C3CONbits.SEN = 1;
    while(I2C3CONbits.SEN != 0)  continue;
    I2C3TRN = device_add; /* I2C device address in write mode */

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    if(I2C3STATbits.ACKSTAT){
        LOG_WARN("I2C. Acknowledgement not received from device");
        I2C3CONbits.PEN = 1;
        while(I2C3CONbits.PEN != 0)  continue;
        return 0;
    }

    for(n_bytes = 0; n_bytes < nData; n_bytes++){
        I2C3TRN = *(data+n_bytes);

        while(I2C3STATbits.TBF) continue;
        while(I2C3STATbits.TRSTAT) continue;
        if(I2C3STATbits.ACKSTAT){
            I2C3CONbits.PEN = 1;
            while(I2C3CONbits.PEN != 0)  continue;
            LOG_WARN("I2C. Acknowledgement not received after writting");
            return n_bytes;
        }
    }
    I2C3CONbits.PEN = 1;
    while(I2C3CONbits.PEN != 0)  continue;
    return n_bytes;
}

/**
 * @brief Check whether a I2C device is available or not
 *
 * @param device_add Address of the device that will be checked (last bit
 * should be 0)
 *
 * @return 1 if device is available 0 in other case
 */
int isDevAvailable(uint8_t device_add)
{
    I2C3CONbits.SEN = 1;
    while(I2C3CONbits.SEN != 0)  continue;
    I2C3TRN = device_add; /* I2C device address in write mode */

    while(I2C3STATbits.TBF)continue;
    while(I2C3STATbits.TRSTAT) continue;
    return I2C3STATbits.ACKSTAT ? 0u : 1u;
}
