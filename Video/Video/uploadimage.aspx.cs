using System;
using System.Collections.Generic;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Runtime.InteropServices;

namespace Video
{
    public partial class uploadimage : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {
            if (!IsPostBack)
            {
                string command = Request["command"].ToString();
                imageUploadButton.CommandName = command;
                contentTitle.InnerText = command;   
                siteTitle.InnerHtml = "上传图片";
            }
        }

        //上传图片
        protected void imageUpload_tj(object sender, EventArgs e)
        {
            UploadFile imageObj = new UploadFile();
            imageObj.MaxFileSize = 1024;  //最多只能上传1M的文件大小
            imageObj.FileType = "jpg|jepg|bmp";
            string imagePath = Server.MapPath("~/tmpupload/");

            //上传图片并保存
            imageObj.UploadFileGo(imagePath, imageUploader);
            if (imageObj.UploadState == false)
            {
                uploadInfo.Text = "";
                uploadInfo.Text = "上传失败:" + imageObj.UploadInfo;
            }
            else
            {
                uploadInfo.Text = "Tips:上传图片的格式为jpg,jepg,bmp,小于1M";
                imageToModify.ImageUrl = "tmpupload/" + imageObj.NewFileName;
                //限制图片的高度和宽度
                System.Drawing.Image img = System.Drawing.Image.FromFile(Server.MapPath("~/tmpupload/") + imageObj.NewFileName);
                int height = img.Height;
                int width = img.Width;
                if (width >= height)
                {
                    imageToModify.Style["width"] = "640px";
                    imageToModify.Style["height"] = (640.0 * height / width).ToString() + "px";
                }
                else
                {
                    imageToModify.Style["width"] = (640.0 * width / height).ToString() + "px";
                    imageToModify.Style["height"] = "640px";
                }
                hiddenHW.Value = width.ToString() + "," + height.ToString();
                hiddenFileName.Value = imageObj.NewFileName;
                modify.Style["display"] = "block";  //最后才显示图片
            }
        }

        //两个路径都是包含文件名的
        [DllImport("cutImage.dll", EntryPoint = "CutImage", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Auto)]
        public static extern void CutImage(int x, int y, int width, int height, [MarshalAs(UnmanagedType.LPStr)] String directory, [MarshalAs(UnmanagedType.LPStr)] String storePath);

        //提交选择
        protected void imageSelectArea_tj(object sender, EventArgs e)
        {
            string[] whArr = hiddenHW.Value.Split(',');
            int actualWidth = MyFunction.numeric(whArr[0]);
            int actualHeight = MyFunction.numeric(whArr[1]);
            double rate = 1.0;
            if (actualWidth >= actualHeight) rate = actualWidth / 640.0;
            else rate = actualHeight / 640.0;
            
            string[] selectArr = hiddenSelectHW.Value.Split(',');
            int acX = (int)(MyFunction.numeric(selectArr[0]) * rate);
            int acY = (int)(MyFunction.numeric(selectArr[1]) * rate);
            int acW = (int)(MyFunction.numeric(selectArr[2]) * rate);
            int acH = (int)(MyFunction.numeric(selectArr[3]) * rate);

            //保存图片在原始图片前面加个s
            if (acW > 0 && acH > 0)
            {
                CutImage(acX, acY, acW, acH, Server.MapPath("~/tmpupload/") + hiddenFileName.Value, Server.MapPath("~/tmpupload/s") + hiddenFileName.Value);
            }

            //关闭iframe
            string imageDiv = "<div id=\"cuttedImage\" style=\"display:none;\">" + hiddenFileName.Value + "</div>"; //filename
            string whois = "<div id=\"whois\" style=\"display:none;\">" + contentTitle.InnerText + "</div>";        //people,car,others
            Response.Write(imageDiv);
            Response.Write(whois);
            Response.Write("<script type=\"text/javascript\">");
            Response.Write("parent.$.fancybox.close();");
            Response.Write("</script>");
        }
    }
}
