#include "ckernel.h"
#include<QDebug>
#include"TcpClientMediator.h"
#include"TcpServerMediator.h"
#include<QMessageBox>
#include<QThread>


#define NetMap(a) m_netPackMap[(a) - _DEF_PACK_BASE]


CKernel::CKernel(QObject *parent) : QObject(parent),m_id(0),m_CurDir("/"),m_PreSearchDir(""),m_quit(false )
{

    //设置协议映射表
    setNetPackMap();

    //加载配置文件
    loadIniFile();
    //设置默认路径
    setSystemPath();
    //网络连接
     m_tcpClient=new TcpClientMediator;
     connect(m_tcpClient,SIGNAL(SIG_ReadyData(uint,char*,int)),this,SLOT(slot_dealClientData(uint,char*,int)) );
#ifdef USE_SERVER
     m_tcpServer=new TcpServerMediator;
     connect(m_tcpServer,SIGNAL(SIG_ReadyData(uint,char*,int)),this,SLOT(slot_dealServerData(uint,char*,int)) );
     //服务端使用默认地址
     m_tcpServer->OpenNet();
#endif

     //客户端连接真实地址
     m_tcpClient->OpenNet(m_ip.toStdString().c_str(),m_port.toUInt());
     m_loginDialog=new LoginDialog;
     m_loginDialog->show();
     connect(m_loginDialog,SIGNAL(SIG_registerCommit(QString,QString,QString)),this,SLOT(slot_registerCommit(QString,QString,QString)));
     connect(m_loginDialog,SIGNAL(SIG_loginCommit(QString,QString)),this,SLOT(slot_loginCommit(QString,QString)));
    //在此显示窗口，在申请空间时要申请堆区的空间，防止函数执行完就销毁
    m_mainDialog=new Dialog;


    connect(m_mainDialog,SIGNAL(SIG_close()),this,SLOT(slot_destory()));
   // m_mainDialog->show();
#ifdef USE_SERVER
    //测试 对服务器发送数据
    char strBuf[100]="hello server";  //sizeof+数组名  整个数组的大小
    int len=strlen("hello server");  //strlen函数不包括\0
    m_tcpClient->SendData(0,strBuf,len+1);  //客户端一定是发给服务器的，套接字参数随意
    //    STRU_LOGIN_RQ rq;
    //    m_tcpClient->SendData(0,(char*)&rq,sizeof(rq));
#endif

    connect(m_mainDialog,SIGNAL(SIG_uploadFile(QString,QString)),this,SLOT(slot_uploadFile(QString,QString)));

    connect(this,SIGNAL(SIG_updateUploadFileProgress(int ,int )),m_mainDialog,SLOT(slot_updateUploadFileProgress(int ,int )));
    connect(m_mainDialog,SIGNAL(SIG_downloadFile(int,QString)),this,SLOT(slot_downloadFile(int,QString)));
    connect(m_mainDialog,SIGNAL(SIG_downloadFolder(int,QString)),this,SLOT(slot_downloadFolder(int,QString)));
    connect(this,SIGNAL(SIG_updateDownloadFileProgress(int,int)),m_mainDialog,SLOT(slot_updateDownloadFileProgress(int,int)));
    connect(m_mainDialog,SIGNAL(SIG_addFolder(QString,QString)),this,SLOT(slot_addFolder(QString,QString)));
    connect(m_mainDialog,SIGNAL(SIG_changeDir(QString)),this,SLOT(slot_changeDir(QString)));
    connect(m_mainDialog,SIGNAL(SIG_uploadFolder(QString,QString)),this,SLOT(slot_uploadFolder(QString,QString)));
    connect(m_mainDialog,SIGNAL(SIG_shareFile(QVector<int>, QString )),this,SLOT(slot_shareFile(QVector<int>, QString )));
    connect(m_mainDialog,SIGNAL(SIG_getShareByLink( int, QString )),this,SLOT(slot_getShareByLink(int, QString )));
    connect(m_mainDialog,SIGNAL(SIG_deleteFile(QVector<int>, QString)),this,SLOT(slot_deleteFile(QVector<int>, QString)));

    connect(m_mainDialog, SIGNAL(SIG_setuploadPause(int,int)),
        this, SLOT(slot_setuploadPause(int,int)));

    connect(m_mainDialog, SIGNAL(SIG_setdownloadPause(int,int)),
        this, SLOT(slot_setdownloadPause(int,int)));
    connect(m_mainDialog, SIGNAL(SIG_searchFile(QString)),
        this, SLOT(slot_searchFile(QString)));
    connect(m_mainDialog, SIGNAL(SIG_getRecycle()),
        this, SLOT(slot_getRecycle()));
    connect(m_mainDialog, SIGNAL(SIG_restoreFile(QVector<int>)),
        this, SLOT(slot_restoreFile(QVector<int>)));
    connect(m_mainDialog, SIGNAL(SIG_addFavorite(QString,QString,int)),
        this, SLOT(slot_addFavorite(QString,QString,int)));
    connect(m_mainDialog, SIGNAL(SIG_getFavorite()),
        this, SLOT(slot_getFavorite()));
    connect(m_mainDialog, SIGNAL(SIG_delFavorite(int)),
        this, SLOT(slot_delFavorite(int)));
}
void CKernel::setNetPackMap()
{
    memset(m_netPackMap,0,sizeof(PFUN)*_DEF_PACK_COUNT);

    //协议映射表 key协议头偏移量 value 函数zhizhen
    //通过协议头 找到对应处理函数
    //m_netPackMap[_DEF_PACK_REGISTER_RS-_DEF_PACK_BASE]=&CKernel::slot_sealLoginRS;
    NetMap(_DEF_PACK_LOGIN_RS)=&CKernel::slot_dealLoginRS;
    NetMap(_DEF_PACK_REGISTER_RS)=&CKernel::slot_dealRegisterRS;
    NetMap(_DEF_PACK_UPLOAD_FILE_RS)=&CKernel::slot_dealUploadfileRS;
    NetMap(_DEF_PACK_FILE_CONTENT_RS)=&CKernel::slot_dealFileContentRS;
    NetMap(_DEF_PACK_GET_FILE_INFO_RS)=&CKernel::slot_dealGetFileInfoRS;
    NetMap(_DEF_PACK_FILE_HEADER_RQ)=&CKernel::slot_dealFileHeadRQ;
    NetMap(_DEF_PACK_FILE_CONTENT_RQ)=&CKernel::slot_dealFileContentRQ;
    NetMap(_DEF_PACK_ADD_FOLDER_RS) = &CKernel::slot_dealAddFolderRS;
    NetMap(_DEF_PACK_QUICK_UPLOAD_RS) = &CKernel::slot_dealQuickUploadRS;
    NetMap(_DEF_PACK_SHARE_FILE_RS) = &CKernel::slot_dealShareFileRS;
    NetMap(_DEF_PACK_MY_SHARE_RS) = &CKernel::slot_dealMyShareRS;
    NetMap(_DEF_PACK_GET_SHARE_RS) = &CKernel::slot_dealGetShareRs;
    NetMap(_DEF_PACK_FOLDER_HEADER_RQ) = &CKernel::slot_dealFolderHaedRq;
    NetMap(_DEF_PACK_DELETE_FILE_RS) = &CKernel::slot_dealDeleteFileRs;
    NetMap(_DEF_PACK_CONTINUE_UPLOAD_RS) = &CKernel::slot_dealContinueUploadRs;
    NetMap(_DEF_PACK_SEARCH_FILE_RS) = &CKernel::slot_dealSearchFileRs;
    NetMap(_DEF_PACK_GET_RECYCLE_RS) = &CKernel::slot_dealGetRecycleRs;
    NetMap(_DEF_PACK_RESTORE_FILE_RS) = &CKernel::slot_dealRestoreFileRs;
    NetMap(_DEF_PACK_ADD_FAVORITE_RS) = &CKernel::slot_dealAddFavoriteRs;
    NetMap(_DEF_PACK_GET_FAVORITE_RS) = &CKernel::slot_dealGetFavoriteRs;
    NetMap(_DEF_PACK_DEL_FAVORITE_RS) = &CKernel::slot_dealDelFavoriteRs;
}

