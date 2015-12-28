<html><head><title>Broadcaset Daemon Settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">
<script type="text/javascript">

window.onload=function(e) {
	sc('broadcastd-enable',%broadcastd-enable%);
	di();
};

function sc(l,v) {
document.getElementById(l).checked=v;}

function sd(l,v) {
if(document.getElementById(l)) document.getElementById(l).disabled=v;}

function di(){
var v=true;
if (document.getElementById('broadcastd-enable').checked) v=false;
sd('broadcastd-host',v);
sd('broadcastd-port',v);
sd('broadcastd-URL',v);
sd('broadcastd-thingspeak-channel',v);
sd('broadcastd-ro-apikey',v);
}

</script>

</head>
<body>
<div id="main">
<p>
<b>Broadcast Daemon Settings</b>
</p>
<form name="broadcasetdform" action="broadcastd.cgi" method="post">

<table>
<tr><td>HTTP GET broadcast enabled?:</td><td><input type="checkbox" name="broadcastd-enable" id="broadcastd-enable" onclick="di();"/></td></tr>
<tr><td>Site:</td><td><input type="text" name="broadcastd-host" id="broadcastd-host" value="%broadcastd-host%"/>     </td></tr>
<tr><td>Port:</td><td><input type="text" name="broadcastd-port" id="broadcastd-port" value="%broadcastd-port%"/>     </td></tr>
<tr><td>URL:</td><td><input type="text" name="broadcastd-URL" id="broadcastd-URL" value="%broadcastd-URL%"/>     </td></tr>
<tr><td>ThingSpeak channel:</td><td><input type="text" name="broadcastd-thingspeak-channel" id="broadcastd-thingspeak-channel" value="%broadcastd-thingspeak-channel%"/>     </td></tr>
<tr><td>ThingSpeak RO API key:</td><td><input type="text" name="broadcastd-ro-apikey" id="broadcastd-ro-apikey" value="%broadcastd-ro-apikey%"/>     </td></tr>
<tr><td><button type="button" onClick="parent.location='/'">Back</button><input type="submit" name="save" value="Save"></td></tr>
</table>
</form>

</body>
</html>