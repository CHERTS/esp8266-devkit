<html><head><title>WiFi connection</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="style.css">
<script type="text/javascript" src="140medley.min.js"></script>
<script type="text/javascript">

var xhr=j();
var currAp="%currSsid%";

function createInputForAp(ap) {
	if (ap.essid=="" && ap.rssi==0) return;
	var div=document.createElement("div");
	div.id="apdiv";
	var rssi=document.createElement("div");
	var rssiVal=-Math.floor(ap.rssi/51)*32;
	rssi.className="icon";
	rssi.style.backgroundPosition="0px "+rssiVal+"px";
	var encrypt=document.createElement("div");
	var encVal="-64"; //assume wpa/wpa2
	if (ap.enc=="0") encVal="0"; //open
	if (ap.enc=="1") encVal="-32"; //wep
	encrypt.className="icon";
	encrypt.style.backgroundPosition="-32px "+encVal+"px";
	var input=document.createElement("input");
	input.type="radio";
	input.name="essid";
	input.value=ap.essid;
	if (currAp==ap.essid) input.checked="1";
	input.id="opt-"+ap.essid;
	var label=document.createElement("label");
	label.htmlFor="opt-"+ap.essid;
	label.textContent=ap.essid;
	div.appendChild(input);
	div.appendChild(rssi);
	div.appendChild(encrypt);
	div.appendChild(label);
	return div;
}

function getSelectedEssid() {
	var e=document.forms.wifiform.elements;
	for (var i=0; i<e.length; i++) {
		if (e[i].type=="radio" && e[i].checked) return e[i].value;
	}
	return currAp;
}


function scanAPs() {
	xhr.open("GET", "wifiscan.cgi");
	xhr.onreadystatechange=function() {
		if (xhr.readyState==4 && xhr.status>=200 && xhr.status<300) {
			var data=JSON.parse(xhr.responseText);
			currAp=getSelectedEssid();
			if (data.result.inProgress=="0" && data.result.APs.length>1) {
				$("#aps").innerHTML="";
				for (var i=0; i<data.result.APs.length; i++) {
					if (data.result.APs[i].essid=="" && data.result.APs[i].rssi==0) continue;
					$("#aps").appendChild(createInputForAp(data.result.APs[i]));
				}
				window.setTimeout(scanAPs, 20000);
			} else {
				window.setTimeout(scanAPs, 1000);
			}
		}
	}
	xhr.send();
}


window.onload=function(e) {
	di();
	scanAPs();
};


function sd(l,v) {
if(document.getElementById(l)) document.getElementById(l).disabled=v;}

function di(){
var v=true;
if (document.getElementById('sta-mode').value == "static") v=false;
sd('sta-ip',v);
sd('sta-mask',v);
sd('sta-gw',v);}

</script>
</head>
<body>
<div id="main">
<p>
<b>WiFi Settings</b>
</p>
<p>
Current WiFi mode: %WiFiMode%
</p>
<p>
Note: %WiFiapwarn%
</p>
<form name="wifiform" action="connect.cgi" method="post">
<p>
To connect to a WiFi network, please select one of the detected networks...<br>
<div id="aps">Scanning...</div>
<br>

<p>
Current IP: %IPAddress%
</p>

<table>
<tr><td>WiFi password:</td><td><input type="text" name="passwd" value="%WiFiPasswd%"> </td></tr>
<tr><td>Mode:</td><td>

<select name="sta-mode" id="sta-mode"  onchange='di();' >
  <option value="dhcp" %selecteddhcp% >DHCP</option>
  <option value="static" %selectedstatic% >Static IP</option>
</select>

</td></tr>
<tr><td>IP address:</td><td><input type="text" name="sta-ip" id="sta-ip" value="%sta-ip%"/>     </td></tr>
<tr><td>IP mask: </td><td><input type="text" name="sta-mask" id="sta-mask" value="%sta-mask%"/> </td></tr>
<tr><td>GW address:</td><td><input type="text" name="sta-gw" id="sta-gw" value="%sta-gw%"/>     </td></tr>
</table>
<br/>
<button type="button" onClick="parent.location='/'">Back</button>
<input type="submit" name="connect" value="Connect!">
</p>
</form>

</body>
</html>
