#include "clogic.h"

void CLogic::setNetPackMap()
{
    NetPackMap(_DEF_PACK_REGISTER_RQ)    = &CLogic::RegisterRq;
    NetPackMap(_DEF_PACK_LOGIN_RQ)       = &CLogic::LoginRq;
    NetPackMap(_DEF_PACK_UPLOAD_FILE_RQ) = &CLogic::UploadFileRq;
    NetPackMap(_DEF_PACK_FILE_CONTENT_RQ) = &CLogic::FileContentRq;
    NetPackMap(_DEF_PACK_GET_FILE_INFO_RQ) = &CLogic::GetFileInfoRq;
    NetPackMap(_DEF_PACK_DOWNLOAD_FILE_RQ) = &CLogic::DownloadFileRq;
    NetPackMap(_DEF_PACK_DOWNLOAD_FOLDER_RQ) = &CLogic::DownloadFolderRq;
    NetPackMap(_DEF_PACK_FILE_HEADER_RS) = &CLogic::FileHeadRs;
    NetPackMap(_DEF_PACK_FILE_CONTENT_RS) = &CLogic::FileContentRs;
    NetPackMap(_DEF_PACK_ADD_FOLDER_RQ) = &CLogic::AddFolderRq;
    NetPackMap(_DEF_PACK_SHARE_FILE_RQ) = &CLogic::shareFileRq;
    NetPackMap(_DEF_PACK_MY_SHARE_RQ)=&CLogic::MyShareRq;
    NetPackMap(_DEF_PACK_GET_SHARE_RQ)=&CLogic::GetShareRq;
    NetPackMap(_DEF_PACK_DELETE_FILE_RQ)=&CLogic::DeleteFileRq;
    NetPackMap(_DEF_PACK_CONTINUE_DOWNLOAD_RQ)=&CLogic::ContinueDownloadRq;
    NetPackMap(_DEF_PACK_CONTINUE_UPLOAD_RQ)=&CLogic::ContinueUploadRq;
    NetPackMap(_DEF_PACK_SEARCH_FILE_RQ) =&CLogic::SearchFileRq;
    NetPackMap(_DEF_PACK_GET_RECYCLE_RQ) =&CLogic::GetRecycleRq;
    NetPackMap(_DEF_PACK_RESTORE_FILE_RQ) =&CLogic::RestoreFileRq;
    NetPackMap(_DEF_PACK_ADD_FAVORITE_RQ) =&CLogic::AddFavoriteRq;
    NetPackMap(_DEF_PACK_GET_FAVORITE_RQ) =&CLogic::GetFavoriteRq;
    NetPackMap(_DEF_PACK_DEL_FAVORITE_RQ) =&CLogic::DelFavoriteRq;
}

#define _DEF_COUT_FUNC_    cout << "clientfd:"<< clientfd << __func__ << endl;
#define DEF_PATH "/home/zlx/NetDisk/"

// 递归创建目录
static void mkdir_p(const char* path) {
    char tmp[1000];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0777);
            *p = '/';
        }
    }
    mkdir(tmp, 0777);
}
//注册
void CLogic::RegisterRq(sock_fd clientfd,char* szbuf,int nlen)
{
    //cout << "clientfd:"<< clientfd << __func__ << endl;
    _DEF_COUT_FUNC_
     //拆包 tel password name
     STRU_REGISTER_RQ* rq=( STRU_REGISTER_RQ*)szbuf;
     STRU_REGISTER_RS rs;
     //根据tel 查看手机号是否存在
     char sqlstr[1000]="";
     sprintf(sqlstr,"select u_tel from t_user where u_tel='%s';",rq->tel);
     list<string> lstRes;
     bool res=m_sql->SelectMysql(sqlstr,1,lstRes);
     if(!res)
     {
         std::cout<<"select failed"<<sqlstr<<std::endl;
     }
     if(lstRes.size()!=0)
     {
          //存在返回
         rs.result=tel_is_exist;
     }
     else
     {
         //不存在查看昵称 是否存在
         sprintf(sqlstr,"select u_tel from t_user where u_name='%s';",rq->name);
         list<string> lstRes;
         bool res=m_sql->SelectMysql(sqlstr,1,lstRes);
         if(!res)
         {
             std::cout<<"select failed"<<sqlstr<<std::endl;
         }

         if(lstRes.size()!=0)
         {
              //存在返回
             rs.result=name_is_exist;
         }
         else
         {
             //不存在
             rs.result=register_success;
             //注册成功，写入信息
             sprintf(sqlstr,"insert into t_user(u_tel,u_password,u_name) value('%s','%s','%s')",rq->tel,rq->password,rq->name);
             m_sql->UpdataMysql(sqlstr);

             //取出该人id
              sprintf(sqlstr,"select u_id from t_user where u_tel='%s' and u_password='%s';",rq->tel,rq->password);
              lstRes.clear();
              bool res=m_sql->SelectMysql(sqlstr,1,lstRes);
              if(!res)
              {
                  std::cout<<"select failed"<<sqlstr<<std::endl;
              }
              if(lstRes.size()!=0)
              {
                   int id =stoi(lstRes.front());
                   lstRes.pop_front();
                   //网盘特有：创建该人对应的目录 id命名
                   //默认路径 DEF_PATH /home/colin/NetDisk
                   char pathbuf[_MAX_PATH_SIZE]="";
                   sprintf(pathbuf,"%s%d/",DEF_PATH,id);

                   //创建路径
                   umask(0); //设置权限掩码
                   mkdir(pathbuf,0777);//创建目录 并设置权限0777
              }


         }

     }

   SendData(clientfd,(char*)&rs,sizeof(rs));
}

