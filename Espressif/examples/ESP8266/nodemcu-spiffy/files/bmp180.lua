-- ***************************************************************************
-- BMP180 module for ESP8266 with nodeMCU
-- BMP085 compatible but not tested
--
-- Written by Javier Yanez
--
-- MIT license, http://opensource.org/licenses/MIT
-- ***************************************************************************

local moduleName = ...
local M = {}
_G[moduleName] = M

local ADDR = 0x77 --BMP180 address
local REG_CALIBRATION = 0xAA
local REG_CONTROL = 0xF4
local REG_RESULT = 0xF6

local COMMAND_TEMPERATURE = 0x2E
local COMMAND_PRESSURE = {0x34, 0x74, 0xB4, 0xF4}

-- calibration coefficients
local AC1, AC2, AC3, AC4, AC5, AC6, B1, B2, MB, MC, MD

-- temperature and pressure
local t,p

local init = false

-- i2c interface ID
local id = 0

-- 16-bit  two's complement
-- value: 16-bit integer
local function twoCompl(value)
 if value > 32767 then value = -(65535 - value + 1)
 end
 return value
end

-- read data register
-- reg_addr: address of the register in BMP180
-- lenght: bytes to read
local function read_reg(reg_addr, length)
  i2c.start(id)
  i2c.address(id, ADDR, i2c.TRANSMITTER)
  i2c.write(id, reg_addr)
  i2c.stop(id)
  i2c.start(id)
  i2c.address(id, ADDR,i2c.RECEIVER)
  c = i2c.read(id, length)
  i2c.stop(id)
  return c
end

-- write data register
-- reg_addr: address of the register in BMP180
-- reg_val: value to write to the register
local function write_reg(reg_addr, reg_val)
  i2c.start(id)
  i2c.address(id, ADDR, i2c.TRANSMITTER)
  i2c.write(id, reg_addr)
  i2c.write(id, reg_val)
  i2c.stop(id)
end

-- initialize module
-- sda: SDA pin
-- scl SCL pin
function M.init(sda, scl)
  i2c.setup(id, sda, scl, i2c.SLOW)
  local calibration = read_reg(REG_CALIBRATION, 22)

  AC1 = twoCompl(string.byte(calibration, 1) * 256 + string.byte(calibration, 2))
  AC2 = twoCompl(string.byte(calibration, 3) * 256 + string.byte(calibration, 4))
  AC3 = twoCompl(string.byte(calibration, 5) * 256 + string.byte(calibration, 6))
  AC4 = string.byte(calibration, 7) * 256 + string.byte(calibration, 8)
  AC5 = string.byte(calibration, 9) * 256 + string.byte(calibration, 10)
  AC6 = string.byte(calibration, 11) * 256 + string.byte(calibration, 12)
  B1 = twoCompl(string.byte(calibration, 13) * 256 + string.byte(calibration, 14))
  B2 = twoCompl(string.byte(calibration, 15) * 256 + string.byte(calibration, 16))
  MB = twoCompl(string.byte(calibration, 17) * 256 + string.byte(calibration, 18))
  MC = twoCompl(string.byte(calibration, 19) * 256 + string.byte(calibration, 20))
  MD = twoCompl(string.byte(calibration, 21) * 256 + string.byte(calibration, 22))

  init = true
end

-- read temperature from BMP180
local function read_temp()
  write_reg(REG_CONTROL, COMMAND_TEMPERATURE)
  tmr.delay(5000)
  local dataT = read_reg(REG_RESULT, 2)
  UT = string.byte(dataT, 1) * 256 + string.byte(dataT, 2)
  local X1 = (UT - AC6) * AC5 / 32768
  local X2 = MC * 2048 / (X1 + MD)
  B5 = X1 + X2
  t = (B5 + 8) / 16
  return(t)
end

-- read pressure from BMP180
-- must be read after read temperature
local function read_pressure(oss)
  write_reg(REG_CONTROL, COMMAND_PRESSURE[oss + 1]);
  tmr.delay(30000);
  local dataP = read_reg(0xF6, 3)
  local UP = string.byte(dataP, 1) * 65536 + string.byte(dataP, 2) * 256 + string.byte(dataP, 1)
  UP = UP / 2 ^ (8 - oss)
  local B6 = B5 - 4000
  local X1 = B2 * (B6 * B6 / 4096) / 2048
  local X2 = AC2 * B6 / 2048
  local X3 = X1 + X2
  local B3 = ((AC1 * 4 + X3) * 2 ^ oss + 2) / 4
  X1 = AC3 * B6 / 8192
  X2 = (B1 * (B6 * B6 / 4096)) / 65536
  X3 = (X1 + X2 + 2) / 4
  local B4 = AC4 * (X3 + 32768) / 32768
  local B7 = (UP - B3) * (50000/2 ^ oss)
  p = (B7 / B4) * 2
  X1 = (p / 256) * (p / 256)
  X1 = (X1 * 3038) / 65536
  X2 = (-7357 * p) / 65536
  p = p +(X1 + X2 + 3791) / 16
  return (p)
end

-- read temperature and pressure from BMP180
-- oss: oversampling setting. 0-3
function M.read(oss)
  if (oss == nil) then
     oss = 0
  end
  if (not init) then
     print("init() must be called before read.")
  else  
     read_temp()
     read_pressure(oss)
  end
end;

-- get temperature
function M.getTemperature()
  return t
end

-- get pressure
function M.getPressure()
  return p
end

return M
