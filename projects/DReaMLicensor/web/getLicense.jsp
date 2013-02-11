<%@ include file="includes.jsp" %>
<%@ page import="java.io.BufferedOutputStream" %>
<%@ page import="java.io.InputStream" %>

<%
DatabaseHelper.init(application);
License lic = null;
try {
    lic = License.getLicenseFromDatabase(request);
    InputStream licStream = lic.getLicense();
    if (licStream == null) {
        out.println("Valid license not available");
        return;
    }
    response.setContentType(lic.getMime());
    BufferedOutputStream outS = new BufferedOutputStream(response.getOutputStream());
    byte by[] = new byte[32768];
    int index = licStream.read(by, 0, 32768);
    while (index != -1) {
        outS.write(by, 0, index);
        index = licStream.read(by, 0, 32768);
    }
    outS.flush();
       
} catch(LicenseServerException e) {
    out.println(e.getMessage());
    return;	
}
%>