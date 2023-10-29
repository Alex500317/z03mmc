#include "tl_common.h"
#include "types.h"
#include "app_cfg.h"
#include "app_i2c.h"

#define _I2C_SEG_

_I2C_SEG_
void init_i2c(void) {
	i2c_gpio_set(I2C_GROUP); // I2C_GPIO_GROUP_C0C1, I2C_GPIO_GROUP_C2C3, I2C_GPIO_GROUP_B6D7, I2C_GPIO_GROUP_A3A4
	reg_i2c_speed = (unsigned char)(CLOCK_SYS_CLOCK_HZ/(4*I2C_CLOCK)); // 400 kHz
    //reg_i2c_id  = slave address
    reg_i2c_mode |= FLD_I2C_MASTER_EN; //enable master mode
	reg_i2c_mode &= ~FLD_I2C_HOLD_MASTER; // Disable clock stretching for Sensor

    reg_clk_en0 |= FLD_CLK0_I2C_EN;    //enable i2c clock
    reg_spi_sp  &= ~FLD_SPI_ENABLE;   //force PADs act as I2C; i2c and spi share the hardware of IC
}

unsigned char test_i2c_device(unsigned char address){
	unsigned char r = irq_disable();

	if ((reg_clk_en0 & FLD_CLK0_I2C_EN)==0)
		init_i2c();
#if I2C_CLOCK != 100000
	unsigned char s = reg_i2c_speed;
	reg_i2c_speed = (u8)(CLOCK_SYS_CLOCK_HZ/(4*100000)); // 100 kHz
#endif
	reg_i2c_id = (unsigned char) address;
	reg_i2c_ctrl = FLD_I2C_CMD_START | FLD_I2C_CMD_ID | FLD_I2C_CMD_STOP;
	while (reg_i2c_status & FLD_I2C_CMD_BUSY);
#if I2C_CLOCK != 100000
	reg_i2c_speed = s;
#endif
	irq_restore(r);
	return ((reg_i2c_status & FLD_I2C_NAK)? 0 : address);
}

_I2C_SEG_
int send_i2c(unsigned char i2c_addr, unsigned char * dataBuf, size_t dataLen) {
	int err = 0;
	unsigned char r = irq_disable();
	if ((reg_clk_en0 & FLD_CLK0_I2C_EN)==0)
			init_i2c();
	unsigned char * p = dataBuf;
	reg_i2c_id = i2c_addr;
	reg_i2c_ctrl = FLD_I2C_CMD_START | FLD_I2C_CMD_ID;
	while (reg_i2c_status & FLD_I2C_CMD_BUSY);
	err = reg_i2c_status & FLD_I2C_NAK;
	if(!err) {
		while (dataLen--) {
			reg_i2c_do = *p++;
			reg_i2c_ctrl = FLD_I2C_CMD_DO;
			while (reg_i2c_status & FLD_I2C_CMD_BUSY);
			err = reg_i2c_status & FLD_I2C_NAK;
			if(err)
				break;
		}
	}
	reg_i2c_ctrl = FLD_I2C_CMD_STOP;
	while (reg_i2c_status & FLD_I2C_CMD_BUSY);
	irq_restore(r);
	return err;
}
