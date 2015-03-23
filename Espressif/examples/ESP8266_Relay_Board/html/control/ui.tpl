<!DOCTYPE html>
<html>
  <head>

    <title>Relay Board</title>
    <meta http-equiv="Content-Type" content="text/html">
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
    <meta name="apple-mobile-web-app-capable" content="yes" />
    <meta name="apple-mobile-web-app-status-bar-style" content="black" />
    <link rel="stylesheet" href="http://code.jquery.com/mobile/1.3.0/jquery.mobile-1.3.0.min.css" />
    <script src="http://code.jquery.com/jquery-1.8.2.min.js"></script>
    <script src="http://code.jquery.com/mobile/1.3.0/jquery.mobile-1.3.0.min.js"></script>
    <script src="ui.js"></script>
    <link rel="stylesheet" href="ui.css" />
  </head>

  <body>
  
    <div data-role="page" data-title="Relay Board" id="root">
    
     <div data-theme="d" data-role="header">
       <h1>
         ESP8266 Relay Board
       </h1>
     </div><br>
     
     <div data-role="content">

        <div class="content-primary">
          <div class="s-title"><center>Relay Control</center></div>
          <ul data-role="listview" data-inset="true" >
            <li>
                    <label for="relay1"><b>%relay1name%</b></label>
                    <select name="relay1" id="relay1" data-mini="false" data-role="slider">
                      <option value="0">Off</option>
                      <option value="1">On</option>
                    </select>
            </li>
            <li>
                    <label for="relay2"><b>%relay2name%</b></label>
                    <select name="relay2" id="relay2" data-mini="false" data-role="slider">
                      <option value="0">Off</option>
                      <option value="1">On</option>
                    </select>
            </li>
            <li>
                    <label for="relay3"><b>%relay3name%</b></label>
                    <select name="relay3" id="relay3" data-mini="false" data-role="slider">
                      <option value="0">Off</option>
                      <option value="1">On</option>
                    </select>
            </li>
          </ul>
        </div><br>
        
        <div class="content-secondary" %sensor-ds18b20-enable% >
          <div class="s-title"><center>DS18B20 temperature sensor</center></div>
          <ul data-role="listview" data-inset="true" >
            <li>
                    <label><b>DS18B20</b></label>
                    <span class="inputvalue" name="DS18B20temperature" id="DS18B20temperature">N/A</span>
            </li>
        </div>
     
        <div class="content-secondary" %sensor-dht22-enable% >
          <div class="s-title"><center>DHT22 sensor</center></div>
          <ul data-role="listview" data-inset="true" >
            <li>
                    <label><b>DHT22 Temperature</b></label>
                    <span class="inputvalue" name="DHT22temperature" id="DHT22temperature">N/A</span>
            </li>
            <li>
                    <label><b>DHT22 Humidity</b></label>
                    <span class="inputvalue" name="DHT22humidity" id="DHT22humidity">N/A</span>
            </li>
        </div>
   
     </div><br>
      
     <div data-theme="d" data-role="footer">
 
     </div>
      
    </div>

  </body>
  
</html>
