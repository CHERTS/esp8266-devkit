require("ds3231")

-- ESP-01 GPIO Mapping
gpio0, gpio2 = 3, 4

ds3231.init(gpio0, gpio2)

second, minute, hour, day, date, month, year = ds3231.getTime();

-- Get current time
print(string.format("Time & Date: %s:%s:%s %s/%s/%s", hour, minute, second, date, month, year))

-- Don't forget to release it after use
ds3231 = nil
package.loaded["ds3231"]=nil
