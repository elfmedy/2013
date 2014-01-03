<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="result.aspx.cs" Inherits="Video.result" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
    <head runat="server">
        <title>检索结果</title>
        <meta http-equiv="Content-Type" content="test/html;charset=utf-8" />
        <link rel="Stylesheet" type="text/css" href="style/result.css" />
        <link rel="Stylesheet" type="text/css" href="style/page_navi/pagenavi-css.css" />
        <script type="text/javascript" src="style/jquery-1.7.1.min.js"></script>
        <!-- Add fancyBox -->
        <link rel="stylesheet" href="style/fancybox/source/jquery.fancybox.css" type="text/css" media="screen" />
        <script type="text/javascript" src="style/fancybox/source/jquery.fancybox.pack.js"></script>
        <!-- Optionally add button and/or thumbnail helpers -->
        <link rel="stylesheet" href="/fancybox/source/helpers/jquery.fancybox-buttons.css?v=2.0.5" type="text/css" media="screen" />
        <script type="text/javascript" src="/fancybox/source/helpers/jquery.fancybox-buttons.js?v=2.0.5"></script>
        <link rel="stylesheet" href="/fancybox/source/helpers/jquery.fancybox-buttons.css?v=2.0.5" type="text/css" media="screen" />
        <script type="text/javascript" src="/fancybox/source/helpers/jquery.fancybox-buttons.js?v=2.0.5"></script>
        <script type="text/javascript">
            $(document).ready(function() {
                $(".various").fancybox({
                    openEffect: 'none',
                    closeEffect: 'none',
                    nextEffect : 'none',
                    prevEffect : 'none',
                    loop: false,
                    width : '640px',
                    helpers: {
                        title: { type: 'float' }
                    }
                });
            });
        </script>
    </head>
    <body>
    <form id="form2" runat="server">
        <h1><a href="index.aspx" id="header"></a></h1>
        <div id="content">
            <h2>检索结果：</h2>
            <div id="resultContent">
                <ul class="resultList clear"> 
                    <asp:Repeater ID="rptResult" runat="server">
                    <ItemTemplate>
                        <li>
                            <a class="various fancybox.iframe" rel="gallery" href="showvideo.aspx?uid=<%# Eval("id").ToString().Trim() %>" title="第<%# Container.ItemIndex + 1 %>项">
                                <img src="<%# ShowPic(Eval("id")) %>" alt="pic" />
                                <span>日期时间：<%# ShowTime(Eval("starttime")) %> <br />摄像头号：<%# Eval("cameraid").ToString().Trim() %></span>
                            </a>
                        </li>
                    </ItemTemplate>
                    </asp:Repeater>
                    <!--
                    <li>
                        <a class="fancybox various" data-fancybox-group="gallery" href="#hide">
                            <img src="a.jpg" />
                            <span>日期时间：2012-12-1 22:26 <br />摄像头号：12</span>
                        </a>
                    </li> 
                    -->
                </ul>
            </div>
        </div>
        
	    <div class="light">
		    <div class="wp-pagenavi" runat="server" id="bottom_nav">
		    <!--
		        <a class="previouspostslink" href="">&lt;&lt;<</a>
			    <span class="current">1</span>
			    <a class="page" href="">2</a>
			    <a class="page" href="">3</a>
			    <a class="page" href="">4</a>
			    <span class="extend">...</span>
			    <a class="page" href="">6</a>
			    <a class="page" href="">7</a>
			    <a class="page" href="">8</a>
			    <a class="nextpostslink" href="">&gt;&gt;</a>
			-->
	        </div>
	        <div runat="server" id="error_message"></div>
	        <div class="hide">
			    <span class="pages">page <asp:Label ID="lb_currentPage" runat="server" Text="1"></asp:Label> of <asp:Label ID="lb_page" runat="server" Text="0"></asp:Label></span>
			</div>
	    </div>
	    <div id="copy">&copy;WHUT 2012
        </div>
    </form>
    </body>
</html>