//登录
void CLogic::LoginRq(sock_fd clientfd ,char* szbuf,int nlen)
{
//    cout << "clientfd:"<< clientfd << __func__ << endl;
    _DEF_COUT_FUNC_

    //拆包 tel password
            STRU_LOGIN_RQ* rq=( STRU_LOGIN_RQ*)szbuf;
            STRU_LOGIN_RS rs;
             //根据tel 查 id password name
            char sqlstr[1000]="";
            sprintf(sqlstr,"select u_id,u_password,u_name from t_user where u_tel='%s';",rq->tel);
            list<string> lstRes;
            bool res=m_sql->SelectMysql(sqlstr,3,lstRes);
            if(!res)
            {
                std::cout<<"select failed"<<sqlstr<<std::endl;
            }
            if(lstRes.size()==0)
            {
                  //不存在，返回
                rs.result=tel_not_exist;
            }
            else
            {
                //存在
                int id=stoi(lstRes.front());
                lstRes.pop_front();
                string strPassword=lstRes.front();
                lstRes.pop_front();
                string strName=lstRes.front();
                lstRes.pop_front();
                //密码是否一致
               if(strcmp(strPassword.c_str(),rq->password)!=0)
               {
                    //不一致返回
                   rs.result=password_error;
               }
               else
               {
                    //一致
                   rs.result=login_success;
                   rs.userid=id;
                   strcpy(rs.name,strName.c_str());
                   //需要创建用户身份结构，保留用户信息
                   //
                   UserInfo *info=nullptr;
                   if(!m_mapIDToUserInfo.find(id,info))
                   {
                       info=new UserInfo;
                   }
                   else
                   {
                        //如果存在考虑下线
                   }
                   info->name=strName;
                   info->clientfd=clientfd;
                   info->userid=id;
                   m_mapIDToUserInfo.insert(id,info);
               }

            }

            SendData(clientfd,(char*)&rs,sizeof(rs));
}

void CLogic::UploadFileRq(sock_fd clientfd, char *szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    //拆包
    STRU_UPLOAD_FILE_RQ* rq=(STRU_UPLOAD_FILE_RQ*)szbuf;
    //查看是否秒传
    {
        //判断文件是否上传过
        //根据md5 state=1; 查数据库 得到id
        char sqlbuf[1000]="";
        sprintf(sqlbuf,"select f_id from t_file where f_MD5='%s' and f_state=1;",rq->md5);
        list<string> lstRes;
        bool res=m_sql->SelectMysql(sqlbuf,1,lstRes);
        if(!res)
        {
            cout<<"select fail:"<<sqlbuf<<endl;
            return;
        }
        if(lstRes.size()>0)
        {
            int fileid=stoi(lstRes.front());lstRes.pop_front();
            //（如果state=0, 怎么处理？ 客户端写拒绝 或者挂起请求）
                //已经上传 查到了
                //写入用户文件关系 由于触发器 文件引用计数+1
            sprintf(sqlbuf,"insert into t_user_file ( u_id , f_id , f_dir , f_name , f_uploadtime ) values( %d , %d , '%s' , '%s' , '%s' ); ",rq->userid,fileid,rq->dir,rq->fileName,rq->time);
            res=m_sql->UpdataMysql(sqlbuf);
            if(!res)
            {
                printf("update fail:%s\n",sqlbuf);
                return;
            }
                //写回复包 客户端收到之后 就更新列表
            STRU_QUICK_UPLOAD_RS rs;
            rs.result=1;
            rs.timestamp=rq->timestamp;
            rs.userid=rq->userid;
                //发送
            SendData(clientfd,(char*)&rs,sizeof(rs));
                //返回
            return;
        }

    }
    //不是秒传
    //文件信息创建 打开文件
    FileInfo* info=new FileInfo;
    char strpath[1000]="";
    sprintf(strpath,"%s%d%s%s",DEF_PATH,rq->userid,rq->dir,rq->md5); //文件路径是由DEF_PATH+userid+dir+name-md
    info->absolutePath=strpath;  //通过这个写数据库 打开文件 名字 md5
    //文件名字是md5
    info->dir=rq->dir;
    info->fid;
    info->md5=rq->md5;
    info->name=rq->fileName;
    info->size=rq->size;  //fopen fseek ftell
    info->time=rq->time;
    info->type=rq->type;
    // 确保父目录存在
    {
        char dirpath[1000] = "";
        strncpy(dirpath, strpath, sizeof(dirpath) - 1);
        char* lastSlash = strrchr(dirpath, '/');
        if (lastSlash) {
            *lastSlash = '\0';
            mkdir_p(dirpath);
        }
    }
    info->fileFd=open(strpath,O_CREAT | O_WRONLY |O_TRUNC,00777);
    if(info->fileFd<0)
    {
        std::cout<<"file open failed"<<std::endl;
        return;
    }

    //map存储文件信息
    int64_t user_time=rq->userid*getNumber()+rq->timestamp;
    m_mapTimestampToFileInfo.insert(user_time,info);
    //数据库记录
    //查询文件信息引用计数是0
    char sqlbuf[1000]="";
     sprintf(sqlbuf,"insert into t_file ( f_size , f_path , f_MD5 , f_count , f_state , f_type ) values ( %d ,'%s' , '%s' ,0 , 0 , 'file'); ",rq->size,strpath,rq->md5);
    bool res=m_sql->UpdataMysql(sqlbuf);
    if(!res)
    {
        printf("update fail:%s\n",sqlbuf);
    }
    //cha文件id
     sprintf(sqlbuf,"select f_id from t_file where f_path='%s' and f_MD5='%s';",strpath,rq->md5);
     list<string> lstRes;
     res=m_sql->SelectMysql(sqlbuf,1,lstRes);
     if(!res)
     {
         printf("Select fail:%s\n",sqlbuf);
         return;
     }
     if(lstRes.size()>0)
     {
         info->fid=stoi(lstRes.front());
     }
     lstRes.clear();
    //插入用户文件关系（由于触发器 引用计数-> 1 ，状态是0--上传结束为1）
    sprintf(sqlbuf,"insert into t_user_file ( u_id , f_id , f_dir , f_name , f_uploadtime ) values( %d , %d , '%s' , '%s' , '%s' ); ",rq->userid,info->fid,rq->dir,rq->fileName,rq->time);
    res=m_sql->UpdataMysql(sqlbuf);
    if(!res)
    {
        printf("update fail:%s\n",sqlbuf);
        return;
    }
    //写回复包
    STRU_UPLOAD_FILE_RS rs;
    rs.fileid=info->fid;
    rs.result=1;
    rs.timestamp=rq->timestamp;
    rs.userid=rq->userid;
    SendData(clientfd,(char*)&rs,sizeof(rs));

}

