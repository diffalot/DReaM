<%@ include file="includes.jsp" %>
<html>
<head>
<title>Added License</title>
</head>
<body>
<%@ include file="banner.jsp" %>
<%
DatabaseHelper.init(application);
try {
    License lic = new License(request);
    lic.addToDatabase();
} catch(LicenseServerException e) {
    out.println(e.getMessage());
    return;
}
%>
Successfully added the license to the license server.
</body>
</html>