#include<QDir>
#include <QCoreApplication>
//系统路径 ：exe同级 ./NetDisk +dir +name
void CKernel::setSystemPath()
{
    QString path = QCoreApplication::applicationDirPath() + "/NetDisk";

    QDir dir;
    //没有文件夹就创建文件夹
    if(!dir.exists(path))
    {
        dir.mkdir(path); //这个函数只能创建一层
    }
    m_sysPath=path;
}

void CKernel::SendData(char *buf, int len)
{
    m_tcpClient->SendData(0,buf,len);
}
#include<QCoreApplication>
#include<QFileInfo>
//配置文件使用类
#include<QSettings>
void CKernel::loadIniFile()
{
    //默认值
    m_ip="192.168.207.130";
    m_port="8004";

    //获取exe目录
    QString path=QCoreApplication::applicationDirPath()+"/config.ini";
    //根据目录查看文件是否存在，存在加载 不存在创建并且写入默认值
    QFileInfo info(path);
    if(info.exists())
    {
        //存在
        QSettings setting(path,QSettings::IniFormat);
        //打开组
        setting.beginGroup("net");
        QVariant strIP=setting.value("ip","");
        QVariant strPort=setting.value("port","");
        if(!strIP.toString().isEmpty()) m_ip=strIP.toString();
        if(!strPort.toString().isEmpty()) m_port=strPort.toString();
        //关闭组
        setting.endGroup();
    }
    else
    {
        //不存在
        QSettings setting(path,QSettings::IniFormat);
        //打开组
        setting.beginGroup("net");
        //设置键值对
        setting.value("ip",m_ip);
        setting.value("port",m_port);
        //关闭组
        setting.endGroup();
    }
    qDebug()<<"ip是"<<m_ip<<"端口是"<<m_port;

}

void CKernel::slot_destory()
{
    qDebug()<<__func__;
    m_quit=true;
    m_tcpClient->CloseNet();
    delete m_tcpClient;
    delete m_mainDialog;
    delete m_loginDialog;

}
#include"md5.h"
#define MD5_KEY "1234"
//password_1234
//生成md5函数
static std::string getMD5(QString val)  //静态函数，只有当前文件文件可以使用
{
    QString str=QString("%1_%2").arg(val).arg(MD5_KEY);
    MD5 md(str.toStdString());
    qDebug()<<str<<"MD5："<<md.toString().c_str();
    return md.toString();
}
#include<QTextCodec>

// QString -> char* gb2312
void Utf8ToGB2312( char* gbbuf , int nlen ,QString& utf8)
{
    //转码的对象
    QTextCodec * gb2312code = QTextCodec::codecForName( "gb2312");
    //QByteArray char 类型数组的封装类 里面有很多关于转码 和 写IO的操作
    QByteArray ba = gb2312code->fromUnicode( utf8 );// Unicode -> 转码对象的字符集

    strcpy_s ( gbbuf , nlen , ba.data() );
}

//获取文件md
static std::string getFileMD5(QString path)
{
    //打开文件 ，读取文件，读到md5类，生成md5
    FILE* pFile=nullptr;
    //fopen 如果有中文 支持ANSI 编码 使用ascii码
    //path里面是utf-8（qt默认）编码
    char buf[1000]="";
    Utf8ToGB2312(buf,1000,path);
    pFile=fopen(buf,"rb");  //二进制只读
    if(!pFile)
    {
        qDebug()<<"file md5 open failed";
        return " ";
    }
    MD5 md;
    int len=0;
    do{
        len=fread(buf,1,1000,pFile);//缓冲区，一次读多少，读多少次，文件指针 返回值读成功次数
        md.update(buf,len);  //不断拼接文本 不断更新md5

        //为了避免阻塞窗口线程，影响事件循环，加入下面的处理 将信号取出并且执行
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);  //在循环中处理GUI事件，防止界面卡死,100毫秒超时，确保不会长时间阻塞
    }while(len>0);
    fclose(pFile);
    qDebug()<<"file md5:"<<md.toString().c_str();
    return md.toString();

}

void CKernel::slot_registerCommit(QString tel, QString password, QString name)
{
    STRU_REGISTER_RQ rq;
    strcpy(rq.tel,tel.toStdString().c_str());
    //strcpy(rq.password,password.toStdString().c_str());
    strcpy(rq.password,getMD5(password).c_str());
    //昵称要兼容中文
    std::string strName=name.toStdString();
    strcpy(rq.name,strName.c_str());

    SendData((char*)&rq,sizeof(rq));

}

