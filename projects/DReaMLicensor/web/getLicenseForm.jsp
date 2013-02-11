<html>
<head>
<title>Get a License</title>
</head>
<body>
<%@ include file="banner.jsp" %>
Get a License from the license Server Database using this form:
<form action="getLicense.jsp">
<table border="0" cellspacing="0" cellpadding="0">
<tr><td align="left" width="30%">
User Id
</td><td align="leftt">
<input type="text" name="userId"/>
</td></tr>
<tr><td align="left" width="30%">
Shop Id
</td><td align="left">
<input type="text" name="shopId"/>
</td></tr>
<tr><td align="left" width="30%">
Content Id
</td><td align="left">
<input type="text" name="contentId"/>
</td></tr>
<tr><td colspan="2">
<input type="submit" value="Get" />
</td>
</tr>
</table>
</form>
</body>
</html>