void CLogic::FileContentRq(sock_fd clientfd, char *szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    //拆包
    STRU_FILE_CONTENT_RQ* rq=(STRU_FILE_CONTENT_RQ*)szbuf;
    //获取文件信息
    int64_t user_time=rq->userid*getNumber()+rq->timestamp;
    FileInfo* info=nullptr;
    if(!m_mapTimestampToFileInfo.find(user_time,info))
    {
        cout<<"file not found"<<endl;
        return;
    }
    STRU_FILE_CONTENT_RS rs;

    //写入
    int len=write(info->fileFd,rq->content,rq->len);
    if(len!=rq->len)
    {
         //失败 跳回读取之前
        rs.result=0;
        lseek(info->fileFd,-1*len,SEEK_CUR);
    }
    else
    {
        //成功pos更新位置
        rs.result=1;
        info->pos+=len;
        //看是否到达末尾
        if(info->pos >= info->size)
        {
            //是关闭文件
            close(info->fileFd);
           //回收map节点
            m_mapTimestampToFileInfo.erase(user_time);
            delete info;
            info=nullptr;

            //更新数据库 吧文件信息的状态更新为1 表示已完成
            char sqlbuf[1000]="";
            sprintf(sqlbuf,"update t_file set f_state=1 where f_id=%d;",rq->fileid);
            bool res=m_sql->UpdataMysql(sqlbuf);
            if(!res)
            {
                cout<<"Update fail:"<<sqlbuf<<endl;
                return;
            }
        }


    }
    
    //返回结果
    rs.fileid=rq->fileid;
    rs.len=rq->len;
    rs.timestamp=rq->timestamp;
    rs.userid=rq->userid;
    SendData(clientfd,(char*)&rs,sizeof(rs));
}

void CLogic::GetFileInfoRq(sock_fd clientfd, char *szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    //拆包
    STRU_GET_FILE_INFO_RQ *rq=(STRU_GET_FILE_INFO_RQ *)szbuf;
    //根据id dir查表 获取文件信息
    cout << "用户ID: " << rq->userid << " 请求目录: " << rq->dir << endl;
    char sqlbuf[1000]="";
    sprintf(sqlbuf,"select f_id,f_name,f_size,f_uploadtime,f_type from user_file_info where u_id =%d and f_dir='%s' and f_state=1",rq->userid,rq->dir);
    list<string> lstRes;
    bool res=m_sql->SelectMysql(sqlbuf,5,lstRes);
    if(!res){
        cout<<"select fail:"<<sqlbuf;
        return;
    }
    if(lstRes.size()==0) return;

    //写回复包
    int count=lstRes.size()/5;
    int packlen=sizeof(STRU_GET_FILE_INFO_RS)+count*sizeof(STRU_FILE_INFO);
    STRU_GET_FILE_INFO_RS* rs=(STRU_GET_FILE_INFO_RS*)malloc(packlen);
    rs->init();
    rs->count=count;
    strcpy(rs->dir,rq->dir);
    for(int i=0;i<count;i++)
    {
        int f_id=stoi(lstRes.front()); lstRes.pop_front();
        string name=lstRes.front(); lstRes.pop_front();
        cout << "SQL查询结果名字: " << name << endl;
        int f_size=stoi(lstRes.front()); lstRes.pop_front();
        string time=lstRes.front(); lstRes.pop_front();
        string f_type=lstRes.front(); lstRes.pop_front();

        rs->fileinfo[i].fileid=f_id;
        strcpy(rs->fileinfo[i].fileType,f_type.c_str());
        strcpy(rs->fileinfo[i].name,name.c_str());
        strcpy(rs->fileinfo[i].time,time.c_str());
        rs->fileinfo[i].size=f_size;
    }
    //发
    SendData(clientfd,(char*)rs,packlen);
    free(rs);
}

void CLogic::DownloadFileRq(sock_fd clientfd, char *szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    //拆包
    STRU_DOWNLOAD_FILE_RQ* rq=(STRU_DOWNLOAD_FILE_RQ*)szbuf;
    //查数据库 f_name,f_path,F_MD5,f_size
    char sqlbuf[1000]="";
    sprintf(sqlbuf,"select f_name,f_path,f_MD5,f_size from user_file_info where u_id=%d and f_dir='%s' and f_id=%d",rq->userid,rq->dir,rq->fileid);
    list<string> lstRes;
    bool res=m_sql->SelectMysql(sqlbuf,4,lstRes);
    if(!res)
    {
        cout<<"select fail:"<<sqlbuf<<endl;
        return;
    }
    if(lstRes.size()==0)//没有返回
    {
        return;
    }
    string strName=lstRes.front(); lstRes.pop_front();
    string strPath=lstRes.front(); lstRes.pop_front();
    string strMD5=lstRes.front(); lstRes.pop_front();
    int size=stoi(lstRes.front());lstRes.pop_front();
    //有写文件信息
    FileInfo* info=new FileInfo;
    info->absolutePath=strPath;
    info->fid=rq->fileid;
    info->size=size;
    info->dir=rq->dir;

    info->md5=strMD5;
    info->name=strName;
    cout<<"服务端"<<strName<<endl;
    info->type="file";
    info->fileFd=open(info->absolutePath.c_str(),O_RDONLY);
    if(info->fileFd<=0)
    {
        cout<<"file open fail"<<endl;
        return;
    }

    //key取出来
    int64_t user_time=rq->userid*getNumber()+rq->timestamp;
    //存在map中
    m_mapTimestampToFileInfo.insert(user_time,info);
    //发送文件头请求
    STRU_FILE_HEADER_RQ headrq;
    strcpy(headrq.dir,rq->dir);
    headrq.fileid=rq->fileid;
    strcpy(headrq.fileName,info->name.c_str());
    strcpy(headrq.md5,info->md5.c_str());
    strcpy(headrq.fileType,"file");
    headrq.size=info->size;
    headrq.timestamp=rq->timestamp;
    SendData(clientfd,(char*)&headrq,sizeof(headrq));

}

void CLogic::DownloadFolderRq(sock_fd clientfd, char *szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
        STRU_DOWNLOAD_FOLDER_RQ* rq = (STRU_DOWNLOAD_FOLDER_RQ*)szbuf;
        // 查数据库表 拿到信息 查的属性：
        // f_id, f_name, f_path, f_MD5, f_size, f_dir, f_type
        cout<<rq->userid<<" "<< rq->dir<<" "<<rq->fileid<<endl;
        char sqlbuf[1000] = "";
        sprintf(sqlbuf, "select f_type,f_id, f_name, f_path, f_MD5, f_size, f_dir from user_file_info where u_id = %d and f_dir = '%s' and f_id = %d;",
                rq->userid, rq->dir, rq->fileid);

        list<string> lstRes;
        bool res = m_sql->SelectMysql(sqlbuf, 7, lstRes);
        if (!res) {
            cout << "select fail:" << sqlbuf << endl;
            return;
        }
        if (lstRes.size() == 0) return;

        string type = lstRes.front(); lstRes.pop_front();
        int timestamp = rq->timestamp;
        // 下载文件夹
        DownloadFolder(rq->userid, timestamp, clientfd, lstRes);
}