void CKernel::slot_loginCommit(QString tel, QString password)
{
    STRU_LOGIN_RQ rq;
    strcpy(rq.tel,tel.toStdString().c_str());
    strcpy(rq.password,password.toStdString().c_str());
     strcpy(rq.password,getMD5(password).c_str());
    SendData((char*)&rq,sizeof(rq));

}
#include"common.h"
#include<QFileInfo>
#include"QDateTime"
//上传文件的槽函数
void CKernel::slot_uploadFile(QString path, QString dir)
{
    QFileInfo qFileInfo(path);

    //文件信息的存储
    FileInfo info;
    info.absolutePath=path;
    info.dir=dir;
    info.md5=QString::fromStdString(getFileMD5(path));
    info.name=qFileInfo.fileName();

    info.size=qFileInfo.size();  //fopen fseek ftell
    info.time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    info.type="file";
    char buf[1000]="";
    Utf8ToGB2312(buf,1000,path);

    info.pFile=fopen(buf,"rb");
    if(!info.pFile)
    {
        qDebug()<<"file open failed";
        return ;
    }
    int timestamp=QDateTime::currentDateTime().toString("hhmmsszzz").toInt();
    //bug修复 如果上传时间非常相近 可能出现时间戳一样的状况，就会造成文件的覆盖
    //修复方法 就是 反复检测时间戳是否存在
    while(m_mapTimestampToFileInfo.count(timestamp)>0)
    {
        timestamp++;
    }
    info.timestamp=timestamp;
    //存储到map里面
    m_mapTimestampToFileInfo[timestamp]=info;
    //发上传文件请求
    STRU_UPLOAD_FILE_RQ rq;
    //兼容中文
    std::string strdir=dir.toStdString();
    strcpy(rq.dir,strdir.c_str());
    std::string strname=info.name.toStdString();
    strcpy(rq.fileName,strname.c_str());
    strcpy_s(rq.fileType, sizeof(rq.fileType), "file");
    strcpy(rq.md5,info.md5.toStdString().c_str());
    rq.size=info.size;
    strcpy( rq.time,info.time.toStdString().c_str());
    strcpy(rq.md5,info.md5.toStdString().c_str());
    rq.timestamp=timestamp;
    rq.userid=m_id;
   SendData((char*)&rq,sizeof(rq));

}

void CKernel::slot_uploadFolder(QString path, QString dir)
{
    qDebug()<<__func__;
    //当前文件夹处理 addfolder c:/项目 下面有 /113/ /112/ 1.txt 上传到/05/
    QFileInfo info(path);
    QDir dr(path);
    qDebug()<<"folder:"<<info.fileName()<<"dir:"<<dir;
    slot_addFolder(info.fileName(),dir);
    //获取文件夹下面一层 所有文件的路径
    QFileInfoList lst=dr.entryInfoList(); //获取路径下所有文件的文件信息列表
    //遍历所有文件
    QString newdir=dir+info.fileName()+"/";
    for(int i=0;i<lst.size();i++)
    {
        QFileInfo file=lst.at(i);
        //如果是.继续
        if(file.fileName()==".")  continue;
        //如果是..继续
        if(file.fileName()=="..")  continue;
    //如果是文件就调用 uploadFile-> 路径 文件信息的绝对路径 传到什么目录 /05/项目
        if(file.isFile())
        {
            qDebug()<<"file:"<<file.absoluteFilePath()<<"dir:"<<newdir;
            slot_uploadFile(file.absoluteFilePath(),newdir);
        }

        //如果是文件夹 slot_uoloadFolder 递归
        if(file.isDir())
        {
            slot_uploadFolder(file.absoluteFilePath(),newdir);
        }
    }
}

