<html><head><title>Relay Settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">
</head>
<body>
<div id="main">
<p>
<b>Relay Settings</b>
</p>
<form name="relayform" action="relay.cgi" method="post">

<table>
<tr><td>Relay latching enabled?:</td><td><input type="checkbox" name="relay-latching-enable" id="relay-latching-enable" %relay-latching-enable% /></td></tr>
<tr><td>Relay 1 name:</td><td><input type="text" name="relay1name" id="relay1name" value="%relay1name%"/>     </td></tr>
<tr><td>Relay 2 name:</td><td><input type="text" name="relay2name" id="relay2name" value="%relay2name%"/>     </td></tr>
<tr><td>Relay 3 name:</td><td><input type="text" name="relay3name" id="relay3name" value="%relay3name%"/>     </td></tr>
</table>
<br/>
<button type="button" onClick="parent.location='/'">Back</button>
<input type="submit" name="save" value="Save">
</p>
</form>

</body>
</html>