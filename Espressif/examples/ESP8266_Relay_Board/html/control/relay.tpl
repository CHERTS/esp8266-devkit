<html><head><title>Relay control</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">
</head>
<body>
<div id="main">
<h1>Relay control page</h1>
<p>
%relay1name% is now %relay1%. You can change that using the buttons below.
</p>
<form method="get" action="relay.cgi">
<input type="submit" name="relay1" value="1">
<input type="submit" name="relay1" value="0">
</form>
<p>
%relay2name% is now %relay2%. You can change that using the buttons below.
</p>
<form method="get" action="relay.cgi">
<input type="submit" name="relay2" value="1">
<input type="submit" name="relay2" value="0">
</form>
<p>
%relay3name% is now %relay3%. You can change that using the buttons below.
</p>
<form method="get" action="relay.cgi">
<input type="submit" name="relay3" value="1">
<input type="submit" name="relay3" value="0">
</form>

<button onclick="location.href = '/';" class="float-left submit-button" >Back</button>
</div>
</body></html>