void CLogic::DownloadFile(int userId, int &timestamp, sock_fd clientfd, list<string> &lstRes)
{
    int fileid = stoi(lstRes.front()); lstRes.pop_front();
    string strName = lstRes.front(); lstRes.pop_front();
    string strPath = lstRes.front(); lstRes.pop_front();
    string strMD5 = lstRes.front(); lstRes.pop_front();
    int size = stoi(lstRes.front()); lstRes.pop_front();
    string dir = lstRes.front(); lstRes.pop_front();
    //string type = lstRes.front(); lstRes.pop_front();

    // 用写文件包
    FileInfo* info = new FileInfo;
    info->absolutePath = strPath;
    info->dir = dir;
    info->fid = fileid;

    info->md5 = strMD5;
    info->name = strName;
    info->size = size;
    info->type = "file";

    info->fileFd = open(info->absolutePath.c_str(), O_RDONLY);
    if (info->fileFd <= 0) {
        cout << "file open fail" << endl;
        return;
    }

    // key求出来
    int64_t user_time = userId*getNumber() + (++timestamp);
    // 存到map里面
    m_mapTimestampToFileInfo.insert(user_time, info);

    // 定义结构体
    STRU_FILE_HEADER_RQ headrq;
    strcpy(headrq.dir, dir.c_str());
    headrq.fileid = fileid;
    strcpy(headrq.fileName, info->name.c_str());
    strcpy(headrq.md5, info->md5.c_str());
    strcpy(headrq.fileType, "file");
    headrq.size = info->size;
    headrq.timestamp = timestamp;

    SendData(clientfd, (char*)&headrq, sizeof(headrq));

}
void CLogic::DownloadFolder(int userId, int& timestamp, sock_fd clientfd, list<string>& lstRes)
{
    // f_id, f_name, f_path, f_MD5, f_size, f_dir, f_type  
    int fileid = stoi(lstRes.front()); lstRes.pop_front();
    string strName = lstRes.front(); lstRes.pop_front();
    string strPath = lstRes.front(); lstRes.pop_front();
    string strMD5 = lstRes.front(); lstRes.pop_front();
    int size = stoi(lstRes.front()); lstRes.pop_front();
    string dir = lstRes.front(); lstRes.pop_front();

    // 发送创建文件夹请求
    STRU_FOLDER_HEADER_RQ rq;
    rq.timestamp = ++timestamp; // 时间戳处理
    strcpy(rq.dir, dir.c_str());
    rq.fileid = fileid;
    strcpy(rq.fileName, strName.c_str());
    SendData(clientfd, (char*)&rq, sizeof(rq));
    // 拼接路径
    string newdir = dir + strName + "/";

    // 查询 newdir userid 所有文件信息(包含type) 列表 3个文件 21项
    char sqlbuf[1000] = "";
    sprintf(sqlbuf, "select f_type, f_id, f_name, f_path, f_MD5, f_size, f_dir from user_file_info where u_id = %d and f_dir = '%s'",userId, newdir.c_str());

    list<string> newlstRes;
    bool res = m_sql->SelectMysql(sqlbuf, 7, newlstRes);
    if (!res) {
        cout << "select fail:" << sqlbuf << endl;
        return;
    }

    while (newlstRes.size() != 0) {
        string type = newlstRes.front(); newlstRes.pop_front();
        if (type == "file") {
            // 如果是文件 下载文件流程
           DownloadFile(userId, timestamp, clientfd, newlstRes);
        } else {
            // 如果是文件夹 递归
            DownloadFolder(userId, timestamp, clientfd, newlstRes);
        }
    }
}
void CLogic::FileHeadRs(sock_fd clientfd, char *szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    //拆包
    STRU_FILE_HEADER_RS* rs=(STRU_FILE_HEADER_RS*)szbuf;
    //拿到文件信息
    int64_t user_time=rs->userid*getNumber()+rs->timestamp;
    FileInfo* info=nullptr;
    if(!m_mapTimestampToFileInfo.find(user_time,info)) return;
    //读文件
    STRU_FILE_CONTENT_RQ rq;
    //发送文件内容请求
    rq.len=read(info->fileFd,rq.content,_DEF_BUFFER);
    if(rq.len<0)
    {
        perror("read fail");
        return;
    }
    rq.fileid=rs->fileid;
    rq.timestamp=rs->timestamp;
    rq.userid=rs->userid;
    SendData(clientfd,(char*)&rq,sizeof(rq));

}

void CLogic::FileContentRs(sock_fd clientfd, char *szbuf, int nlen)
{
     _DEF_COUT_FUNC_;
    //拆包
    STRU_FILE_CONTENT_RS* rs=(STRU_FILE_CONTENT_RS*)szbuf;
    //文件信息结构
    //拿到文件信息结构体
    int64_t user_time=rs->userid*getNumber()+rs->timestamp;
    FileInfo* info=nullptr;
    if(!m_mapTimestampToFileInfo.find(user_time,info)) return;
        //panduan是否成功
    if(rs->result==0)
    {
        lseek(info->fileFd,-1*(rs->len),SEEK_CUR);
    }
    else
    {
        info->pos+=rs->len;
        if(info->pos>=info->size)
        {
             //结束 关闭文件 回收
            close(info->fileFd);
            m_mapTimestampToFileInfo.erase(user_time);
            delete info;
            info=nullptr;
            return;
        }
    }
    //
    STRU_FILE_CONTENT_RQ rq;
    rq.len=read(info->fileFd,rq.content,_DEF_BUFFER);
    if(rq.len==0) return;
    if(rq.len<0)
    {
        perror("read fail");
        return;
    }
    rq.fileid=rs->fileid;
    rq.timestamp=rs->timestamp;
    rq.userid=rs->userid;
    SendData(clientfd,(char*)&rq,sizeof(rq));
}

