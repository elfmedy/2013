using System;
using System.IO;
using System.Collections.Generic;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

namespace Video
{
    public partial class index : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {
            /*
            if (!Page.IsPostBack)
            {
                //删除一天内没有使用的图片
                string[] strFiles = Directory.GetFiles("Video\\tmpimg\\");
                foreach (string strFile in strFiles)
                {
                    FileInfo fi = new FileInfo(strFile);
                    if (fi.LastAccessTime < DateTime.Now.AddDays(-1))
                        fi.Delete();
                }
                //删除一天之内没有使用的tmpupload之内的图像
                string[] strUploadFiles = Directory.GetFiles("Video\\tmpupload\\");
                foreach (string strFile in strUploadFiles)
                {
                    FileInfo fi = new FileInfo(strFile);
                    if (fi.LastAccessTime < DateTime.Now.AddDays(-1))
                        fi.Delete();
                }
            }
            */
        }

    }
}
