-- ******************************************************
-- LM92 module for ESP8266 with nodeMCU
--
-- Written by Levente Tamas <levente.tamas@navicron.com>
--
-- GNU LGPL, see https://www.gnu.org/copyleft/lesser.html
-- ******************************************************

-- Module Bits
local moduleName = ...
local M = {}
_G[moduleName] = M

-- Default ID
local id = 0

-- Local vars
local address = 0

-- read regs for len number of bytes
-- return table with data
local function read_reg(reg_addr, len)
	local ret={}
	local c
	local x
	i2c.start(id)
	i2c.address(id, address ,i2c.TRANSMITTER)
	i2c.write(id,reg_addr)
	i2c.stop(id)
	i2c.start(id)
	i2c.address(id, address,i2c.RECEIVER)
	c=i2c.read(id,len)
	for x=1,len,1 do
		tc=string.byte(c,x)
		table.insert(ret,tc)
	end
	i2c.stop(id)
	return ret
end 

--write reg with data table
local function write_reg(reg_addr, data)
	i2c.start(id)
	i2c.address(id, address, i2c.TRANSMITTER)
	i2c.write(id, reg_addr)
	i2c.write(id, data)
	i2c.stop(id)
end 

-- initialize i2c
-- d: sda
-- c: scl
-- a: i2c addr 0x48|A1<<1|A0 (A0-A1: chip pins)
function M.init(d,c,a)
if (d ~= nil) and (c ~= nil) and (d >= 0) and (d <= 11) and (c >= 0) and ( c <= 11) and (d ~= l) and (a ~= nil) and (a >= 0x48) and (a <= 0x4b ) then
		sda = d
		scl = c 
		address = a
		i2c.start(id)
		res = i2c.address(id, address, i2c.TRANSMITTER) --verify that the address is valid
		i2c.stop(id)
		if (res == false) then
			print("device not found")
			return nil
		end
		else 
        print("i2c configuration failed") return nil
      end
	i2c.setup(id,sda,scl,i2c.SLOW)
end

-- Return the temperature data
function M.getTemperature()
	local temperature
	local tmp=read_reg(0x00,2) --read 2 bytes from the temperature register
	temperature=bit.rshift(tmp[1]*256+tmp[2],3) --lower 3 bits are status bits
	if (temperature>=0x1000) then
		temperature= temperature-0x2000 --convert the two's complement
	end
	return temperature * 0.0625
end

-- Put the LM92 into shutdown mode
function M.shutdown()
	write_reg(0x01,0x01)
end

-- Bring the LM92 out of shutdown mode
function M.wakeup()
	write_reg(0x01,0x00)
end


return M