void CLogic::AddFolderRq(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
    STRU_ADD_FOLDER_RQ * rq = (STRU_ADD_FOLDER_RQ *)szbuf;

    //数据库写表 插入文件信息
    //f_size , f_path , f_count , f_MDS , f_state , f_type
    char pathbuf[1000] = "";
    sprintf(pathbuf , "%s%d%s%s", DEF_PATH , rq->userid , rq->dir , rq->fileName );
    ///root/id/dir/name

    char sqlbuf[1000] = "";
    sprintf( sqlbuf , "insert into t_file ( f_size , f_path , f_count , f_MD5 , f_state , f_type ) values ( 0 , '%s', 0, '?' , 1 , 'folder');", pathbuf );

    bool res = m_sql->UpdataMysql( sqlbuf );
    if(!res){
        cout << "update fail:"<<sqlbuf <<endl;
        return;
    }

    //查询 id
    sprintf( sqlbuf , "select f_id from t_file where f_path = '%s';", pathbuf);
    list<string > lstRes;
    res = m_sql->SelectMysql( sqlbuf , 1 , lstRes );
    if(!res ){
        cout << "SelectMysql fail:"<<sqlbuf <<endl;
        return;
    }
    if( lstRes.size() == 0) return;

    int id = stoi( lstRes.front() ); lstRes.pop_front();

    //写入用户文件关系 -- 隐藏 触发器引用计数会+1
    //u_id，f_id，f_dir，f_name，f_uploadtime
    sprintf( sqlbuf , "insert into t_user_file (u_id , f_id, f_dir, f_name, f_uploadtime ) values ( %d , %d , '%s' , '%s' , '%s');", rq->userid, id, rq->dir, rq->fileName, rq->time );
    res = m_sql->UpdataMysql( sqlbuf );
    if(!res){
        cout << "update fail:"<<sqlbuf <<endl;
        return;
    }
    //创建目录
    umask(0);

    mkdir(pathbuf, 0777);

    //写回复
    STRU_ADD_FOLDER_RS rs;
    rs.result = 1;
    rs.timestamp = rq->timestamp;
    rs.userid = rq->userid;
    //发送
    SendData( clientfd , (char*)&rs ,sizeof(rs ) );
}

void CLogic::shareFileRq(sock_fd clientfd, char *szbuf, int nlen)
{
    SRTU_SHARE_FILE_RQ * rq=(SRTU_SHARE_FILE_RQ*)szbuf;
    //随机生成分享链接
    //分享吗规则
    int link=0;
    do{
        link=1+random()%9;
        link*=100000000;
        link+=random()%100000000;

        char sqlbuf[1000]="";
        sprintf(sqlbuf,"select s_link from t_user_file where s_link=%d;",link);
        list<string> lstRes;
        bool res=m_sql->SelectMysql(sqlbuf,1,lstRes);
        if(!res)
        {
            cout<<"select fail"<<sqlbuf<<endl;
            return;
        }
        if(lstRes.size()>0)
        {
            link=0;
        }
    }while(link==0);
    //遍历所有文件 ，设置分享马
    int itemCount=rq->itemConunt;
    for(int i=0;i<itemCount;i++)
    {
        ShareItem(rq->userid,rq->fileArray[i],rq->dir,rq->shareTime,link);
    }
    STRU_SHARE_FILE_RS rs;
    rs.result=1;
    SendData(clientfd,(char*)&rs,sizeof (rs));
}

void CLogic::MyShareRq(sock_fd clientfd, char *szbuf, int nlen)
{

    //拆包
    STRU_MY_SHARE_RQ * rq = (STRU_MY_SHARE_RQ *)szbuf;
    //rq->userid;
    //根据id查询 获得分享文件列表
    //查的内容 f_name f_size s_linkTime s_link
    char sqlbuf[1000] = "";
    sprintf( sqlbuf , "select f_name ,f_size, s_linkTime ,s_link from user_file_info where u_id = %d and s_link is not null and s_linkTime is not null;", rq->userid );
    list<string> lst;
    bool res = m_sql->SelectMysql( sqlbuf , 4 , lst );
    if(!res){
        cout << "select fail:"<<sqlbuf << endl; return;
    }
    int count = lst.size();
    if((count/4 == 0) || ( count %4 != 0) ) return;

    count/=4;
    //写回复
    int packlen = sizeof( STRU_MY_SHARE_RS ) + count* sizeof(STRU_MY_SHARE_FILE );

    STRU_MY_SHARE_RS *rs = (STRU_MY_SHARE_RS *)malloc(packlen);
    rs->init();
    rs->itemCount = count;
    for( int i = 0 ; i < count ; ++i){
        string name = lst.front(); lst.pop_front();
        int size = stoi( lst.front());lst.pop_front();
        string time = lst.front(); lst.pop_front();
        int link = stoi(lst.front()); lst.pop_front();
        strcpy( rs->items[i].name , name.c_str() );
        rs->items[i].size =size;
        strcpy( rs->items[i].time , time.c_str() );
        rs->items[i].shareLink = link;
    }

    //发送
    SendData( clientfd , (char*)rs , packlen );

    free(rs);
}

