<html><head><title>Sensor settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">

</head>
<body>
<div id="main">
<p>
<b>Sensor Settings</b>
</p>
<form name="sensorform" action="sensor.cgi" method="post">

<table>
<tr><td>DS18B20 enabled?:</td><td><input type="checkbox" name="sensor-ds18b20-enable" id="sensor-ds18b20-enable" %sensor-ds18b20-enable% /></td></tr>
<tr><td>DHT22 enabled?:</td><td><input type="checkbox" name="sensor-dht22-enable" id="sensor-dht22-enable" %sensor-dht22-enable% /></td></tr>
<tr><td>DHT22 humidity as thermostat input?:</td><td><input type="checkbox" name="sensor-dht22-humi-thermostat" id="sensor-dht22-humi-thermostat" %sensor-dht22-humi-thermostat% /></td></tr>
</table>
<br/>
<button type="button" onClick="parent.location='/'">Back</button>
<input type="submit" name="save" value="Save">
</p>
</form>

</body>
</html>