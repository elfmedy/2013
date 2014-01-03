using System;
using System.Configuration;
using System.IO;
using System.Text;
using System.Data;
using System.Data.Odbc;
using System.Collections.Generic;
using System.Web;
using System.Web.UI;
using System.Web.UI.HtmlControls;
using System.Web.UI.WebControls;
using System.Runtime.InteropServices;

namespace Video
{
    public partial class result : System.Web.UI.Page
    {
        /// <summary>
        /// 点击页码的时候不是post事件
        /// </summary>
        protected DataTable g_dt;

        [DllImport("SearchPic.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Auto)]
        public static extern void SearchPic([MarshalAs(UnmanagedType.LPStr)] String picFile, [MarshalAs(UnmanagedType.LPStr)] String firstDataFile, [MarshalAs(UnmanagedType.LPArray,SizeParamIndex=1)] int[] idArr, ref int hashValue, ref int idCount);
        [DllImport("SearchPic.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Auto)]
        public static extern void SearchVal([MarshalAs(UnmanagedType.LPStr)] String dataFilePath, int fileSerialNum, int hashValue, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] int[] idArr);


        protected void Page_Load(object sender, EventArgs e)
        {
            //ts,te,ps,cp,cc,co
            string ts = HttpUtility.HtmlDecode(Request.QueryString["ts"].ToString());  //开始时间
            string te = HttpUtility.HtmlDecode(Request.QueryString["te"].ToString());  //结束时间
            string ps = HttpUtility.HtmlDecode(Request.QueryString["ps"].ToString());  //地点
            string tp = HttpUtility.HtmlDecode(Request.QueryString["tp"].ToString());  //类型
            string cp = HttpUtility.HtmlDecode(Request.QueryString["cp"].ToString());  //基本颜色
            string pp = HttpUtility.HtmlDecode(Request.QueryString["pp"].ToString());  //图片 (图片名称)

            string condition = "";
            string state = "";

            //时间 201204140000
            string startTime = ConvertDate(ts);
            string endTime = ConvertDate(te);
            condition += " ([endtime] > '" + startTime + "') AND ([starttime] < '" + endTime + "')";

            //地点 2+3+4+5+10, all表示全部选中了
            if (ps == "")
            {
                state += "地点为空;";
            }
            else
            {
                if (ps == "all") condition += "";
                else
                {
                    string[] posArr = ps.Split(' ');
                    condition += " AND ([cameraid] =" + posArr[0];
                    for (int i = 1; i < posArr.Length; i++)
                    {
                        condition += " OR [cameraid] =" + posArr[i];
                    }
                    condition += ")";
                }

            }

            //类型 1+2+3, all表示全选中
            if (tp == "")
            {
                state += "物体类型为空;";
            }
            else
            {
                if (tp == "all") condition += "";
                else
                {
                    string[] ptArr = tp.Split(' ');
                    condition += " AND ([objtype] =" + ptArr[0];
                    for (int i = 1; i < ptArr.Length; i++)
                    {
                        condition += " OR [objtype] =" + ptArr[i];
                    }
                    condition += ")";
                }
            }

            //颜色 black+white+grey+red, all表示全选中
            if (cp == "")
            {
                state += "颜色为空;";
            }
            else
            {
                if (cp == "all") condition += "";
                else
                {
                    uint colorPeople = ReturnColor(cp);
                    condition += " AND ([basiccolor] & " + colorPeople + "!= 0)";
                }
            }
            //图片
            int extCount = 0;
            string[] extCondition = new string[10];
            for (int i = 0; i < 10; i++) extCondition[i] = "";
            if (pp != "")
            {
                string pic = Server.MapPath("~/tmpupload/") + pp;
                string dat = "VideoSYS\\pictureFeat\\feat_001";
                string datPath = "VideoSYS\\pictureFeat\\";

                string tmpRootDir = Server.MapPath(System.Web.HttpContext.Current.Request.ApplicationPath.ToString());
                int tmpPos = tmpRootDir.LastIndexOf("\\") - 5;
                tmpRootDir = tmpRootDir.Substring(0, tmpPos);

                dat = tmpRootDir + dat.Replace(@"/", @"\");
                datPath = tmpRootDir + datPath.Replace(@"/", @"\");

                int[] idArr = new int[100];
                int hashValue = 0;
                int idCount = 0;
                SearchPic(pic, dat, idArr, ref hashValue, ref idCount);
                if (idCount > 0)
                {
                    condition += " AND ([id]=" + idArr[0];
                    for (int i = 1; i < idCount; i++)
                    {
                        condition += " OR [id]=" + idArr[i];
                    }
                    condition += ")";
                    if (idCount > 100)  //如果大于100的话就得搜索后面的文件
                    {
                        extCount = (idCount - 1) / 100;
                        for (int i = 0; i < extCount - 1; i++)
                        {
                            SearchVal(datPath, extCount + 1, hashValue, idArr);     //
                            extCondition[i] += " AND ([id]=" + idArr[0];
                            for (int j = 1; j < 100; j++)
                            {
                                extCondition[i] += " OR [id]=" + idArr[i];
                            }
                            extCondition[i] += ")";
                        }
                        int offset = idCount % 100;
                        if (offset > 0) offset = 100;
                        extCondition[extCount - 1] += " AND ([id]=" + idArr[0];
                        for (int j = 1; j < offset; j++)
                        {
                            extCondition[extCount - 1] += " OR [id]=" + idArr[j];
                        }
                        extCondition[extCount - 1] += ")";
                    }
                }
                else
                {
                    condition += "AND ([id]=-1)";
                }
            }

            //最后
            if (state != "")
            {
                error_message.InnerHtml = "";
                error_message.InnerHtml = state + "请重新选择条件后查询";
            }
            else
            {
                g_dt = MyFunction.dataTable("SELECT * FROM [video] WHERE" + condition);
                if (extCount > 0)
                {
                    for (int i = 0; i < extCount; i++)
                    {
                        DataTable tmpTable = MyFunction.dataTable("SELECT * FROM [video] WHERE" + extCondition[i]);
                        g_dt.Merge(tmpTable);
                    }
                }
                if (g_dt.Rows.Count == 0)
                {
                    error_message.InnerHtml = "";
                    error_message.InnerHtml = "没有找到满足查询条件的查询结果";
                }
                else
                {
                    ShowResult();
                }
            }
        }

        /// <summary>
        /// 转换时间格式 201204140000 -> 2012-04-14 00:00
        /// </summary>
        /// <param name="time"></param>
        /// <returns></returns>
        protected string ConvertDate(string time)
        {
            string str = time.Substring(0, 4) + '-' + time.Substring(4, 2) + '-' + time.Substring(6, 2) + ' ' + time.Substring(8, 2) + ':' + time.Substring(10, 2) + ".000";
            return str;
        }

        /// <summary>
        /// 转换时间
        /// </summary>
        protected string ShowTime(object time)
        {
            string outTime = "unknow time";
            if (!Convert.IsDBNull(time))
                outTime = Convert.ToDateTime(time.ToString()).ToString("yyyy-mm-dd hh:mm:ss");
            return outTime;
        }

        /// <summary>
        /// 传入quere的字符串，然后返回颜色的代码
        /////	  灰   白		黑		蓝		绿		黄		红
		////MSB 0  0	0		0		0		0		0		0  
        /// </summary>
        /// <param name="color"></param>
        /// <returns></returns>
        protected uint ReturnColor(string color)
        {
            uint intColor = 0;
            string[] colorArr = color.Split(' ');
            foreach (string singleColor in colorArr)
            {
                if (singleColor == "red")
                    intColor |= 0x01;
                else if (singleColor == "yellow")
                    intColor |= 0x02;
                else if (singleColor == "green")
                    intColor |= 0x04;
                else if (singleColor == "blue")
                    intColor |= 0x08;
                else if (singleColor == "black")
                    intColor |= 0x10;
                else if (singleColor == "white")
                    intColor |= 0x20;
                else if (singleColor == "grey")
                    intColor |= 0x40;
            }
            return intColor;
        }


        //[DllImport("createImage.dll", EntryPoint = "CreateImage", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Auto)]
        [DllImport("createImage.dll", EntryPoint = "CreateImage", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Auto)]
        public static extern void CreateImage(int id, [MarshalAs(UnmanagedType.LPStr)] String idDir, [MarshalAs(UnmanagedType.LPStr)] String outDir);

        protected string sqloverallimageDir = "VideoSYS\\overallImage\\";
        /// <summary>
        /// 利用数据库中的数据生成图片，并且返回图片的地址
        /// </summary>
        /// <param name="time"></param>
        /// <returns></returns>
        protected string ShowPic(object uId)
        {
            string fileName = string.Format("{0:D9}", Convert.ToInt32(uId.ToString().Trim())) + "_img.jpg";

            string tmpRootDir = Server.MapPath(System.Web.HttpContext.Current.Request.ApplicationPath.ToString());
            int tmpPos = tmpRootDir.LastIndexOf("\\") - 5;
            tmpRootDir = tmpRootDir.Substring(0, tmpPos);

            string sqloverallimageDirIn = tmpRootDir + sqloverallimageDir.Replace(@"/", @"\");

            if (!File.Exists(Server.MapPath("~/tmpimg/") + fileName))
            {
                CreateImage(Convert.ToInt32(uId.ToString().Trim()), sqloverallimageDirIn, Server.MapPath("~/tmpimg/"));
            }
            return "tmpimg/" + fileName;
        }

        /// <summary>
        /// 获得地址，相比于ShowPic来说并不生成图片，只是获得图片地址
        /// </summary>
        /// <param name="uId"></param>
        /// <returns></returns>
        protected string ShowPicLess(object uId)
        {
            string fileName = "tmpimg/" + string.Format("{0:D9}", Convert.ToInt32(uId.ToString().Trim())) + "_img.jpg";
            return fileName;
        }


        /// <summary>
        /// 导航
        /// </summary>
        protected void ShowResult()
        {
            int currentPage = Convert.ToInt32(lb_currentPage.Text);    //当前页数
            PagedDataSource ps = new PagedDataSource();
            ps.DataSource = g_dt.DefaultView;
            ps.AllowPaging = true;
            ps.PageSize = 15;
            ps.CurrentPageIndex = currentPage - 1;                          //-分页控件的页数从0开始
            lb_page.Text = ps.PageCount.ToString();                  //总页数

            bottom_nav.Controls.Clear();
            LinkButton prev = new LinkButton();
            prev.ID = "LinkUp";
            prev.Text = "&lt;";
            prev.Click += new EventHandler(this.LinkUp_Click);
            if (currentPage == 1)
                prev.Enabled = false;
            bottom_nav.Controls.Add(prev);
            //currentPage之前(小于9个显示所有前面页面的按钮,大于9个就不完全显示了)
            if (currentPage - 1 <= 9)
            {
                for (int i = 1; i < currentPage; i++)
                {
                    LinkButton select = new LinkButton();
                    select.CssClass = "page";
                    select.Text = i.ToString();
                    select.ID = "SelectPage" + i.ToString();
                    select.Command += new CommandEventHandler(this.SelectPage_Click);
                    select.CommandArgument = i.ToString();
                    bottom_nav.Controls.Add(select);
                }
            }
            else
            {
                for (int i = 1; i < 3; i++)
                {
                    LinkButton select = new LinkButton();
                    select.CssClass = "page";
                    select.Text = i.ToString();
                    select.ID = "SelectPage" + i.ToString();
                    select.Command += new CommandEventHandler(this.SelectPage_Click);
                    select.CommandArgument = i.ToString();
                    bottom_nav.Controls.Add(select);
                }
                HtmlGenericControl div = new HtmlGenericControl();
                div.TagName = "span";
                div.Attributes["class"] = "extend";
                div.InnerHtml = "...";
                bottom_nav.Controls.Add(div);
                for (int i = currentPage - 7; i < currentPage; i++)
                {
                    LinkButton select = new LinkButton();
                    select.CssClass = "page";
                    select.Text = i.ToString();
                    select.ID = "SelectPage" + i.ToString();
                    select.Command += new CommandEventHandler(this.SelectPage_Click);
                    select.CommandArgument = i.ToString();
                    bottom_nav.Controls.Add(select);
                }
            }
            //currentPage
            HtmlGenericControl curDiv = new HtmlGenericControl();
            curDiv.TagName = "span";
            curDiv.Attributes["class"] = "current";
            curDiv.InnerHtml = currentPage.ToString();
            bottom_nav.Controls.Add(curDiv);
            //currentPage之后(小于9个就完全显示，大于9个就不完全显示)
            if (ps.PageCount - currentPage <= 9)
            {
                for (int i = currentPage + 1; i <= ps.PageCount; i++)
                {
                    LinkButton select = new LinkButton();
                    select.CssClass = "page";
                    select.Text = i.ToString();
                    select.ID = "SelectPage" + i.ToString();
                    select.Command += new CommandEventHandler(this.SelectPage_Click);
                    select.CommandArgument = i.ToString();
                    bottom_nav.Controls.Add(select);
                }
            }
            else
            {
                for (int i = currentPage + 1; i <= currentPage + 7; i++)
                {
                    LinkButton select = new LinkButton();
                    select.CssClass = "page";
                    select.Text = i.ToString();
                    select.ID = "SelectPage" + i.ToString();
                    select.Command += new CommandEventHandler(this.SelectPage_Click);
                    select.CommandArgument = i.ToString();
                    bottom_nav.Controls.Add(select);
                }
                HtmlGenericControl div = new HtmlGenericControl();
                div.TagName = "span";
                div.Attributes["class"] = "extend";
                div.InnerHtml = "...";
                bottom_nav.Controls.Add(div);
                for (int i = ps.PageCount - 1; i <= ps.PageCount; i++)
                {
                    LinkButton select = new LinkButton();
                    select.CssClass = "page";
                    select.Text = i.ToString();
                    select.ID = "SelectPage" + i.ToString();
                    select.Command += new CommandEventHandler(this.SelectPage_Click);
                    select.CommandArgument = i.ToString();
                    bottom_nav.Controls.Add(select);
                }
            }
            //最后
            LinkButton next = new LinkButton();
            next.ID = "LinkDown";
            next.Text = "&gt;";
            next.Click += new EventHandler(this.LinkDown_Click);
            if (currentPage == ps.PageCount)
                next.Enabled = false;
            bottom_nav.Controls.Add(next);

            rptResult.DataSource = ps;      //将数据给rptResult
            rptResult.DataBind();
        }


        /// <summary>
        /// 上一页
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void LinkUp_Click(object sender, EventArgs e)
        {
            try
            {
                lb_currentPage.Text = Convert.ToString(Convert.ToInt32(lb_currentPage.Text) - 1);
                ShowResult();
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        /// <summary>
        /// 下一页
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void LinkDown_Click(object sender, EventArgs e)
        {
            try
            {
                lb_currentPage.Text = Convert.ToString(Convert.ToInt32(lb_currentPage.Text) + 1);
                ShowResult();
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }

        /// <summary>
        /// 选择页面
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void SelectPage_Click(object sender, CommandEventArgs e)
        {
            try
            {
                lb_currentPage.Text = e.CommandArgument.ToString();
                ShowResult();
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }
    }

}
