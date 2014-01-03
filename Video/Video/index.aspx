<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="index.aspx.cs" Inherits="Video.index" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
    <head runat="server">
        <title>视频搜索系统</title>
        <meta http-equiv="Content-Type" content="test/html;charset=utf-8" />
        <link rel="Stylesheet" type="text/css" href="style/index.css" />
        <link rel="Stylesheet" type="text/css" href="style/jQAllRangeSliders_css/dev.css" />
        <link rel="stylesheet" type="text/css" href="style/jquery-ui/css/smoothness/jquery-ui-1.8.10.custom.css" />  
        <link rel="Stylesheet" type="text/css" href="style/jquery-hcheckbox/css.css" />
        <script type="text/javascript" src="style/jquery-1.7.1.min.js"></script>
        <script type="text/javascript" src="style/jquery-ui/js/jquery-ui-1.8.16.custom.min.js"></script>
        <script type="text/javascript" src="style/jQAllRangeSliders-min.js"></script>
        <script type="text/javascript" src="style/jquery-hcheckbox/jquery-hcheckbox.js"></script>
        <link rel="stylesheet" href="style/fancybox/source/jquery.fancybox.css" type="text/css" media="screen" />
        <script type="text/javascript" src="style/fancybox/source/jquery.fancybox.pack.js"></script>
        <script type="text/javascript" src="style/index.js"></script>
    </head>
    <body>
    <form id="form1" runat="server">
        <h1><a href="index.aspx" id="header"></a></h1>
        <div id="content">
            <div id="basicSearch">
                <h2>基本选项</h2>
                <div class="searchListTable">
                    <div class="searchListTableItem clear">
                        <div class="sImage left">
                            <div id="img1"></div>
                        </div>
                        <div id="selectTime">
                            <div class="sepTime1"></div>
                            <div id="sDate"></div>
                            <div class="sepTime2"></div>
                            <div id="sTime"></div>
                        </div>
                    </div>
                    <div class="searchListTableItem clear">
                        <div class="sImage left">
                            <div id="img2"></div>
                        </div>
                        <div id="selectPos">
                            <div id="posCheckList" class="clear">
                                <input type="checkbox" value='1' /><label>摄像头1</label>
                                <input type="checkbox" value='2' /><label>摄像头2</label>
                                <input type="checkbox" value='3' /><label>摄像头3</label>
                                <input type="checkbox" value='4' /><label>摄像头4</label>
                                <input type="checkbox" value='5' /><label>摄像头5</label>
                                <input type="checkbox" value='6' /><label>摄像头6</label>
                                <input type="checkbox" value='7' /><label>摄像头7</label>
                                <input type="checkbox" value='8' /><label>摄像头8</label>
                                <input type="checkbox" value='9' /><label>摄像头9</label>
                                <input type="checkbox" value='10' /><label>摄像头10</label>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
            <div id="extendSearch">
                <h2>扩展选项</h2>
                <div class="searchListTable">
                    <div class="searchListTableItem clear">
                        <div class="sImage left">
                            <div id="img3"></div>
                        </div>
                        <div id="selectPeople">
                            <div id="selectCheckList" class="clear">
                                <input type="checkbox" value='1' /><label>人</label>
                                <input type="checkbox" value='2' /><label>车</label>
                                <input type="checkbox" value='3' /><label>其他</label>
                            </div> 
                        </div>          
                    </div>
                    <div class="searchListTableItem clear">
                        <div class="sImage left">
                            <div id="img4"></div>
                        </div>
                        <div id="selectSingle">
                            <div id="peopleSingle">
                                <div class="radioTitle">选择颜色：</div>
                                <div id="peoColorCheck" class="clear">
                                    <input type="checkbox" value="black" /><label>黑</label>
                                    <input type="checkbox" value="white" /><label>白</label>
                                    <input type="checkbox" value="grey" /><label>灰</label>
                                    <input type="checkbox" value="red" /><label>红</label>
                                    <input type="checkbox" value="yellow" /><label>黄</label>
                                    <input type="checkbox" value="blue" /><label>蓝</label>
                                    <input type="checkbox" value="green" /><label>绿</label>
                                </div>
                                <div class="radioTitle">上传图片：</div>
                                <div id="peopleUploadImage" class="uploadImage">
                                    <a href="uploadimage.aspx?command=people" id="peopleUploadImageLink" class="various fancybox.iframe">上传图片</a>
                                    <img id="peopleImage" src="" alt="" />
                                </div>
                            </div> 
                        </div>          
                    </div>                    
                    <div class="searchListTableItem clear">
                        <div class="sImage left">
                            <div id="img5"></div>
                        </div>
                        <div id="selectMovement">
                            <div id="peopleMovement">
                                <div class="radioTitle">行为：</div>
                            </div>
                        </div>     
                    </div>
                </div>
            </div>
            <div id="selectSum">
                <div><span>日期时间：</span> <span id="timeResult_0"></span>&nbsp;<span id="timeResult_1"></span> | <span id="timeResult_2"></span>&nbsp;<span id="timeResult_3"></span></div>
                <div><span>选择地点：</span> <span id="posResult"></span></div>
                <div><span>物体类型：</span> <span id="typeResult"></span></div>
                <div><span>个体特征：</span>
                    <span id="singleResult_0">
                        <span id="singleResult_0_1"></span>
                        <span id="singleResult_0_2"></span>
                        <span id="singleResult_0_3"></span>
                    </span> 
                </div>
                <div><span>行为特征：</span></div>
                <div class="button_con">
                    <asp:LinkButton ID="button_submit" runat="server"></asp:LinkButton>
                </div>
                <div id="copy">&copy;WHUT 2012</div>
            </div>
        </div>
    </form>
    </body>
</html>
