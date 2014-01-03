using System;
using System.Collections.Generic;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

namespace Video
{
    public partial class showvideo : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {
            string uid = Request.QueryString["uid"];
            string preStr = string.Format("{0:D9}", Convert.ToInt32(uid));
            imageToShow.ImageUrl = "tmpimg/" + preStr + "_img.jpg";
        }
    }
}