void CLogic::GetShareRq(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
        STRU_GET_SHARE_RQ * rq = (STRU_GET_SHARE_RQ *)szbuf;

        //根据分享码 查询到一系列文件
        //查询路径: f_id, f_name, f_dir(分享人的) f_type u_id(分享人的)
        //select f_id, f_name, f_dir, f_type ,u_id from t_user_file where s_link = %d;
        char sqlbuf[1000] = "";
        sprintf(sqlbuf, "select f_id, f_name, f_dir, f_type ,u_id from user_file_info where s_link = %d;", rq->shareLink);
        list<string> lst;
        bool res = m_sql->SelectMysql( sqlbuf , 5 , lst );

        STRU_GET_SHARE_RS rs;
        if(!res){
            cout<<"select fail:"<<sqlbuf<<endl;
            rs.result = 0;
            SendData( clientfd , (char*)&rs , sizeof(rs) );
            return;
        }

        if(lst.size() == 0){
            rs.result = 0;
            SendData( clientfd , (char*)&rs , sizeof(rs) );
            return;
        }
        rs.result = 1;

        //遍历文件列表
        while( !lst.empty() ) {
            int fileid = stoi( lst.front() ); lst.pop_front();
            string name = lst.front(); lst.pop_front();
            string fromdir = lst.front(); lst.pop_front();
            string type = lst.front(); lst.pop_front();
            int fromuserid = stoi( lst.front() ); lst.pop_front();

            if( type == "file") {
                //如果是文件
                //插入信息到 用户文件夹系统
                GetShareByFile( rq->userid , fileid , rq->dir , name , rq->time );
            }else{
                //如果是文件夹
                //插入信息到 用户文件夹系统
                //拼接路径  获取人目录 /->/06/  分享人的目录 /->/06/
                //根据新路径 在分享人那边查询 文件夹下的文件
                //遍历列表 --> 递归
                GetShareByFolder( rq->userid , fileid , rq->dir , name , rq->time , fromuserid , fromdir );
            }
        }

        //写回复包
        strcpy( rs.dir , rq->dir );
        //发送
        SendData( clientfd , (char*)&rs , sizeof(rs) );
}
void CLogic::GetShareByFile( int userId, int fileid , string dir , string name , string time )
{
    char sqlbuf[1000] = "";
    sprintf( sqlbuf , "insert into t_user_file ( u_id , f_id , f_dir , f_name , f_uploadtime ) values( %d , %d , '%s' , '%s' , '%s');" , userId , fileid, dir.c_str()  , name.c_str() , time.c_str() );

    bool res = m_sql->UpdataMysql( sqlbuf );
    if( !res ) {
        printf( "update fail:%s\n" , sqlbuf );
    }
}

void CLogic::GetShareByFolder(int userId, int fileid, string dir, string name, string time, int fromuserid, string fromdir)
{
    //插入信息表 用户文件夹名称
    GetShareByFile( userId, fileid , dir , name , time );
    //删除密码
    //获取当前路径 /->/360/
    string newdir = dir + name + "/";
    //分享来源路径 /->/360/
    string newfromdir = fromdir + name + "/";

    //根据来源路径 在分享人获取文件夹下的文件
    char sqlbuf[1000] = "";
    sprintf( sqlbuf , "select f_id, f_name, f_type from user_file_info where u_id = %d and f_dir = '%s';" , fromuserid , newfromdir.c_str() );
    list<string> lst;
    bool res = m_sql->SelectMysql(sqlbuf , 3 , lst );
    if( !res ){
        cout << "select fail:" << sqlbuf << endl;
        return;
    }

    while( !lst.empty() ){
        int fileid = stoi( lst.front() ); lst.pop_front();
        string name = lst.front(); lst.pop_front();
        string type = lst.front(); lst.pop_front();

        if( type == "file" ) {
            //文件
            GetShareByFile( userId , fileid , newdir , name , time );
        }else{
            //文件夹 ->递归
            GetShareByFolder( userId , fileid , newdir , name , time ,
            fromuserid , newfromdir );
        }
    }
}




void CLogic::ShareItem(int userid, int fileid, string dir, string time, int link)
{
    char sqlbuf[1000] = "";
    sprintf( sqlbuf , "update t_user_file set s_link = '%d', s_linkTime = '%s' where u_id = %d and f_id = %d and f_dir = '%s';", link , time.c_str() , userid , fileid , dir.c_str() );

  bool res = m_sql->UpdataMysql( sqlbuf ) ;
  if( !res ) {
      cout << "UpdataMysql fail " << sqlbuf <<endl;
      return;
  }
}

void CLogic::DeleteFileRq(sock_fd clientfd, char *szbuf, int nlen)
{
    //拆包
    STRU_DELETE_FILE_RQ* rq=(STRU_DELETE_FILE_RQ*)szbuf;
    //获得id列表
    for(int i=0;i<rq->fileCount;++i)
    {
        //删除每一项
        int fileid=rq->fileidArray[i];
        DeleteOneItem(rq->userid,fileid,rq->dir);
    }
    //写回复
    STRU_DELETE_FILE_RS rs;
    rs.result=1;
    strcpy(rs.dir,rq->dir);
    SendData(clientfd,(char*)&rs,sizeof(rs));
}
void CLogic::DeleteOneItem(int userid,int fileid,string dir)
{
    // 软删除：设置 f_state=0，文件进入回收站
    char sqlbuf[1000] = "";
    sprintf(sqlbuf, "update t_file set f_state=0 where f_id=%d", fileid);
    bool res = m_sql->UpdataMysql(sqlbuf);
    if (!res) {
        cout << "soft delete fail:" << sqlbuf << endl;
    }
}
void CLogic::DeleteFile(int userid,int fileid,string dir,string path)
{
    //删除用户文件对应的关系
    char sqlbuf[1000]="";
    sprintf(sqlbuf,"delete from t_user_file where u_id =%d and f_id=%d and f_dir='%s';"
            ,userid,fileid,dir.c_str());
    bool res=m_sql->UpdataMysql(sqlbuf);
    if(!res)
    {
        cout<<"delete fail:"<<sqlbuf<<endl;
        return;
    }
    //再次查询 id 看看能不能找到数据库记录 ，如果不能，删除本地文件
    sprintf(sqlbuf,"select f_id from t_file where f_id =%d",fileid);
    list<string> lst;
    res=m_sql->SelectMysql(sqlbuf,1,lst);
    if(!res)
    {
        cout<<"select fail:"<<sqlbuf<<endl;
        return;
    }
    if(lst.size()==0)
    {
        unlink(path.c_str());  //文件io 删除文件
    }
}
void CLogic::DeleteFolder(int userid,int fileid,string dir,string name)
{
    //删除用户文件对应关系 u_id f_dir f_id
    char sqlbuf[1000]="";
    sprintf(sqlbuf,"delete from t_user_file where u_id =%d and f_id=%d and f_dir='%s';"
            ,userid,fileid,dir.c_str());
    bool res=m_sql->UpdataMysql(sqlbuf);
    if(!res)
    {
        cout<<"delete fail:"<<sqlbuf<<endl;
        return;
    }
    //拼接新路经
   string newDir=dir+name+"/";

   ///查表 根据新路径查表 得到列表 f_type  f_id  name  path
   sprintf(sqlbuf, "select f_type, f_id, f_name, f_path from user_file_info where u_id = %d and f_dir = '%s';", userid, newDir.c_str());
   list<string> lst;
    res = m_sql->SelectMysql(sqlbuf, 4, lst);
   if(!res)
   {
       cout << "SelectMysql fail:" << sqlbuf << endl;
       return;
   }

   while(lst.size() != 0)
   {
       //循环
       string type = lst.front(); lst.pop_front();
       int fileid = stoi(lst.front()); lst.pop_front();
       string name = lst.front(); lst.pop_front();
       string path = lst.front(); lst.pop_front();

       if(type == "file")
           //如果是文件
           DeleteFile(userid, fileid, newDir, path);
       else
           //如果是文件夹
           DeleteFolder(userid, fileid, newDir, name);
   }
}

