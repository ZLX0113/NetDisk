#ifndef COMMON_H
#define COMMON_H

#include<QString>
////////////////////文件信息/////////////////
struct FileInfo
{

    FileInfo():fileid(0) , size(0),pFile( nullptr )
      , pos(0) , isPause(0),timestamp(0){

    }

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
    int isPause; //暂停  0 1

    //文件指针
    FILE* pFile;
    //字节单位换算
    static QString getSize(int size)//得到。。。KB ...MB
    {
        QString res;
        int count=0;
        int tmp=size;
        while(tmp!=0)
        {
            tmp/=1024;
            if(tmp!=0) count++;
        }
        switch(count)
        {
        case 0://kb
            res=QString("0.%1KB").arg((int)(size%1024/1024.0*100),2,10,QChar('0'));
            //arg 参数第二个 多宽 3进制 4不够宽度 缺省的字符
            if(size!=0 && res=="0.00KB")
            {
                res=="0.01KB";
            }
            break;
        case 1: //kb
            res=QString("%1.%2KB").arg(size/1024).arg((int)(size%1024/1024.0*100),2,10,QChar('0'));
            break;
        case 2:
        case 3://mb
            res=QString("%1.%2MB").arg(size/(1024*1024)).arg((int)(size%(1024*1024)/(1024*1024.0)*100),2,10,QChar('0'));
            break;
        default:
            break;
        }
        return res;
    }
};






#endif // COMMON_H
