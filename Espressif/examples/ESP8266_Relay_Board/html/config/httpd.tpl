<html><head><title>HTTPD settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">
<script type="text/javascript">

window.onload=function(e) {
	sc('httpd-auth',%httpd-auth%);
	di();
};

function sc(l,v) {
document.getElementById(l).checked=v;}

function sd(l,v) {
if(document.getElementById(l)) document.getElementById(l).disabled=v;}

function di(){
var v=true;
if (document.getElementById('httpd-auth').checked) v=false;
sd('httpd-user',v);
sd('httpd-pass',v);
}

</script>

</head>
<body>
<div id="main">
<p>
<b>HTTPD Settings</b>
</p>
<form name="httpdform" action="httpd.cgi" method="post">

<table>
<tr><td>HTTPD Port:</td><td><input type="text" name="httpd-port" id="httpd-port" value="%httpd-port%"/>     </td></tr>
<tr><td>HTTP Basic Auth enabled?:</td><td><input type="checkbox" name="httpd-auth" id="httpd-auth" onclick="di();"/></td></tr>
<tr><td>User:</td><td><input type="text" name="httpd-user" id="httpd-user" value="%httpd-user%"/>     </td></tr>
<tr><td>Password:</td><td><input type="text" name="httpd-pass" id="httpd-pass" value="%httpd-pass%"/>     </td></tr>
<tr><td><button type="button" onClick="parent.location='/'">Back</button><input type="submit" name="save" value="Save"></td></tr>
</table>
</form>

</body>
</html>