void CLogic::ContinueDownloadRq(sock_fd clientfd, char *szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    //拆包
    STRU_CONTINUE_DOWNLOAD_RQ* rq=(STRU_CONTINUE_DOWNLOAD_RQ*)szbuf;
    //看是否存在文件信息
    int64_t user_time=rq->userid*getNumber()+rq->timestamp;
    FileInfo * info=nullptr;
    if(!m_mapTimestampToFileInfo.find(user_time,info))
    {
        //没有创建文件信息  ---由查表来 添加到map
        //查数据库 f_name,f_path,F_MD5,f_size
        char sqlbuf[1000]="";
        sprintf(sqlbuf,"select f_name,f_path,f_MD5,f_size from user_file_info where u_id=%d and f_dir='%s' and f_id=%d;",rq->userid,rq->dir,rq->fileid);
        list<string> lstRes;
        bool res=m_sql->SelectMysql(sqlbuf,4,lstRes);
        if(!res)
        {
            cout<<"select fail:"<<sqlbuf<<endl;
            return;
        }
        if(lstRes.size()==0)//没有返回
        {
            return;
        }
        string strName=lstRes.front(); lstRes.pop_front();
        string strPath=lstRes.front(); lstRes.pop_front();
        string strMD5=lstRes.front(); lstRes.pop_front();
        int size=stoi(lstRes.front());lstRes.pop_front();
        //有写文件信息
        info->absolutePath=strPath;
        info->fid=rq->fileid;
        info->size=size;
        info->dir=rq->dir;

        info->md5=strMD5;
        info->name=strName;
        cout<<"服务端"<<strName<<endl;
        info->type="file";
        info->fileFd=open(info->absolutePath.c_str(),O_RDONLY);
        if(info->fileFd<=0)
        {
            cout<<"file open fail"<<endl;
            return;
        }
        //存在map中
        m_mapTimestampToFileInfo.insert(user_time,info);
    }


    //现在已经由信息

    //文件指针跳转 pos位置 同步pos
    lseek(info->fileFd,rq->pos,SEEK_SET );
    info->pos=rq->pos;

    //读文件块 发送文件块请求
    STRU_FILE_CONTENT_RQ contentRq;
    contentRq.len=read(info->fileFd,contentRq.content,_DEF_BUFFER);
    contentRq.fileid=rq->fileid;
    contentRq.userid=rq->userid;
    contentRq.timestamp=rq->timestamp;

    SendData(clientfd,(char*)&contentRq,sizeof(contentRq));

}

void CLogic::ContinueUploadRq(sock_fd clientfd, char *szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    //拆包
    STRU_CONTINUE_UPLOAD_RQ* rq=(STRU_CONTINUE_UPLOAD_RQ*)szbuf;

    int64_t user_time=rq->timestamp+rq->userid*getNumber();
    //需要看map中是否存在
    FileInfo* info=nullptr;
     //不存在 创建
    if(!m_mapTimestampToFileInfo.find(user_time,info))
    {
        info=new FileInfo;
        info->dir=rq->dir;
        info->fid=rq->fileid;
        info->type="file";
        //查表 获取信息 给info赋值 然后打开文件 info加到map中
        char sqlbuf[1000]="";
        sprintf(sqlbuf,"select f_name,f_path,f_size,f_MD5 from user_file_info where u_id=%d and f_dir='%s' and f_id=%d and f_state=0;",rq->userid,rq->dir,rq->fileid);
        list<string> lst;
        bool res=m_sql->SelectMysql(sqlbuf,4,lst);
        if(!res)
        {
            cout<<"select fail:"<<sqlbuf<<endl;
            return;
        }
        if(lst.size()==0)//没有返回
        {
            return;
        }
        info->name=lst.front();lst.pop_front();
        info->absolutePath=lst.front();lst.pop_front();
        info->size=stoi(lst.front());lst.pop_front();
        info->md5=lst.front();lst.pop_front();

        info->fileFd=open(info->absolutePath.c_str(),O_WRONLY);
        if(info->fileFd<=0)
        {
            cout<<"file open fail:"<<errno<<endl;
            return;
        }
        m_mapTimestampToFileInfo.insert(user_time,info);


    }


    //当已经存在的时候 lseek跳转并读取文件当前写的位置（文件末尾） 更新pos
    info->pos=lseek(info->fileFd,0,SEEK_END);
    //写回复
    STRU_CONTINUE_UPLOAD_RS rs;
    rs.fileid=rq->fileid;
    rs.pos=info->pos;
    rs.timestamp=rq->timestamp;
    SendData(clientfd,(char*)&rs,sizeof(rs));
}

// 搜索文件
void CLogic::SearchFileRq(sock_fd clientfd, char* szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    STRU_SEARCH_FILE_RQ* rq = (STRU_SEARCH_FILE_RQ*)szbuf;

    char sqlbuf[1000] = "";
    sprintf(sqlbuf, "select f_id,f_name,f_size,f_uploadtime,f_type from user_file_info where u_id = %d and f_name like '%%%s%%' and f_state=1",
            rq->userid, rq->searchKey);
    list<string> lstRes;
    bool res = m_sql->SelectMysql(sqlbuf, 5, lstRes);
    if (!res) {
        cout << "select fail:" << sqlbuf << endl;
        return;
    }
    if (lstRes.size() == 0) return;

    int count = lstRes.size() / 5;
    int packlen = sizeof(STRU_SEARCH_FILE_RS) + count * sizeof(STRU_FILE_INFO);
    STRU_SEARCH_FILE_RS* rs = (STRU_SEARCH_FILE_RS*)malloc(packlen);
    rs->init();
    rs->count = count;
    for (int i = 0; i < count; i++) {
        int f_id = stoi(lstRes.front()); lstRes.pop_front();
        string name = lstRes.front(); lstRes.pop_front();
        int f_size = stoi(lstRes.front()); lstRes.pop_front();
        string time = lstRes.front(); lstRes.pop_front();
        string f_type = lstRes.front(); lstRes.pop_front();

        rs->fileinfo[i].fileid = f_id;
        strcpy(rs->fileinfo[i].name, name.c_str());
        rs->fileinfo[i].size = f_size;
        strcpy(rs->fileinfo[i].time, time.c_str());
        strcpy(rs->fileinfo[i].fileType, f_type.c_str());
    }
    SendData(clientfd, (char*)rs, packlen);
    free(rs);
}

