#ifndef CLOGIC_H
#define CLOGIC_H

#include"TCPKernel.h"

class CLogic
{
public:
    CLogic( TcpKernel* pkernel )
    {
        m_pKernel = pkernel;
        m_sql = pkernel->m_sql;
        m_tcp = pkernel->m_tcp;
    }
public:
    //设置协议映射
    void setNetPackMap();

    int getNumber()
    {
        return 1000000000;
    }
    /************** 发送数据*********************/
    void SendData( sock_fd clientfd, char*szbuf, int nlen )
    {
        m_pKernel->SendData( clientfd ,szbuf , nlen );
    }
    /************** 网络处理 *********************/
    //注册
    void RegisterRq(sock_fd clientfd, char*szbuf, int nlen);
    //登录
    void LoginRq(sock_fd clientfd, char*szbuf, int nlen);
    //上传文件请求
    void UploadFileRq(sock_fd clientfd, char*szbuf, int nlen);
    //文件块请求
    void FileContentRq(sock_fd clientfd, char*szbuf, int nlen);
    //获取文件信息请求
    void GetFileInfoRq(sock_fd clientfd, char*szbuf, int nlen);
    //下载文件请求
    void DownloadFileRq(sock_fd clientfd, char*szbuf, int nlen);
    //下载文件夹请求
    void DownloadFolderRq(sock_fd clientfd, char*szbuf, int nlen);
    void DownloadFolder(int userId, int& timestamp, sock_fd clientfd, list<string>& lstRes);
    void DownloadFile(int userId, int& timestamp, sock_fd clientfd, list<string>& lstRes);
    //文件头回复
    void FileHeadRs(sock_fd clientfd, char*szbuf, int nlen);
    //文件内容回复
    void FileContentRs(sock_fd clientfd, char*szbuf, int nlen);
    //新建文件夹
    void AddFolderRq(sock_fd clientfd, char*szbuf, int nlen);
    //分享文件
    void shareFileRq(sock_fd clientfd, char*szbuf, int nlen);
    //刷新分享文件
    void MyShareRq(sock_fd clientfd, char*szbuf, int nlen);
    //获取分享文件
    void GetShareRq(sock_fd clientfd, char*szbuf, int nlen);
    void GetShareByFile( int userId, int fileid , string dir , string name , string time );
    void GetShareByFolder( int userId, int fileid , string dir , string name , string time ,int fromuserid,string fromdir);
    /*******************************************/
    void ShareItem(int userid,int fileid,string dir,string time ,int link);
    void DeleteFileRq(sock_fd clientfd, char*szbuf, int nlen);
    void DeleteOneItem(int userid, int fileid, string dir);
    void DeleteFile(int u_id, int f_id, string dir, string path);
    void DeleteFolder(int u_id, int f_id, string dir, string name);

    void ContinueDownloadRq(sock_fd clientfd, char*szbuf, int nlen);
    void ContinueUploadRq(sock_fd clientfd, char*szbuf, int nlen);
    // 搜索文件
    void SearchFileRq(sock_fd clientfd, char*szbuf, int nlen);
    // 回收站
    void GetRecycleRq(sock_fd clientfd, char*szbuf, int nlen);
    void RestoreFileRq(sock_fd clientfd, char*szbuf, int nlen);
    // 收藏
    void AddFavoriteRq(sock_fd clientfd, char*szbuf, int nlen);
    void GetFavoriteRq(sock_fd clientfd, char*szbuf, int nlen);
    void DelFavoriteRq(sock_fd clientfd, char*szbuf, int nlen);
private:
    TcpKernel* m_pKernel;
    CMysql * m_sql;
    Block_Epoll_Net * m_tcp;
    MyMap<int,UserInfo*> m_mapIDToUserInfo;
    //key是userid*10e9 +timestamp value文件信息
    MyMap<int64_t,FileInfo*> m_mapTimestampToFileInfo;
};

#endif // CLOGIC_H
