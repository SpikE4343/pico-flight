
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "math_util.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "data_vars.h"

#include "camera.h"
#include "ov5642_regs.h"


// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_cam_sda_pin, 12, 
  "cam.i2c.pins.sda",
  "Camera control i2c sda pin",
  u8, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_cam_scl_pin, 13, 
  "cam.i2c.pins.scl",
  "Camera control i2c scl pin",
  u8, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_cam_baud, 100000, 
  "cam.bauds",
  "Camera control i2c baud",
  u32, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------
DEF_DATA_VAR(tdv_cam_i2c_dev, 0, 
  "cam.i2c.dev",
  "Camera control i2c device id",
  b8, Tdm_RW | Tdm_config);

// ---------------------------------------------------------------

i2c_inst_t* i2c_dev;
uint8_t cam_addr = 0x3c;

// ---------------------------------------------------------------
uint8_t read_reg(uint16_t reg, uint8_t* data)
{ 
  uint8_t write[] = { reg >> 8, reg & 0x00FF};
  if(i2c_write_blocking(i2c_dev, cam_addr, write, sizeof(write), true ) < 0)
    return -1;
  return i2c_read_blocking(i2c_dev, cam_addr, data, 1, false);
}

// ---------------------------------------------------------------
uint8_t write_reg(uint16_t reg, uint8_t data)
{
  uint8_t write[] = { reg >> 8, reg & 0x00FF, data};
  i2c_write_blocking(i2c_dev, cam_addr, write, sizeof(write), false );
}

// ---------------------------------------------------------------
uint8_t write_all_reg(sensor_reg_t* data, int len)
{
  for(int i=0; i < len; ++i)
  {
    write_reg(data[i].reg, data[i].val);
  }
}

//0x3008
// ---------------------------------------------------------------
void camera_init()
{
  i2c_dev = tdv_cam_i2c_dev.v.b8 ? i2c1 : i2c0;
  i2c_init(i2c_dev, tdv_cam_baud.v.u32);

  gpio_set_function(tdv_cam_sda_pin.v.u8, GPIO_FUNC_I2C);
  gpio_set_function(tdv_cam_scl_pin.v.u8, GPIO_FUNC_I2C);
  gpio_pull_up(tdv_cam_sda_pin.v.u8);
  gpio_pull_up(tdv_cam_scl_pin.v.u8);

  // reset
  write_reg(0x3008, 0x80);

  sleep_ms(100);

  uint8_t id[] = {0,0};

  // read id
  read_reg(0x300A, id);
  read_reg(0x300B, id+1);

  if(id[0] != 0x56 || id[1] != 0x42)
    return;

  write_all_reg(OV5642_720P_Video_setting, sizeof(OV5642_720P_Video_setting)/sizeof(sensor_reg_t));
}
