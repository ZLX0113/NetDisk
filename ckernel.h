#ifndef CKERNEL_H
#define CKERNEL_H
#include "maindialog.h"
#include <QObject>
#include<INetMediator.h>
#include"packdef.h"
#include"logindialog.h"
#include"common.h"
#include "csqlite.h"
//#define USE_SERVER 1
//核心处理类
//单例
//单例如何实现？
//1.对构造、拷贝构造 析构进行私有化
//2.提供静态的、公有的获取对象的方法

//协议映射表

//类成员函数指针
class CKernel;
typedef void (CKernel::*PFUN)(unsigned int lSendIP , char* buf , int nlen);

class CKernel : public QObject
{
    Q_OBJECT
private:
    explicit CKernel(QObject *parent = nullptr);
    explicit CKernel(const CKernel& kernel){}
    ~CKernel(){}

    void loadIniFile();
    void setNetPackMap();
    void setSystemPath();
signals:
    void SIG_updateUploadFileProgress(int timestamp,int pos);
    void SIG_updateDownloadFileProgress(int timestamp,int pos);
public:

    static CKernel * GetInstance()  //在全局区申请空间的代码  调用的时候初始化一次 是线程安全的 无法进行回收
    {
        static CKernel kernel;
        return &kernel;
    }

private slots:
    //普通槽函数
    void slot_destory();
    void slot_registerCommit(QString tel,QString password,QString name);
    void slot_loginCommit(QString tel,QString password);
    //信号什么绝对路径的文件，上传到什么目录下
    void slot_uploadFile(QString path,QString dir);
    //信号什么路径下文件，上传到什么目录下
    void slot_uploadFolder(QString path,QString dir);
    //什么文件id，什么目录下的文件 下载
    void slot_downloadFile(int fileid,QString dir);
    //什么文件id，什么目录下的文件jia 下载
    void slot_downloadFolder(int fileid,QString dir);
    //什么路径下创建什么名字的文件夹
    void slot_addFolder(QString name,QString dir);
    void slot_getCurDirFileList();
    //选中文件，分享到什么路径下
    void slot_shareFile(QVector<int> fileidArray,QString dir);
    //获取什么分享码的文件 添加到什么目录
    void slot_getShareByLink( int code , QString dir );
    //删除什么目录下的 一系列的文件（文件id 数组）
    void slot_deleteFile(QVector<int> fileidArray,QString dir);
    //上传zanting
    void slot_setuploadPause(int timestamp,int isPause);
    //下载暂停 0开始 1暂停
    void slot_setdownloadPause(int timestamp,int isPause);
    //改变路径
    void slot_changeDir(QString dir);
    //搜索文件
    void slot_searchFile(QString key);
    //网络响应槽函数
    void slot_dealClientData(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealLoginRS(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealRegisterRS(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealUploadfileRS(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealFileContentRS(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealGetFileInfoRS(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealFileHeadRQ(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealFileContentRQ(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealAddFolderRS(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealQuickUploadRS(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealShareFileRS(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealMyShareRS(unsigned int lSendIP , char* buf , int nlen);
    void  slot_getMyShare();
    void slot_dealGetShareRs(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealFolderHaedRq(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealDeleteFileRs(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealContinueUploadRs(unsigned int lSendIP , char* buf , int nlen);
    void slot_dealSearchFileRs(unsigned int lSendIP , char* buf , int nlen);
    // 回收站
    void slot_getRecycle();
    void slot_dealGetRecycleRs(unsigned int lSendIP , char* buf , int nlen);
    void slot_restoreFile(QVector<int> fileids);
    void slot_dealRestoreFileRs(unsigned int lSendIP , char* buf , int nlen);
    // 收藏
    void slot_addFavorite(QString dir, QString name, int fileid);
    void slot_dealAddFavoriteRs(unsigned int lSendIP , char* buf , int nlen);
    void slot_getFavorite();
    void slot_dealGetFavoriteRs(unsigned int lSendIP , char* buf , int nlen);
    void slot_delFavorite(int fileid);
    void slot_dealDelFavoriteRs(unsigned int lSendIP , char* buf , int nlen);
#ifdef USE_SERVER
    void slot_dealServerData(unsigned int lSendIP , char* buf , int nlen);
#endif
private:

    void SendData(char* buf,int len);
private:
    Dialog* m_mainDialog;
    LoginDialog* m_loginDialog;
    QString m_ip;
    QString m_port;
    QString m_name;
    int m_id;
    QString m_CurDir;  //网盘当前的目录
    QString m_PreSearchDir; // 搜索前的目录，用于返回
    QString m_sysPath; //默认存储的系统路径（绝对路径）  exe同级下 NetDisk文件夹

    INetMediator* m_tcpClient;
    //key是时间戳 hhmmss
    std::map<int,FileInfo> m_mapTimestampToFileInfo;
    PFUN m_netPackMap[_DEF_PACK_COUNT];
    //退出标记
    bool m_quit;
    //数据库
    CSqlite * m_sql;
#ifdef USE_SERVER
    INetMediator* m_tcpServer;
#endif
private:
    void InitDatabase(int id);
    //缓存上传的任务
    void slot_writeUploadTask(FileInfo & info);
    //缓存下载的任务
    void slot_writeDownloadTask(FileInfo & info);
    //完成任务，删除上传记录
    void slot_deleteUploadTask(FileInfo & info);
    //完成任务，删除下载任务
    void slot_deleteDownloadTask(FileInfo & info);
    //加载上传任务
    void slot_getUploadTask(QList<FileInfo> & infolist);
    //加载下载任务
    void slot_getDownloadTask(QList<FileInfo> & infolist);

};

#endif // CKERNEL_H

