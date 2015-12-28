<html><head><title>NTP settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">
<script type="text/javascript">

window.onload=function(e) {
	sc('ntp-enable',%ntp-enable%);
	di();
};

function sc(l,v) {
document.getElementById(l).checked=v;}

function sd(l,v) {
if(document.getElementById(l)) document.getElementById(l).disabled=v;}

function di(){
var v=true;
if (document.getElementById('ntp-enable').checked) v=false;
sd('ntp-tz',v);
}

</script>

</head>
<body>
<div id="main">
<p>
<b>NTP Settings</b>
</p>
<form name="ntpform" action="ntp.cgi" method="post">

<table>
<tr><td>NTP client enabled?:</td><td><input type="checkbox" name="ntp-enable" id="ntp-enable" onclick="di();"/></td></tr>
<tr><td>GMT offset:</td><td><input type="text" name="ntp-tz" id="ntp-tz" value="%ntp-tz%"/>     </td></tr>
<tr><td><button type="button" onClick="parent.location='/'">Back</button><input type="submit" name="save" value="Save"></td></tr>
</table>

</form>

<p>%NTP%</p>
</body>
</html>