// 获取回收站文件列表
void CLogic::GetRecycleRq(sock_fd clientfd, char* szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    STRU_GET_RECYCLE_RQ* rq = (STRU_GET_RECYCLE_RQ*)szbuf;

    char sqlbuf[1000] = "";
    sprintf(sqlbuf, "select f_id,f_name,f_size,f_uploadtime,f_type from user_file_info where u_id=%d and f_state=0", rq->userid);
    list<string> lstRes;
    bool res = m_sql->SelectMysql(sqlbuf, 5, lstRes);
    if (!res) {
        cout << "select fail:" << sqlbuf << endl;
        return;
    }
    if (lstRes.size() == 0) return;

    int count = lstRes.size() / 5;
    int packlen = sizeof(STRU_GET_RECYCLE_RS) + count * sizeof(STRU_FILE_INFO);
    STRU_GET_RECYCLE_RS* rs = (STRU_GET_RECYCLE_RS*)malloc(packlen);
    rs->init();
    rs->count = count;
    for (int i = 0; i < count; i++) {
        int f_id = stoi(lstRes.front()); lstRes.pop_front();
        string name = lstRes.front(); lstRes.pop_front();
        int f_size = stoi(lstRes.front()); lstRes.pop_front();
        string time = lstRes.front(); lstRes.pop_front();
        string f_type = lstRes.front(); lstRes.pop_front();

        rs->fileinfo[i].fileid = f_id;
        strcpy(rs->fileinfo[i].name, name.c_str());
        rs->fileinfo[i].size = f_size;
        strcpy(rs->fileinfo[i].time, time.c_str());
        strcpy(rs->fileinfo[i].fileType, f_type.c_str());
    }
    SendData(clientfd, (char*)rs, packlen);
    free(rs);
}

// 从回收站恢复文件
void CLogic::RestoreFileRq(sock_fd clientfd, char* szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    STRU_RESTORE_FILE_RQ* rq = (STRU_RESTORE_FILE_RQ*)szbuf;

    for (int i = 0; i < rq->fileCount; i++) {
        char sqlbuf[1000] = "";
        sprintf(sqlbuf, "update t_file set f_state=1 where f_id=%d", rq->fileidArray[i]);
        bool res = m_sql->UpdataMysql(sqlbuf);
        if (!res) {
            cout << "restore fail:" << sqlbuf << endl;
        }
    }
    STRU_RESTORE_FILE_RS rs;
    rs.result = 1;
    SendData(clientfd, (char*)&rs, sizeof(rs));
}

// 添加收藏
void CLogic::AddFavoriteRq(sock_fd clientfd, char* szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    STRU_ADD_FAVORITE_RQ* rq = (STRU_ADD_FAVORITE_RQ*)szbuf;

    char timebuf[60] = "";
    time_t t = time(NULL);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&t));

    char sqlbuf[1000] = "";
    sprintf(sqlbuf, "insert into t_favorite (u_id, f_id, f_dir, f_name, fav_time) values(%d, %d, '%s', '%s', '%s')",
            rq->userid, rq->fileid, rq->dir, rq->name, timebuf);
    bool res = m_sql->UpdataMysql(sqlbuf);
    STRU_ADD_FAVORITE_RS rs;
    rs.result = res ? 1 : 0;
    SendData(clientfd, (char*)&rs, sizeof(rs));
}

// 获取收藏列表
void CLogic::GetFavoriteRq(sock_fd clientfd, char* szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    STRU_GET_FAVORITE_RQ* rq = (STRU_GET_FAVORITE_RQ*)szbuf;

    char sqlbuf[1000] = "";
    sprintf(sqlbuf, "select f.f_id, f.f_name, f.f_size, f.f_uploadtime, f.f_type from t_favorite fv join user_file_info f on fv.f_id=f.f_id and fv.u_id=f.u_id where fv.u_id=%d", rq->userid);
    list<string> lstRes;
    bool res = m_sql->SelectMysql(sqlbuf, 5, lstRes);
    if (!res) {
        cout << "select fail:" << sqlbuf << endl;
        return;
    }
    if (lstRes.size() == 0) return;

    int count = lstRes.size() / 5;
    int packlen = sizeof(STRU_GET_FAVORITE_RS) + count * sizeof(STRU_FILE_INFO);
    STRU_GET_FAVORITE_RS* rs = (STRU_GET_FAVORITE_RS*)malloc(packlen);
    rs->init();
    rs->count = count;
    for (int i = 0; i < count; i++) {
        int f_id = stoi(lstRes.front()); lstRes.pop_front();
        string name = lstRes.front(); lstRes.pop_front();
        int f_size = stoi(lstRes.front()); lstRes.pop_front();
        string time = lstRes.front(); lstRes.pop_front();
        string f_type = lstRes.front(); lstRes.pop_front();

        rs->fileinfo[i].fileid = f_id;
        strcpy(rs->fileinfo[i].name, name.c_str());
        rs->fileinfo[i].size = f_size;
        strcpy(rs->fileinfo[i].time, time.c_str());
        strcpy(rs->fileinfo[i].fileType, f_type.c_str());
    }
    SendData(clientfd, (char*)rs, packlen);
    free(rs);
}

// 取消收藏
void CLogic::DelFavoriteRq(sock_fd clientfd, char* szbuf, int nlen)
{
    _DEF_COUT_FUNC_;
    STRU_DEL_FAVORITE_RQ* rq = (STRU_DEL_FAVORITE_RQ*)szbuf;

    char sqlbuf[1000] = "";
    sprintf(sqlbuf, "delete from t_favorite where u_id=%d and f_id=%d", rq->userid, rq->fileid);
    bool res = m_sql->UpdataMysql(sqlbuf);
    STRU_DEL_FAVORITE_RS rs;
    rs.result = res ? 1 : 0;
    SendData(clientfd, (char*)&rs, sizeof(rs));
}