void CKernel::slot_downloadFile(int fileid, QString dir)
{
    //写请求
    STRU_DOWNLOAD_FILE_RQ rq;
    std::string strDir=dir.toStdString();
    strcpy(rq.dir,strDir.c_str());
    rq.fileid=fileid;
    int timestamp=QDateTime::currentDateTime().toString("hhmmsszzz").toInt();
    while(m_mapTimestampToFileInfo.count(timestamp)>0)
    {
        timestamp++;
    }
    rq.timestamp=timestamp;
    rq.userid=m_id;
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_downloadFolder(int fileid, QString dir)
{
    STRU_DOWNLOAD_FOLDER_RQ rq;
    string strDir = dir.toStdString();
    strcpy(rq.dir, strDir.c_str());

    rq.fileid = fileid;
    int timestamp = QDateTime::currentDateTime().toString("hhmmsszzz").toInt();
    while(m_mapTimestampToFileInfo.count(timestamp) > 0){
        timestamp++;
    }
    rq.timestamp = timestamp;
    rq.userid = m_id;

    SendData((char*)&rq, sizeof(rq));
}
//新建文件夹
void CKernel::slot_addFolder(QString name, QString dir)
{
    STRU_ADD_FOLDER_RQ rq;
    string strDir=dir.toStdString();
    strcpy(rq.dir,strDir.c_str());

    string strName=name.toStdString();
    strcpy(rq.fileName,strName.c_str());

    string strTime=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
    strcpy(rq.time,strTime.c_str());
    rq.userid=m_id;
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_getCurDirFileList()
{
    //向服务器发送信息
    STRU_GET_FILE_INFO_RQ rq;
    rq.userid=m_id;
    //兼容中文
    std::string strDir=m_CurDir.toStdString();
    strcpy(rq.dir,strDir.c_str());

    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_shareFile(QVector<int> fileidArray, QString dir)
{
    //打包
    int packlen=sizeof(SRTU_SHARE_FILE_RQ)+sizeof(int)*fileidArray.size();
    SRTU_SHARE_FILE_RQ* rq=(SRTU_SHARE_FILE_RQ*)malloc(packlen);
    rq->init();
    rq->itemConunt=fileidArray.size();
    for(int i=0;i<fileidArray.size();i++)
    {
        rq->fileArray[i]=fileidArray[i];
    }
    rq->userid=m_id;
    std::string strDir=dir.toStdString();
    strcpy(rq->dir,strDir.c_str());
    QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    strcpy(rq->shareTime,time.toStdString().c_str());
    SendData((char*)rq,packlen);

    free(rq);
}

void CKernel::slot_getShareByLink(int code, QString dir)
{
    //发请求
    STRU_GET_SHARE_RQ rq;
    string tmpDir = dir.toStdString();
    strcpy( rq.dir , tmpDir.c_str() );
    rq.shareLink = code;
    string time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();

    strcpy( rq.time , time.c_str() );
    rq.userid = m_id;

    SendData( (char*)&rq , sizeof(rq) );
}

void CKernel::slot_deleteFile(QVector<int> fileidArray, QString dir)
{
    //发送请求
    int packlen=sizeof(STRU_DELETE_FILE_RQ)+fileidArray.size()*sizeof(int);
    STRU_DELETE_FILE_RQ* rq=(STRU_DELETE_FILE_RQ*)malloc(packlen);
    rq->init();
    string strDir=dir.toStdString();
    strcpy(rq->dir,strDir.c_str());
    rq->fileCount=fileidArray.size();

    rq->userid=m_id;
    for(int i=0;i<rq->fileCount;i++)
    {
        rq->fileidArray[i]=fileidArray[i];
    }
    SendData((char*)rq,packlen);
    free(rq);
}

void CKernel::slot_setuploadPause(int timestamp, int isPause)
{
    //isPause 1 从正在上传变为暂停  ispause 0 从暂停变为继续上传
    //需要找到文件信息结构体
    //map里面 有 程序未退的情况 直接置位
    //map里面没有 证明程序退出的 断点续传 需要走协议
    if(m_mapTimestampToFileInfo.count(timestamp)>0)
    {
        m_mapTimestampToFileInfo[timestamp].isPause=isPause;
        //恢复上传：内存中已有文件信息，通过续传协议询问服务器已上传位置，然后继续发送文件块
        if(isPause==0)
        {
            FileInfo& info=m_mapTimestampToFileInfo[timestamp];
            STRU_CONTINUE_UPLOAD_RQ rq;
            string strDir=info.dir.toStdString();
            strcpy(rq.dir,strDir.c_str());
            rq.fileid=info.fileid;
            rq.timestamp=timestamp;
            rq.userid=m_id;
            SendData((char*)&rq,sizeof(rq));
        }
    }
    else
    {
        //断点续传
        //创建fileinfo 然年后打开文件 放到map中

        FileInfo info;
        bool res=m_mainDialog->slot_getUplaodFileInfoByTimesatmp(timestamp,info);
        if(!res)
        {
            return;
        }
        //转化 路径妆化为ASCII
        char pathbuf[1000]="";
        Utf8ToGB2312(pathbuf,1000,info.absolutePath);
        //打开文件 二进制只读
        info.pFile=fopen(pathbuf,"rb");
        if(!info.pFile)
        {
            qDebug()<<"打开失败"<<info.absolutePath;
            return;
        }
        info.isPause=0;  //避免开始 就停在循环哪里
        m_mapTimestampToFileInfo[timestamp]=info;

        //发送上传续传请求
        STRU_CONTINUE_UPLOAD_RQ rq;
        string strDir=info.dir.toStdString();
        strcpy(rq.dir,strDir.c_str());
        rq.fileid=info.fileid;
        rq.timestamp=timestamp;
        rq.userid=m_id;

        SendData((char*)&rq,sizeof(rq));


    }
}

void CKernel::slot_setdownloadPause(int timestamp, int isPause)
{
    //isPause 1 从正在下载变为暂停  ispause 0 从暂停变为继续下载
    //需要找到文件信息结构体
    //map里面 有 程序未退的情况 直接置位
    //map里面没有 证明程序退出的 断点续传 需要走协议
    if(m_mapTimestampToFileInfo.count(timestamp)>0)
    {
        m_mapTimestampToFileInfo[timestamp].isPause=isPause;
        //恢复下载：内存中已有文件信息，通过续传协议告诉服务器从当前位置继续发送文件块
        if(isPause==0)
        {
            FileInfo& info=m_mapTimestampToFileInfo[timestamp];
            STRU_CONTINUE_DOWNLOAD_RQ rq;
            rq.fileid = info.fileid;
            string dirstr = info.dir.toStdString();
            strcpy( rq.dir , dirstr.c_str() );
            rq.pos = info.pos;
            rq.timestamp = info.timestamp;
            rq.userid = m_id;
            SendData( (char*)&rq , sizeof(rq) );
        }
    }
    else
    {
        //下载的信息 下载到数据库，重新登录加载，然后点击开始（继续）
        if(isPause==0)
        {
            //断点续传
            //1.创建文件信息结构体 装到map里面
            //信息在哪里？ 可以冲控件中取
            FileInfo info;
            bool res=m_mainDialog->slot_getDownlaodFileInfoByTimesatmp(timestamp,info);
            if(!res)
            {
                return;
            }

            //转化 路径妆化为ASCII
            char pathbuf[1000]="";
            Utf8ToGB2312(pathbuf,1000,info.absolutePath);
            //打开文件 二进制追加 不能是w 因为回被清空
            info.pFile=fopen(pathbuf,"ab");
            if(!info.pFile)
            {
                qDebug()<<"打开失败"<<info.absolutePath;
                return;
            }
            info.isPause=0;  //避免开始 就停在循环哪里
            m_mapTimestampToFileInfo[timestamp]=info;
            //2.发协议 告诉服务器 文件下载到哪里了，然后服务器跳转到哪里，从哪里开始西川，然后文件块发送
            //服务器接收 有两种可能 1.文件信息还在（客户端出现异常很快就好了，没有超过 预定的客户端删除文件所有信息的时间）2.不在
            STRU_CONTINUE_DOWNLOAD_RQ rq;
            rq.fileid = info.fileid;
            string dirstr = info.dir.toStdString();
            strcpy( rq.dir , dirstr.c_str() );
            rq.pos = info.pos;
            rq.timestamp = info.timestamp;
            rq.userid = m_id;
            SendData( (char*)&rq , sizeof(rq) );

        }

    }
}
void CKernel::slot_changeDir(QString dir)
{
        // 如果当前在搜索模式下（有保存的搜索前目录），点击路径返回搜索前目录
        if (!m_PreSearchDir.isEmpty() && dir == "/") {
            dir = m_PreSearchDir;
            m_PreSearchDir.clear();
        }
        //更新当前目录
        m_CurDir=dir;

        //刷新列表
        m_mainDialog->slot_deleteAllFileInfo();
        slot_getCurDirFileList();

}

//客户端处理数据
void CKernel::slot_dealClientData(unsigned int lSendIP, char *buf, int nlen)
{
//    QString str=QString("来自服务端：%1").arg(QString::fromStdString(buf));
//    QMessageBox::about(NULL,"提示",str);  //about 阻塞的 模态窗口
    int type=*(int*)buf;
    qDebug()<<__func__;

    //通过协议头 拿到处理函数 并执行
    if(type>=_DEF_PACK_BASE && type<_DEF_PACK_BASE+_DEF_PACK_COUNT)
    {
        PFUN pf=NetMap(type);
        if(pf)
        {
            (this->*pf)(lSendIP, buf, nlen);
        }

    }

    //回收空间
    delete []buf;
}

void CKernel::slot_dealRegisterRS(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_REGISTER_RS* rs=(STRU_REGISTER_RS*)buf;
    //根据不同结果 有不同的提示
    switch(rs->result)
    {
    case tel_is_exist:
        QMessageBox::about(m_loginDialog,"提示","手机号已经存在，注册失败");
        break;
    case name_is_exist:
        QMessageBox::about(m_loginDialog,"提示","昵称已经存在，注册失败");
        break;
    case register_success:
        QMessageBox::about(m_loginDialog,"提示","注册成功");
        break;
    }
}

void CKernel::slot_dealUploadfileRS(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_UPLOAD_FILE_RS* rs=(STRU_UPLOAD_FILE_RS*)buf;
    //看结果是否为真
    if(!rs->result)
    {
        qDebug()<<"上传文件失败";
        return;
    }
    //为真
    //获取文件信息
    if(m_mapTimestampToFileInfo.count(rs->timestamp)==0)
    {
        qDebug()<<"not found";
        return;
    }
    FileInfo& info=m_mapTimestampToFileInfo[rs->timestamp];

    //更新文件id
    info.fileid=rs->fileid;
    //插入上传中信息到上传中的控件里 todo
    slot_writeUploadTask(info);
    m_mainDialog->slot_insertUploadFile(info);
    //发送文件块（内容）请求
    STRU_FILE_CONTENT_RQ rq;
    rq.fileid=rs->fileid;
    rq.timestamp=rs->timestamp;
    rq.userid=m_id;
    rq.len=fread(rq.content,1,_DEF_BUFFER,info.pFile);

    SendData((char*)&rq,sizeof(rq));

}

void CKernel::slot_dealFileContentRS(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_FILE_CONTENT_RS * rs=(STRU_FILE_CONTENT_RS*)buf;

    //找文件信息结构体
    if(m_mapTimestampToFileInfo.count(rs->timestamp)==0)
    {
        qDebug()<<"file not failed";
        return;
    }
    FileInfo& info=m_mapTimestampToFileInfo[rs->timestamp];
    //判断文件是否暂停：暂停则直接返回，不再发送下一个文件块，等待恢复后由续传协议继续
    if(info.isPause)
    {
        return;
    }
    //结果
    if(!rs->result)
    {
        //假跳回
        fseek(info.pFile,-1*(rs->len),SEEK_CUR);
    }
    else
    {
        //真pos +len
        info.pos+=rs->len;

        //更新上传进度todo
        //方案一：写信号槽 考虑多线程
        //方案二：直接调用 当前函数在主线程
        Q_EMIT SIG_updateUploadFileProgress(info.timestamp,info.pos);

        //判断是否结束
        if(info.pos>=info.size)
        {
            slot_deleteUploadTask(info);
            //是 关闭文件 回收 返回
            fclose(info.pFile);
            m_mapTimestampToFileInfo.erase(rs->timestamp);

            //刷新列表
            m_mainDialog->slot_deleteAllFileInfo();
            slot_getCurDirFileList();
            return;
        }
    }
    //发文件块
    STRU_FILE_CONTENT_RQ rq;
    rq.fileid=rs->fileid;
    rq.timestamp=rs->timestamp;
    rq.userid=m_id;
    rq.len=fread(rq.content,1,_DEF_BUFFER,info.pFile);
    SendData((char*)&rq,sizeof(rq));
}
//获取文件列表
void CKernel::slot_dealGetFileInfoRS(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_GET_FILE_INFO_RS * rs=(STRU_GET_FILE_INFO_RS *)buf;
    if(m_CurDir!=QString::fromStdString(rs->dir)) return;
    //刷新列表
    m_mainDialog->slot_deleteAllFileInfo();
    //获取元素插入控件  文件
    int count=rs->count;
    for(int i=0;i<count;i++)
    {
        FileInfo info;
        info.fileid=rs->fileinfo[i].fileid;
        info.type=QString::fromStdString(rs->fileinfo[i].fileType);
        info.name=QString::fromStdString(rs->fileinfo[i].name);
         qDebug() << "文件名字:" << info.name;
        info.size=rs->fileinfo[i].size;
        info.time=rs->fileinfo[i].time;


         m_mainDialog->slot_insertFileInfo(info);
    }
}

void CKernel::slot_dealFileHeadRQ(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
     STRU_FILE_HEADER_RQ* rq=(STRU_FILE_HEADER_RQ*)buf;
    //创建文件信息结构体 fuzhi
     //默认路径 m_sysPath(不含最后的’/‘)+dir+name
     //dir可能有很多层 需要循环创建目录
    QString tmpDir=QString::fromStdString(rq->dir);
    QStringList dirlst=tmpDir.split("/");   //分割函数 按照括号中的内容进行分割
    QString pathsum=m_sysPath;
    for(QString &node :dirlst)
    {
         if(!node.isEmpty())
         {
             pathsum+="/";
             pathsum+=node;

             QDir dir;
             if(!dir.exists(pathsum))
             {
                 dir.mkdir(pathsum);
             }
         }
    }
    FileInfo info;

    info.dir=QString::fromStdString(rq->dir);
    info.fileid=rq->fileid;
    info.md5=QString::fromStdString(rq->md5);
    info.name=QString::fromStdString(rq->fileName);
     qDebug()<<"文件名字"<<rq->fileName;
    info.absolutePath=QString("%1%2%3").arg(m_sysPath).arg(info.dir).arg(info.name);


    info.size=rq->size;
    info.time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    info.timestamp=rq->timestamp;
    info.type="file";
    //打开文件
    char pathbuf[1000]="";
    Utf8ToGB2312(pathbuf,1000,info.absolutePath);
    info.pFile=fopen(pathbuf,"wb");
    if(!info.pFile)
    {
        qDebug()<<"file open fail"<<pathbuf;
        qDebug() << "Error:" << strerror(errno);  // 显示具体错误原因
        return;
    }
    //todo 保存下载信息到控件
    slot_writeDownloadTask(info);
    m_mainDialog->slot_insertDownloadFile(info);
    //保存在map里面
    m_mapTimestampToFileInfo[rq->timestamp]=info;
    //写回复
     STRU_FILE_HEADER_RS rs;
     rs.fileid=rq->fileid;
     rs.result=1;
     rs.timestamp=rq->timestamp;
     rs.userid=m_id;
     SendData((char*)&rs,sizeof(rs));

}

void CKernel::slot_dealFileContentRQ(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_FILE_CONTENT_RQ* rq=(STRU_FILE_CONTENT_RQ*)buf;
    //拿到文件信息结构体
    if(m_mapTimestampToFileInfo.count(rq->timestamp)==0) return;
    FileInfo& info=m_mapTimestampToFileInfo[rq->timestamp];

    //判断文件是否暂停：暂停则直接返回，不写入也不回复，等待恢复后由续传协议继续
    if(info.isPause)
    {
        return;
    }
    STRU_FILE_CONTENT_RS rs;
    //写文件
    int len=fwrite(rq->content,1,rq->len,info.pFile);
    if(len!=rq->len)
    {
         //不成功跳回
        rs.result=0;
        fseek(info.pFile,-1*len,SEEK_CUR);
    }
    else
    {
        //成功移动pos指针
        rs.result=1;
        info.pos+=len;
        //更新进度todo
        Q_EMIT SIG_updateDownloadFileProgress(rq->timestamp,info.pos);
        //要看 有没有到文件末尾 是否接收
        if(info.pos>=info.size)
        {
            slot_deleteDownloadTask(info);
             //结束 关闭文件 回收
            fclose(info.pFile);
            m_mapTimestampToFileInfo.erase(rq->timestamp);
        }

    }
    //写回复
    rs.fileid=rq->fileid;
    rs.len=rq->len;
    rs.timestamp=rq->timestamp;
    rs.userid=m_id;
    //发送
    SendData((char*)&rs,sizeof(rs));

}

void CKernel::slot_dealAddFolderRS(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_ADD_FOLDER_RS * rs = (STRU_ADD_FOLDER_RS *)buf;

    //判断是否成功
    if( rs->result != 1 ) return;
    //先删除原来的
    m_mainDialog->slot_deleteAllFileInfo();
    //更新文件列表
    slot_getCurDirFileList();
}

void CKernel::slot_dealQuickUploadRS(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_QUICK_UPLOAD_RS* rs=(STRU_QUICK_UPLOAD_RS*)buf;
    //获取文件信息
    if(m_mapTimestampToFileInfo.count(rs->timestamp)==0 ) return;
    FileInfo & info=m_mapTimestampToFileInfo[rs->timestamp];
    //关闭文件
    if(info.pFile)
    {
        fclose(info.pFile);
    }
    //写入上传已完成信息
    m_mainDialog->slot_insertDownloadComplete(info);
    //发送刷新文件列表
    if(m_CurDir==info.dir){
        //刷新列表
        m_mainDialog->slot_deleteAllFileInfo();
        slot_getCurDirFileList();
    }
    //删除节点
    m_mapTimestampToFileInfo.erase(rs->timestamp);
}

void CKernel::slot_dealShareFileRS(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_SHARE_FILE_RS * rs=(STRU_SHARE_FILE_RS *)buf;
    if(rs->result!=1) return;

    //刷新 发送过去请求
    slot_getMyShare();
}

void CKernel::slot_dealMyShareRS(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_MY_SHARE_RS* rs=(STRU_MY_SHARE_RS*)buf;
    int count=rs->itemCount;
    //遍历文件信息，加载到控件上
    m_mainDialog->slot_deleteShareAllFileInfo();
    for(int i=0;i<count;i++)
    {
        m_mainDialog->slot_insertShareFileInfo(rs->items[i].name,rs->items[i].size,rs->items[i].time,rs->items[i].shareLink);
    }
}
void CKernel::slot_getMyShare()
{
    STRU_MY_SHARE_RQ rq;
    rq.userid=m_id;
    SendData((char*)&rq,sizeof(rq));

}

void CKernel::slot_dealGetShareRs(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_GET_SHARE_RS * rs = (STRU_GET_SHARE_RS *)buf;

    //根据结果
    if(rs->result == 0)
    {
    //错误返回提示
    QMessageBox::about( m_mainDialog , "提示", "获取分享失败");
    }
    else{
    //正确刷新
    if( QString::fromStdString( rs->dir ) == m_CurDir )
    {
        slot_getCurDirFileList();
    }
    }
}

void CKernel::slot_dealFolderHaedRq(unsigned int lSendIP, char *buf, int nlen)
{
    STRU_FOLDER_HEADER_RQ* rq = (STRU_FOLDER_HEADER_RQ*)buf;
    QString tmpDir=QString::fromStdString(rq->dir);
    QStringList dirList=tmpDir.split("/"); //分割函数
    QString pathsum=m_sysPath;
    for(QString & node :dirList)
    {
        if(!node.isEmpty())
        {
            pathsum+="/";
            pathsum+=node;

        }
        QDir dir;
        if (!dir.exists(pathsum)) {
            dir.mkdir(pathsum);
        }
    }


    // 创建最终的目标文件夹
    pathsum += "/";
    pathsum += QString::fromStdString(rq->fileName);

    QDir dir;
    if (!dir.exists(pathsum)) {
        dir.mkdir(pathsum);
    }
}

void CKernel::slot_dealDeleteFileRs(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_DELETE_FILE_RS* rs=(STRU_DELETE_FILE_RS*)buf;

    //看是否刷新
    if(rs->result==1)
    {
        if(QString::fromStdString(rs->dir)==m_CurDir)
        {
            m_mainDialog->slot_deleteAllFileInfo();
            slot_getCurDirFileList();
        }
    }
}

void CKernel::slot_dealContinueUploadRs(unsigned int lSendIP, char *buf, int nlen)
{
    //拆包
    STRU_CONTINUE_UPLOAD_RS* rs=(STRU_CONTINUE_UPLOAD_RS*)buf;
    //通过map拿到文件信息
    if(m_mapTimestampToFileInfo.count(rs->timestamp)==0) return;
    FileInfo& info=m_mapTimestampToFileInfo[rs->timestamp];
    //文件位置跳转 pos更新 界面显示 百分比更新
    info.pos=rs->pos;
    fseek(info.pFile,rs->pos,SEEK_SET); //从其实位置跳pos这么多

    m_mainDialog->slot_updateUploadFileProgress(info.timestamp,info.pos);
    //发送文件快请求
    STRU_FILE_CONTENT_RQ rq;
    rq.fileid=rs->fileid;
    rq.timestamp=rs->timestamp;
    rq.userid=m_id;
    rq.len=fread(rq.content,1,_DEF_BUFFER,info.pFile);
    SendData((char*)&rq,sizeof(rq));
}

void CKernel::slot_dealLoginRS(unsigned int lSendIP, char *buf, int nlen)
{
    qDebug()<<__func__;
    //拆包
    STRU_LOGIN_RS* rs=(STRU_LOGIN_RS*)buf;
    //根据不同结果 有不同的提示
    switch(rs->result)
    {
    case tel_not_exist:
        QMessageBox::about(m_loginDialog,"提示","手机号不存在，登录失败");
        break;
    case password_error:
        QMessageBox::about(m_loginDialog,"提示","密码错误，登录失败");
        break;
    case login_success:
        QMessageBox::about(m_loginDialog,"提示","登录成功");
        //前台
        m_loginDialog->hide();
        m_mainDialog->show();
        m_mainDialog->debugTableHeaders();

        //后台
        m_name=rs->name;
        m_id=rs->userid;
        m_mainDialog->slot_setInfo(m_name);

        //获取 根目录下面文件列表
        m_CurDir="/";
        slot_getCurDirFileList();

        //刷新 发送过去请求
        slot_getMyShare();
        InitDatabase(m_id);

        break;
    }

}
#ifdef USE_SERVER
//服务端处理数据
void CKernel::slot_dealServerData(unsigned int lSendIP, char *buf, int nlen)
{
    QString str=QString("来自客户端：%1").arg(QString::fromStdString(buf));
    QMessageBox::about(NULL,"提示",str);  //about 阻塞的 模态窗口

    m_tcpServer->SendData(lSendIP,buf,nlen);
    //回收空间
    delete []buf;
}
#endif

// 搜索文件请求
void CKernel::slot_searchFile(QString key)
{
    // 保存搜索前的目录
    m_PreSearchDir = m_CurDir;

    STRU_SEARCH_FILE_RQ rq;
    rq.userid = m_id;
    std::string strKey = key.toStdString();
    strcpy(rq.searchKey, strKey.c_str());
    SendData((char*)&rq, sizeof(rq));
}

// 搜索文件回复
void CKernel::slot_dealSearchFileRs(unsigned int lSendIP, char* buf, int nlen)
{
    STRU_SEARCH_FILE_RS* rs = (STRU_SEARCH_FILE_RS*)buf;
    if (rs->count == 0) {
        QMessageBox::information(m_mainDialog, "搜索", "未找到匹配的文件");
        return;
    }

    m_mainDialog->slot_deleteAllFileInfo();
    for (int i = 0; i < rs->count; i++) {
        FileInfo info;
        info.fileid = rs->fileinfo[i].fileid;
        info.name = QString::fromStdString(rs->fileinfo[i].name);
        info.size = rs->fileinfo[i].size;
        info.time = QString::fromStdString(rs->fileinfo[i].time);
        info.type = QString::fromStdString(rs->fileinfo[i].fileType);
        m_mainDialog->slot_insertFileInfo(info);
    }
}

// 回收站 - 获取列表
void CKernel::slot_getRecycle()
{
    STRU_GET_RECYCLE_RQ rq;
    rq.userid = m_id;
    SendData((char*)&rq, sizeof(rq));
}

void CKernel::slot_dealGetRecycleRs(unsigned int lSendIP, char* buf, int nlen)
{
    STRU_GET_RECYCLE_RS* rs = (STRU_GET_RECYCLE_RS*)buf;
    m_mainDialog->slot_deleteAllFileInfo();
    for (int i = 0; i < rs->count; i++) {
        FileInfo info;
        info.fileid = rs->fileinfo[i].fileid;
        info.name = QString::fromStdString(rs->fileinfo[i].name);
        info.size = rs->fileinfo[i].size;
        info.time = QString::fromStdString(rs->fileinfo[i].time);
        info.type = QString::fromStdString(rs->fileinfo[i].fileType);
        m_mainDialog->slot_insertFileInfo(info);
    }
}

// 回收站 - 恢复文件
void CKernel::slot_restoreFile(QVector<int> fileids)
{
    STRU_RESTORE_FILE_RQ rq;
    rq.userid = m_id;
    rq.fileCount = fileids.size();
    for (int i = 0; i < fileids.size() && i < _DEF_COUNT; i++) {
        rq.fileidArray[i] = fileids[i];
    }
    SendData((char*)&rq, sizeof(rq));
}

void CKernel::slot_dealRestoreFileRs(unsigned int lSendIP, char* buf, int nlen)
{
    STRU_RESTORE_FILE_RS* rs = (STRU_RESTORE_FILE_RS*)buf;
    if (rs->result) {
        QMessageBox::information(m_mainDialog, "恢复", "文件已恢复！");
        slot_getRecycle();
    }
}

// 收藏 - 添加
void CKernel::slot_addFavorite(QString dir, QString name, int fileid)
{
    qDebug() << "slot_addFavorite: dir=" << dir << " name=" << name << " fileid=" << fileid;
    STRU_ADD_FAVORITE_RQ rq;
    rq.userid = m_id;
    rq.fileid = fileid;
    strcpy(rq.dir, dir.toStdString().c_str());
    strcpy(rq.name, name.toStdString().c_str());
    SendData((char*)&rq, sizeof(rq));
}

void CKernel::slot_dealAddFavoriteRs(unsigned int lSendIP, char* buf, int nlen)
{
    STRU_ADD_FAVORITE_RS* rs = (STRU_ADD_FAVORITE_RS*)buf;
    if (rs->result) {
        QMessageBox::information(m_mainDialog, "收藏", "已添加到收藏！");
    } else {
        QMessageBox::information(m_mainDialog, "收藏", "收藏失败，可能已存在！");
    }
}

// 收藏 - 获取列表
void CKernel::slot_getFavorite()
{
    qDebug() << "slot_getFavorite: m_id=" << m_id;
    STRU_GET_FAVORITE_RQ rq;
    rq.userid = m_id;
    SendData((char*)&rq, sizeof(rq));
}

void CKernel::slot_dealGetFavoriteRs(unsigned int lSendIP, char* buf, int nlen)
{
    qDebug() << "slot_dealGetFavoriteRs: nlen=" << nlen;
    STRU_GET_FAVORITE_RS* rs = (STRU_GET_FAVORITE_RS*)buf;
    qDebug() << "slot_dealGetFavoriteRs: count=" << rs->count;
    m_mainDialog->slot_deleteAllFileInfo();
    for (int i = 0; i < rs->count; i++) {
        FileInfo info;
        info.fileid = rs->fileinfo[i].fileid;
        info.name = QString::fromStdString(rs->fileinfo[i].name);
        info.size = rs->fileinfo[i].size;
        info.time = QString::fromStdString(rs->fileinfo[i].time);
        info.type = QString::fromStdString(rs->fileinfo[i].fileType);
        m_mainDialog->slot_insertFileInfo(info);
    }
}

// 收藏 - 取消
void CKernel::slot_delFavorite(int fileid)
{
    STRU_DEL_FAVORITE_RQ rq;
    rq.userid = m_id;
    rq.fileid = fileid;
    SendData((char*)&rq, sizeof(rq));
}

void CKernel::slot_dealDelFavoriteRs(unsigned int lSendIP, char* buf, int nlen)
{
    STRU_DEL_FAVORITE_RS* rs = (STRU_DEL_FAVORITE_RS*)buf;
    if (rs->result) {
        QMessageBox::information(m_mainDialog, "收藏", "已取消收藏！");
        slot_getFavorite();
    }
}




//配置文件  在什么位置？与exe同级目录 思路根据目录查看文件是否存在，存在加载 不存在创建并且写入默认值
// .ini
//格式：[组名]
//key=value
//例如
//[net]
//ip=192.168.3.159
//port=8080
#include<QDir>
#include<QDebug>
void CKernel::InitDatabase(int id)
{
    m_sql=new CSqlite;
    //首先 找到exe 去同级目录 /database/id.db
    QString path=QCoreApplication::applicationDirPath()+"/database/";
    //先看路径是否存在 要不要创建
    QDir dir;
    if(!dir.exists(path))
    {
        dir.mkdir(path);
    }
    path=path+QString("%1.db").arg(id);
    //查看是不是有这个文件
    QFileInfo info(path);
    if(info.exists())
    {
        //有直接加载
        //连接
        m_sql->ConnectSql(path);

        QList<FileInfo> uploadTaskList;
        QList<FileInfo> downloadTaskList;

        slot_getUploadTask(uploadTaskList);
        slot_getDownloadTask(downloadTaskList);

        // 加载上传任务
        for (FileInfo &info : uploadTaskList) {
            // 如果这个文件不存在则跳过
            QFileInfo fi(info.absolutePath);
            if (!fi.exists()) continue;

            // 修改任务的初始状态为暂停
            info.isPause = 1;
            m_mainDialog->slot_insertUploadFile(info);

            //上传续传 控件 上看不到进行了多少
            //todo 获取当前位置

            //同步控件的位置
        }

        // 加载下载任务
        for (FileInfo &info : downloadTaskList) {
            // 如果这个文件不存在则跳过
            QFileInfo fi(info.absolutePath);
            if (!fi.exists()) continue;

            // 修改任务的初始状态为暂停
            info.isPause = 1;
            // 进行到多少 可以知道
            info.pos = fi.size();

            m_mainDialog->slot_insertDownloadFile(info);

            // 控件同步位置
            m_mainDialog->slot_updateDownloadFileProgress(info.timestamp, fi.size());
        }


    }
    else
    {
        //没有 创建表
        QFile file(path);
        if(!file.open(QIODevice::WriteOnly)) return;
        file.close();
        //连接
        m_sql->ConnectSql(path);
        //创建表
        QString sqlbuf="create table t_upload(timestamp int,f_id int,f_name varchar(260),f_dir varchar(260),f_time varchar(60),f_size int,f_md5 varchar(60),f_type varchar(60),f_absolutePath varchar(260));";
        m_sql->UpdateSql(sqlbuf);
        sqlbuf="create table t_download(timestamp int,f_id int,f_name varchar(260),f_dir varchar(260),f_time varchar(60),f_size int,f_md5 varchar(60),f_type varchar(60),f_absolutePath varchar(260));";
        m_sql->UpdateSql(sqlbuf);
    }
}

void CKernel::slot_writeUploadTask(FileInfo &info)
{
    QString sqlbuf = QString("insert into t_upload values( %1 , %2 , '%3', '%4', '%5', %6 , '%7', '%8', '%9' );")
            .arg(info.timestamp)
            .arg(info.fileid)
            .arg(info.name)
            .arg(info.dir)
            .arg(info.time)
            .arg(info.size)
            .arg(info.md5)
            .arg(info.type)
            .arg(info.absolutePath);

        m_sql->UpdateSql(sqlbuf);
}

void CKernel::slot_writeDownloadTask(FileInfo &info)
{
    QString sqlbuf = QString("insert into t_download values( %1 , %2 , '%3', '%4', '%5', %6 , '%7', '%8', '%9' );")
            .arg(info.timestamp)
            .arg(info.fileid)
            .arg(info.name)
            .arg(info.dir)
            .arg(info.time)
            .arg(info.size)
            .arg(info.md5)
            .arg(info.type)
            .arg(info.absolutePath);

        m_sql->UpdateSql(sqlbuf);
}

void CKernel::slot_deleteUploadTask(FileInfo &info)
{
    QString sqlbuf = QString("delete from t_upload where timestamp = %1 and f_absolutePath = '%2'; ")
           .arg(info.timestamp)
           .arg(info.absolutePath);

       m_sql->UpdateSql(sqlbuf);
}

void CKernel::slot_deleteDownloadTask(FileInfo &info)
{
    QString sqlbuf = QString("delete from t_download where timestamp = %1 and f_absolutePath = '%2'; ")
           .arg(info.timestamp)
           .arg(info.absolutePath);

       m_sql->UpdateSql(sqlbuf);
}

void CKernel::slot_getUploadTask(QList<FileInfo> &infolist)
{
    //获取所有的任务
        QString sqlbuf = "select * from t_upload;";
        QStringList lst;
        m_sql->SelectSql(sqlbuf, 9, lst);
        /*timestamp int ,f_id int ,f_name varchar(260),f_dir varchar(260),f_time varchar(60),f_size int,f_md5 varchar(60),f_type varchar(60),f_absolutePath varchar(260)*/
        while(lst.size() != 0){
            FileInfo info;
            info.timestamp = QString(lst.front()).toInt(); lst.pop_front();
            info.fileid = QString(lst.front()).toInt(); lst.pop_front();
            info.name = lst.front(); lst.pop_front();
            info.dir = lst.front(); lst.pop_front();
            info.time = lst.front(); lst.pop_front();
            info.size = QString(lst.front()).toInt(); lst.pop_front();
            info.md5 = lst.front(); lst.pop_front();
            info.type = lst.front(); lst.pop_front();
            info.absolutePath = lst.front(); lst.pop_front();

            infolist.push_back(info);
        }
}

void CKernel::slot_getDownloadTask(QList<FileInfo> &infolist)
{
    //获取所有的任务
        QString sqlbuf = "select * from t_download;";
        QStringList lst;
        m_sql->SelectSql(sqlbuf, 9, lst);
        /*timestamp int ,f_id int ,f_name varchar(260),f_dir varchar(260),f_time varchar(60),f_size int,f_md5 varchar(60),f_type varchar(60),f_absolutePath varchar(260)*/
        while(lst.size() != 0){
            FileInfo info;
            info.timestamp = QString(lst.front()).toInt(); lst.pop_front();
            info.fileid = QString(lst.front()).toInt(); lst.pop_front();
            info.name = lst.front(); lst.pop_front();
            info.dir = lst.front(); lst.pop_front();
            info.time = lst.front(); lst.pop_front();
            info.size = QString(lst.front()).toInt(); lst.pop_front();
            info.md5 = lst.front(); lst.pop_front();
            info.type = lst.front(); lst.pop_front();
            info.absolutePath = lst.front(); lst.pop_front();

            infolist.push_back(info);
        }
}

/*
    int fileid;
    QString name;
    QString dir;  //网盘目录
    QString time;
    int size;  //int 32位 最大值 2GB------假定网盘 文件斗志2GB在最大的
    QString md5;
    QString type;
    QString absolutePath;  //文件本地的绝对路径

    int pos; //上传或下载到什么位置
    int timestamp;   //时间戳
//正在下载表

create table t_download
(
timestamp int,
f_id int,
f_name varchar(260),
f_dir varchar(260),
f_time varchar(260),
f_time varchar(60),
f_size int,
f_md5 varchar(60),
f_type varchar(60),
f_absolutePath varchar(260)
);

//正在上传表
create table t_upload
(
timestamp int,
f_id int,
f_name varchar(260),
f_dir varchar(260),
f_time varchar(260),
f_time varchar(60),
f_size int,
f_md5 varchar(60),
f_type varchar(60),
f_absolutePath varchar(260)
);

*/
