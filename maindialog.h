#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <QCloseEvent>
#include<QMessageBox>
#include <QMenu>
#include<common.h>
class CKernel;
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
signals:
    void SIG_close();
    //信号什么绝对路径的文件，上传到什么目录下
    void SIG_uploadFile(QString path,QString dir);
    //什么文件id，什么目录下的文件 下载
    void SIG_downloadFile(int fileid,QString dir);
    //什么文件id，什么目录下的文件jia 下载
    void SIG_downloadFolder(int fileid,QString dir);
    //什么路径下创建什么名字的文件夹
    void SIG_addFolder(QString name,QString dir);
    //改变路径
    void SIG_changeDir(QString dir);
    //信号什么路径下文件，上传到什么目录下
    void SIG_uploadFolder(QString path,QString dir);
    //选中文件，分享到什么路径下
    void SIG_shareFile(QVector<int> fileidArray,QString dir);
    //获取什么分享码的文件 添加到什么目录
    void SIG_getShareByLink( int code , QString dir );
    //删除什么目录下的 一系列的文件（文件id 数组）
    void SIG_deleteFile(QVector<int> fileidArray,QString dir);
    //上传zanting
    void SIG_setuploadPause(int timestamp,int isPause);
    //下载暂停 0开始 1暂停
    void SIG_setdownloadPause(int timestamp,int isPause);
    //搜索文件
    void SIG_searchFile(QString key);
    // 回收站
    void SIG_getRecycle();
    void SIG_restoreFile(QVector<int> fileids);
    // 收藏
    void SIG_addFavorite(QString dir, QString name, int fileid);
    void SIG_getFavorite();
    void SIG_delFavorite(int fileid);

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
public:
    void debugTableHeaders();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void slot_setInfo(QString name);
    void on_pb_file_clicked();

    void on_pb_transmit_clicked();

    void on_pb_share_clicked();

    void on_pb_addfile_clicked();

    void slot_addFolder(bool flat);
    void slot_uploadFile(bool flat);
    void slot_uploadFolder(bool flat);
    void slot_downloadFile(bool flat);
    void slot_shareFile(bool flat);
    void slot_getShare(bool flat);
    void slot_deleteFile(bool flat);
    void slot_favoriteFile(bool flat);
    void slot_restoreFile(bool flat);
    void slot_delFavorite(bool flat);
    void slot_uploadPause(bool flat);
    void slot_uploadResume(bool flat);
    void slot_uploadAllPause(bool flat);
    void slot_uploadAllResume(bool flat);
    void slot_downloadPause(bool flat);
    void slot_downloadResume(bool flat);
    void slot_downloadAllPause(bool flat);
    void slot_downloadAllResume(bool flat);


    void slot_insertUploadFile(FileInfo& info);

    void slot_insertUploadComplete(FileInfo& info);

    void slot_updateUploadFileProgress(int timestamp,int pos);
    void slot_updateDownloadFileProgress(int timestamp,int pos);

    void slot_deleteUploadFileByrow(int row);

    void slot_insertDownloadComplete(FileInfo& info);
    void slot_deleteDownloadFileByrow(int row);


    void slot_insertFileInfo(FileInfo& info);
    void slot_insertShareFileInfo(QString name,int size,QString time,int shareLink);
    void slot_insertDownloadFile(FileInfo& info);

    void on_table_file_cellClicked(int row, int column);

    void on_table_file_customContextMenuRequested(const QPoint &pos);

    void slot_openPath(bool flag);
    void slot_deleteAllFileInfo();
    void slot_deleteShareAllFileInfo();
    void on_table_file_cellDoubleClicked(int row, int column);

    void on_pb_prev_clicked();

    void on_table_upload_cellClicked(int row, int column);

    void on_table_download_cellClicked(int row, int column);

    bool slot_getDownlaodFileInfoByTimesatmp(int timestamp,FileInfo& info);
    bool slot_getUplaodFileInfoByTimesatmp(int timestamp,FileInfo& info);

private:
    Ui::Dialog *ui;
    QMenu m_menuAddFile;
    QMenu m_menuFileInfo;
    QMenu m_menuUpload;
    QMenu m_menuDownload;
    friend class CKernel;
};

#endif // MAINDIALOG_H


//如何实现点击×的时候--->执行关闭事件（qt中有捕捉信号的函数closeEvent---->弹出窗口
//发送关闭信号 核心类接受信号 然后回收资源
//qt中斜体代表重写的类函数，意味着父类有该事件
