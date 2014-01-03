using System;
using System.Data;
using System.Configuration;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.HtmlControls;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.IO;

/// <summary>
///文件上传类
/// </summary>
public class UploadFile
{
    private string _UploadInfo; // 文件上传的返回信息。   
    private bool _UploadState; // 文件上传的返回状态。   
    private string _FileType; // 允许上传文件的类型。   
    private int _FileSize; // 上传文件的大小，单位B   
    private int _MaxFileSize; // 上传文件大小的最大长度，单位B   
    private string _NewFileName; // 上传后的文件名

    /// <summary>初始化文件上传类</summary>
	public UploadFile()
	{
        _UploadInfo = "NONE";
        _UploadState = false;
        _FileType = "*";
        _MaxFileSize = 1048576;//1M
        _NewFileName = ""; 
	}

    /// <summary>文件上传的返回信息</summary>   
    public string UploadInfo
    {
        set { _UploadInfo = value; }
        get { return _UploadInfo; }
    }

    /// <summary>文件上传的返回状态，成功true，失败false</summary>   
    public bool UploadState
    {
        set { _UploadState = value; }
        get { return _UploadState; }
    }

    /// <summary>允许上传文件的类型,* 默认代表可任意类型,或自定义类型，如 "jpg|gif|bmp"</summary>   
    public string FileType
    {
        set { _FileType = value; }
        get { return _FileType; }
    }

    /// <summary>上传文件的大小，单位K</summary>   
    public int FileSize
    {
        get { return _FileSize / 1024; }
    }

    /// <summary>上传文件大小的最大长度，单位K</summary>   
    public int MaxFileSize
    {
        set { _MaxFileSize = value * 1024; }
        get { return _MaxFileSize / 1024; }
    }

    /// <summary>上传后的文件名</summary>   
    public string NewFileName
    {
        set { _NewFileName = value; }
        get { return _NewFileName; }
    }

    /// <summary>上传本地文件到服务器</summary>   
    /// <param name="strSaveDir">在服务器端保存的物理路径。</param>   
    /// <param name="FileUploadCtrlID">上传的文件对象，这里使用的是FileUpload控件，</param>   
    /// <param>第二个参数如果是HTMl Input(File)控件可改为：HtmlInputFile HtmCtrlObjUploadFile</param>   
    /// <returns></returns>   
    public void UploadFileGo(string strSaveDir, FileUpload FileUploadCtrlID)
    {
        int intFileExtPoint = FileUploadCtrlID.PostedFile.FileName.LastIndexOf("."); //最后一个 . 号的位置   
        string strFileExtName = FileUploadCtrlID.PostedFile.FileName.Substring(intFileExtPoint + 1).ToLower(); // 获取上传文件的后缀名。   

        _FileSize = FileUploadCtrlID.PostedFile.ContentLength;//上传的文件大小 byte   

        if (_FileSize == 0) // 判断是否有文件需要上传或所选文件是否为0字节。   
        {
            _UploadInfo = "没有选择要上传的文件或所选文件大小为0字节";
            _UploadState = false;
            return; // 返回文件上传状态和信息。   
        }

        if (_FileSize > _MaxFileSize) // 限制要上传的文件大小(byte)。   
        {
            _UploadInfo = "上传的文件超过限制大小(" + (_MaxFileSize / 1024).ToString() + "K)";
            _UploadState = false;
            return; // 返回文件上传状态和信息。   
        }

        if (_FileType != "*")
        {
            if (_FileType.ToLower().IndexOf(strFileExtName.ToLower().Trim()) == -1) // 判断要上传的文件类型的是否在允许的范围内。   
            {
                _UploadInfo = "不允许上传的文件类型(允许的类型：|" + _FileType + ")";
                _UploadState = false;
                return; // 返回文件上传状态和信息   
            }
        }

        if (_NewFileName == "")
        {
            DateTime dteNow = DateTime.Now; // 定义日期对象，为上传后的文件命名。   
            _NewFileName = dteNow.Year.ToString() + dteNow.Month.ToString() + dteNow.Day.ToString() + GetRandomStr(8); // 随机地为上传后的文件命名,日期+随机数。   
            _NewFileName = _NewFileName + "." + strFileExtName; //包含扩展名的文件名   
        }
        FileUploadCtrlID.PostedFile.SaveAs(this.GetSaveDirectory(strSaveDir) + _NewFileName); // 以新的文件名保存上传的文件到指定物理路径。              
        _UploadInfo = "文件上传成功"; // 返回上传后的服务器端文件物理路径。   
        _UploadState = true;

    }


    /// <summary>获取指定位数的随机数</summary>   
    /// <param name="RndNumCount">随机数位数。</param>   
    /// <returns></returns>   
    private string GetRandomStr(int RndNumCount)
    {
        string RandomStr;
        RandomStr = "";
        Random Rnd = new Random();
        for (int i = 1; i <= RndNumCount; i++)
        {
            RandomStr += Rnd.Next(0, 9).ToString();
        }
        return RandomStr;
    }


    /// <summary>获取上传文件存放目录</summary>   
    /// <param name="DirectoryPath">存放文件的物理路径。</param>   
    /// <returns>返回存放文件的目录。</returns>   
    public string GetSaveDirectory(string DirectoryPath)
    {
        if (!Directory.Exists(DirectoryPath)) // 判断当前目录是否存在。   
        {
            Directory.CreateDirectory(DirectoryPath); // 建立上传文件存放目录。   
        }
        return DirectoryPath;
    }   
}
