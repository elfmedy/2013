<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="showvideo.aspx.cs" Inherits="Video.showvideo" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
    <title>结果</title>
    <style type="text/css">
        html,body{margin:0;padding:0;}
        #imageToShow{width:640px;height:480px;}
    </style>
</head>
<body>
    <form id="form1" runat="server">
    <div>
        <asp:Image runat="server" ID="imageToShow" />
    </div>
    </form>
</body>
</html>
