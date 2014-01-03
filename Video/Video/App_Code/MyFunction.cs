using System;
using System.Data;
using System.Data.SqlClient;
using System.Configuration;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.HtmlControls;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;

/// <summary>
///MyFunction 的摘要说明
/// </summary>
public class MyFunction
{
    protected static SqlConnection conn = new SqlConnection();  //sql连接
    protected static SqlCommand comm = new SqlCommand();        //sql命令

	public MyFunction()
	{
		//
		//TODO: 在此处添加构造函数逻辑
		//
	}

    /// <summary>
    /// 打开数据库连接
    /// </summary>
    private static void openConnection()
    {
        if (conn.State == ConnectionState.Closed)
        {
            //这里虽然是sqlserver，但是只是名字。其实是ACCESS数据库
            conn.ConnectionString = System.Web.Configuration.WebConfigurationManager.ConnectionStrings["sqlserver"].ConnectionString;
            comm.Connection = conn;
            try
            {
                conn.Open();
            }
            catch (Exception e)
            {
                throw new Exception(e.Message);
            }
        }
    }

    /// <summary> 
    /// 关闭当前数据库连接 
    /// </summary> 
    private static void closeConnection()
    {
        if (conn.State == ConnectionState.Open)
            conn.Close();
        conn.Dispose();
        comm.Dispose();
    }

    /// <summary> 
    /// 执行Sql更新语句 
    /// </summary> 
    /// <param name="sqlstr">传入的Sql语句</param>
    /// <returns>布尔值</returns>
    public static bool ExecuteSql(string sqlstr)
    {
        int isUpdateOk = 0;
        try
        {
            openConnection();
            comm.CommandType = CommandType.Text;
            comm.CommandText = sqlstr;
            isUpdateOk = Convert.ToInt32(comm.ExecuteNonQuery());
        }
        catch (Exception e)
        {
            throw new Exception(e.Message);
        }
        finally
        {
            closeConnection();
        }
        if (isUpdateOk > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    /// <summary> 
    /// 返回指定Sql语句的SqlDataReader，请注意，在使用后请关闭本对象，同时将自动调用closeConnection()来关闭数据库连接 
    /// 方法关闭数据库连接 
    /// </summary> 
    /// <param name="sqlstr">传入的Sql语句</param> 
    /// <returns>SqlDataReader对象</returns> 
    public static SqlDataReader dataReader(string sqlstr)
    {
        SqlDataReader dr = null;
        try
        {
            openConnection();
            comm.CommandText = sqlstr;
            comm.CommandType = CommandType.Text;
            dr = comm.ExecuteReader(CommandBehavior.CloseConnection);
        }
        catch
        {
            try
            {
                dr.Close();
                closeConnection();
            }
            catch
            {
            }
        }
        return dr;
    }

    ///<summary>
    ///关闭datareader
    ///传入SqlDataReader的ref
    ///</summary>
    public static void closeDataReader(ref SqlDataReader sdr)
    {
        try
        {
            sdr.Close();
            sdr.Dispose();
        }
        catch (Exception)
        { }
    }

    /// <summary> 
    /// 返回指定Sql语句的DataSet 
    /// </summary> 
    /// <param name="sqlstr">传入的Sql语句</param> 
    /// <returns>DataSet</returns> 
    public static DataSet dataSet(string sqlstr)
    {
        DataSet ds = new DataSet();
        SqlDataAdapter da = new SqlDataAdapter();
        try
        {
            openConnection();
            comm.CommandType = CommandType.Text;
            comm.CommandText = sqlstr;
            da.SelectCommand = comm;
            da.Fill(ds);
        }
        catch (Exception e)
        {
            throw new Exception(e.Message);
        }
        finally
        {
            closeConnection();
        }
        return ds;
    }

    /// <summary>
    /// 保证表单的安全性
    /// </summary>
    /// <param name="str">输入字符串</param>
    /// <returns></returns>
    public static string safeForm(string str)
    {
        str = str.Trim();
        str = str.Replace("'", "''");
        return str;
    }

    /// <summary> 
    /// 返回指定Sql语句的DataTable 
    /// </summary> 
    /// <param name="sqlstr">传入的Sql语句</param> 
    /// <returns>DataTable</returns> 
    public static DataTable dataTable(string sqlstr)
    {
        SqlDataAdapter da = new SqlDataAdapter();
        DataTable datatable = new DataTable();
        try
        {
            openConnection();
            comm.CommandType = CommandType.Text;
            comm.CommandText = sqlstr;
            da.SelectCommand = comm;
            da.Fill(datatable);
        }
        catch (Exception e)
        {
            throw new Exception(e.Message);
        }
        finally
        {
            closeConnection();
        }
        return datatable;
    }

    /// <summary> 
    /// 执行指定Sql语句,同时给传入DataTable进行赋值 
    /// </summary> 
    /// <param name="sqlstr">传入的Sql语句</param> 
    /// <param name="dt">ref DataTable dt </param> 
    public static void dataTable(string sqlstr, ref DataTable dt)
    {
        SqlDataAdapter da = new SqlDataAdapter();
        try
        {
            openConnection();
            comm.CommandType = CommandType.Text;
            comm.CommandText = sqlstr;
            da.SelectCommand = comm;
            da.Fill(dt);
        }
        catch (Exception e)
        {
            throw new Exception(e.Message);
        }
        finally
        {
            closeConnection();
        }
    }

    /// <summary>
    /// 分割字符串，保留指定的长度
    /// </summary>
    /// <param name="Vstring">要分割的字符串</param>
    /// <param name="nLength">需要保留的长度</param>
    /// <returns></returns>
    public static string subStr(string Vstring, int nLength)
    {

        if (Vstring.Length <= nLength)
        {
            return Vstring;
        }
        string sNewStr = Vstring.Substring(0, nLength);
        sNewStr = sNewStr + "...";
        return sNewStr;
    }

    /// <summary>
    /// 字符串生成数字
    /// </summary>
    /// <param name="number"></param>
    /// <returns></returns>
    public static int numeric(string number)
    {
        int num = 0;
        try
        {
            num = int.Parse(number);//System.Globalization.NumberStyles.HexNumber
            return num;
        }
        catch
        {
            return 0;
        }
    }

}
