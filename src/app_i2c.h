#pragma once

void init_i2c(void);
unsigned char test_i2c_device(unsigned char address);
int send_i2c(unsigned char i2c_addr, unsigned char * dataBuf, size_t dataLen);
