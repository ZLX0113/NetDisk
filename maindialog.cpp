#include "maindialog.h"
#include "ui_maindialog.h"
#include<QMessageBox>
#include<QDebug>
#include<QInputDialog>
#include"mytablewidgetitem.h"
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    //默认文件页
    ui->sw_page->setCurrentIndex(0);
    //传输默认分页
    ui->tw_transmit->setCurrentIndex(2);
    //设置标题栏
    this->setWindowTitle("我的网盘");
    //设置最大最小化
    this->setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    //定义菜单项  资源路径
    QAction * action_addFolder=new QAction(QIcon(":/image/folder.png"),"新建文件夹");
    QAction * action_uploadFile=new QAction("上传文件");
    QAction * action_uploadFolder=new QAction("上传文件夹");
    //添加菜单项
    m_menuAddFile.addAction(action_addFolder);
    m_menuAddFile.addSeparator(); //加入分隔符
    m_menuAddFile.addAction(action_uploadFile);
    m_menuAddFile.addAction(action_uploadFolder);
    connect(action_addFolder,SIGNAL(triggered(bool)),this,SLOT(slot_addFolder(bool)));
    connect(action_uploadFile,SIGNAL(triggered(bool)),this,SLOT(slot_uploadFile(bool)));
    connect(action_uploadFolder,SIGNAL(triggered(bool)),this,SLOT(slot_uploadFolder(bool)));

    QAction * action_downloadFile=new QAction("下载文件");
    QAction * action_shareFile=new QAction("共享文件");
    QAction * action_deleteFile=new QAction("删除文件");
    QAction * action_getShare=new QAction("获取分享");

    m_menuFileInfo.addAction(action_addFolder);
    m_menuFileInfo.addSeparator(); //加入分隔符
    m_menuFileInfo.addAction(action_downloadFile);
    m_menuFileInfo.addAction(action_shareFile);
    m_menuFileInfo.addAction(action_deleteFile);
    m_menuFileInfo.addAction("收藏");
    QAction* action_favorite = m_menuFileInfo.actions().last();
    connect(action_favorite, SIGNAL(triggered(bool)), this, SLOT(slot_favoriteFile(bool)));

    QAction* action_restore = new QAction("恢复");
    QAction* action_delFav = new QAction("取消收藏");
    m_menuFileInfo.addAction(action_restore);
    m_menuFileInfo.addAction(action_delFav);
    connect(action_restore, SIGNAL(triggered(bool)), this, SLOT(slot_restoreFile(bool)));
    connect(action_delFav, SIGNAL(triggered(bool)), this, SLOT(slot_delFavorite(bool)));
    m_menuFileInfo.addSeparator(); //加入分隔符
    m_menuFileInfo.addAction(action_getShare);
    //connect(action_addFolder,SIGNAL(triggered(bool)),this,SLOT(slot_addFolder(bool)));

    connect(action_downloadFile,SIGNAL(triggered(bool)),this,SLOT(slot_downloadFile(bool)));
    connect(action_shareFile,SIGNAL(triggered(bool)),this,SLOT(slot_shareFile(bool)));
    connect(action_deleteFile,SIGNAL(triggered(bool)),this,SLOT(slot_deleteFile(bool)));
    connect(action_getShare,SIGNAL(triggered(bool)),this,SLOT(slot_getShare(bool)));



    // 连接右键菜单信号
    connect(ui->table_download, &QTableWidget::customContextMenuRequested, this,
        [this](QPoint pos){
            int row = ui->table_download->indexAt(pos).row();
            if (row >= 0) {
                ui->table_download->selectRow(row);
                ui->table_download->setCurrentCell(row, 0);
            }
            this->m_menuDownload.exec(QCursor::pos());
        });
    connect(ui->table_upload, &QTableWidget::customContextMenuRequested, this,
        [this](QPoint pos){
            int row = ui->table_upload->indexAt(pos).row();
            if (row >= 0) {
                ui->table_upload->selectRow(row);
                ui->table_upload->setCurrentCell(row, 0);
            }
            this->m_menuUpload.exec(QCursor::pos());
        });

    // 搜索按钮
    connect(ui->pb_search, &QPushButton::clicked, this, [this]() {
        bool ok;
        QString key = QInputDialog::getText(this, "搜索文件", "请输入文件名关键词:", QLineEdit::Normal, "", &ok);
        if (ok && !key.isEmpty()) {
            Q_EMIT SIG_searchFile(key);
        }
    });


    // 创建动作
    QAction *actionUploadPause = new QAction("暂停");
    QAction *actionUploadResume = new QAction("开始");
    QAction *actionUploadAllPause = new QAction("全部暂停");
    QAction *actionUploadAllResume = new QAction("全部开始");
    QAction *actionDownloadPause = new QAction("暂停");
    QAction *actionDownloadResume = new QAction("开始");
    QAction *actionDownloadAllPause = new QAction("全部暂停");
    QAction *actionDownloadAllResume = new QAction("全部开始");

    // 添加上传菜单动作
    m_menuUpload.addAction(actionUploadPause);
    m_menuUpload.addAction(actionUploadResume);
    m_menuUpload.addAction(actionUploadAllPause);
    m_menuUpload.addAction(actionUploadAllResume);

    // 添加下载菜单动作
    m_menuDownload.addAction(actionDownloadPause);
    m_menuDownload.addAction(actionDownloadResume);
    m_menuDownload.addAction(actionDownloadAllPause);
    m_menuDownload.addAction(actionDownloadAllResume);


    // 连接动作信号槽
    connect(actionUploadPause, SIGNAL(triggered(bool)),
        this, SLOT(slot_uploadPause(bool)));
    connect(actionUploadResume, SIGNAL(triggered(bool)),
        this, SLOT(slot_uploadResume(bool)));
    connect(actionUploadAllPause, SIGNAL(triggered(bool)),
        this, SLOT(slot_uploadAllPause(bool)));
    connect(actionUploadAllResume, SIGNAL(triggered(bool)),
        this, SLOT(slot_uploadAllResume(bool)));
    connect(actionDownloadPause, SIGNAL(triggered(bool)),
        this, SLOT(slot_downloadPause(bool)));
    connect(actionDownloadResume, SIGNAL(triggered(bool)),
        this, SLOT(slot_downloadResume(bool)));
    connect(actionDownloadAllPause, SIGNAL(triggered(bool)),
        this, SLOT(slot_downloadAllPause(bool)));
    connect(actionDownloadAllResume, SIGNAL(triggered(bool)),
        this, SLOT(slot_downloadAllResume(bool)));

    // 回收站按钮 - 显示回收站文件列表
    connect(ui->pb_hsz, &QPushButton::clicked, this, [this]() {
        ui->sw_page->setCurrentIndex(0);
        Q_EMIT SIG_getRecycle();
        ui->table_file->setContextMenuPolicy(Qt::CustomContextMenu);
    });

    // 收藏按钮 - 显示收藏列表
    connect(ui->pb_store, &QPushButton::clicked, this, [this]() {
        ui->sw_page->setCurrentIndex(0);
        Q_EMIT SIG_getFavorite();
    });

    // 搜索返回按钮 - 返回搜索前目录
    connect(ui->pb_back, &QPushButton::clicked, this, [this]() {
        Q_EMIT SIG_changeDir("/");
    });
}



Dialog::~Dialog()
{
    delete ui;
}

void Dialog::debugTableHeaders()
{
    qDebug() << "=== 表格状态检查 ===";
        qDebug() << "列数:" << ui->table_myshare->columnCount();
        for(int i = 0; i < ui->table_myshare->columnCount(); ++i) {
            QTableWidgetItem* headerItem = ui->table_myshare->horizontalHeaderItem(i);
            if(headerItem) {
                qDebug() << "表头" << i << ":" << headerItem->text();
            } else {
                qDebug() << "表头" << i << ": NULL (这是问题所在!)";
            }
        }
}


void Dialog::closeEvent(QCloseEvent *event)
{
    if(QMessageBox::question(this,"退出提示","是否退出")
            ==QMessageBox::Yes)
    {
        //是yes,关闭
        event->accept();
        Q_EMIT SIG_close();
    }
    else
    {
        event->ignore();
    }
}

void Dialog::slot_setInfo(QString name)
{
    ui->pb_name->setText(name);
}

void Dialog::on_pb_file_clicked()
{
    ui->sw_page->setCurrentIndex(0);
}


void Dialog::on_pb_transmit_clicked()
{
    ui->sw_page->setCurrentIndex(1);
}


void Dialog::on_pb_share_clicked()
{
    ui->sw_page->setCurrentIndex(2);
}

//点击添加文件
void Dialog::on_pb_addfile_clicked()
{
    m_menuAddFile.exec(QCursor::pos());  //鼠标的坐标 在该店显示菜单
}
//新建文件夹
#include<QInputDialog>
void Dialog::slot_addFolder(bool flat)
{
     qDebug()<<__func__;
     //弹出输入窗口
     QString name=QInputDialog::getText(this,"新建文件夹","输入名称");
     QString tmp=name;
     if(name.isEmpty() || tmp.remove(" ").isEmpty() || name.length()>100)
     {
         QMessageBox::about(this,"提示","名字非法");
         return;
     }
     //不可以使用的名字
     //写文档或数据库 使用正则表达式判断是否有敏感词汇todo

     //一些非法 \ / : ? * < > | "
     if(name.contains("\\") || name.contains("/") || name.contains(":") || name.contains("?") || name.contains("<") || name.contains(">") || name.contains("|") ||name.contains("\""))
     {
         QMessageBox::about(this,"提示","名字非法");
         return;
     }
     //判断现在是否已经存在 todo
     QString dir=ui->lb_path->text();
     Q_EMIT SIG_addFolder(name,dir);
}
#include<QFileDialog>
//上传文件
void Dialog::slot_uploadFile(bool flat)
{
    qDebug()<<__func__;
    //弹出窗口
    QString path=QFileDialog::getOpenFileName(this,"选择文件","./");
    if(path.isEmpty()) return;
    //目前上传的文件 有没有一样的文件 如果是 取消 todo

    //发送信号 核心类处理 传递的消息：上传到什么文件下 什么目录下
    QString dir=ui->lb_path->text();
    Q_EMIT SIG_uploadFile(path,dir);
}



void Dialog::slot_uploadFolder(bool flat)
{
    qDebug()<<__func__;
    //点击 弹出文件选择对话框 选择路径
    QString path=QFileDialog::getExistingDirectory(this,"选择文件夹","./");
    //判断非空
    if(path.isEmpty())   return;
    //过滤 是否正在上传 todo

    //发信号 上传什么目录下的什么名字的文件夹
    emit SIG_uploadFolder(path,ui->lb_path->text());
}

void Dialog::slot_downloadFile(bool flat)
{
    qDebug()<<__func__;
    //遍历列表
    int rows=ui->table_file->rowCount();
     //获取目录
    QString dir=ui->lb_path->text();
    for(int i=0;i<rows;i++)
    {
        //选中的
        MyTableWidgetItem* item0=(MyTableWidgetItem*)ui->table_file->item(i,0);
        if(item0->checkState()==Qt::Checked)
        {
            //列表中有下载的，不能开始todo

            //获取类型 看文件还是文件夹
            if(item0->m_info.type=="file")
            {
                Q_EMIT SIG_downloadFile(item0->m_info.fileid,dir);
            }
            else
            {
                Q_EMIT SIG_downloadFolder(item0->m_info.fileid,dir);
            }
            //发信号 下载文件 或 下载文件夹

        }

    }

}

void Dialog::slot_shareFile(bool flat)
{
    qDebug()<<__func__;
    //申请数组
    QVector<int> array;
    int count=ui->table_file->rowCount();

    //遍历所有项
    for(int i=0;i<count;i++)
    {
        MyTableWidgetItem * item0=( MyTableWidgetItem *)ui->table_file->item(i,0);
        //看是否打勾
        if(item0->checkState()==Qt::Checked)
        {
            //添加到数组中
            array.push_back(item0->m_info.fileid);
        }

    }

    //发送信号
    emit SIG_shareFile(array,ui->lb_path->text());
}

void Dialog::slot_getShare(bool flag)
{
    qDebug() << __func__;
    // 弹窗 输入分享码
    QString txt = QInputDialog::getText( this , "获取分享", "输入分享码" );
    // 过滤
    int code = txt.toInt();
    if( txt.length() != 9 || code < 100000000 ||code >= 1000000000 ) {
    QMessageBox::about( this , "提示", "分享码非法");
    return;
    }
    // 发送信号 什么目录下面 添加什么分享码的文件
    Q_EMIT SIG_getShareByLink( code , ui->lb_path->text() );
}

void Dialog::slot_deleteFile(bool flat)
{
    qDebug()<<__func__;
    //申请数组
    QVector<int> array;
    int count=ui->table_file->rowCount();

    //遍历所有项
    for(int i=0;i<count;i++)
    {
        MyTableWidgetItem * item0=( MyTableWidgetItem *)ui->table_file->item(i,0);
        //看是否打勾
        if(item0->checkState()==Qt::Checked)
        {
            //添加到数组中
            array.push_back(item0->m_info.fileid);
        }

    }

    //发送信号
    emit SIG_deleteFile(array,ui->lb_path->text());

}

void Dialog::slot_favoriteFile(bool flat)
{
    // 获取当前选中行
    int row = ui->table_file->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "收藏", "请先选中一个文件！");
        return;
    }
    MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_file->item(row, 0);
    if (item0) {
        if (item0->m_info.type == "file") {
            Q_EMIT SIG_addFavorite(ui->lb_path->text(), item0->m_info.name, item0->m_info.fileid);
        } else {
            QMessageBox::information(this, "收藏", "只能收藏文件，不能收藏文件夹！");
        }
    }
}

void Dialog::slot_restoreFile(bool flat)
{
    int row = ui->table_file->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "恢复", "请先选中一个文件！");
        return;
    }
    MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_file->item(row, 0);
    if (item0) {
        QVector<int> array;
        array.push_back(item0->m_info.fileid);
        Q_EMIT SIG_restoreFile(array);
    }
}

void Dialog::slot_delFavorite(bool flat)
{
    int row = ui->table_file->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "取消收藏", "请先选中一个文件！");
        return;
    }
    MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_file->item(row, 0);
    if (item0) {
        Q_EMIT SIG_delFavorite(item0->m_info.fileid);
    }
}

void Dialog::slot_uploadPause(bool flat)
{
    qDebug()<<"上传暂停";
    int row = ui->table_upload->currentRow();
    if (row < 0) return;
    MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_upload->item(row, 0);
    if (!item0) return;
    QPushButton * button = (QPushButton *)ui->table_upload->cellWidget(row, 5);
    if (button && button->text() == "暂停") {
        button->setText("开始");
        emit SIG_setuploadPause(item0->m_info.timestamp, 1);
    }
}

void Dialog::slot_uploadResume(bool flat)
{
    qDebug()<<"上传开始";
    int row = ui->table_upload->currentRow();
    if (row < 0) return;
    MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_upload->item(row, 0);
    if (!item0) return;
    QPushButton * button = (QPushButton *)ui->table_upload->cellWidget(row, 5);
    if (button && button->text() == "开始") {
        button->setText("暂停");
        emit SIG_setuploadPause(item0->m_info.timestamp, 0);
    }
}

void Dialog::slot_downloadPause(bool flat)
{
    qDebug()<<"下载暂停";
    int row = ui->table_download->currentRow();
    if (row < 0) return;
    MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_download->item(row, 0);
    if (!item0) return;
    QPushButton * button = (QPushButton *)ui->table_download->cellWidget(row, 5);
    if (button && button->text() == "暂停") {
        button->setText("开始");
        emit SIG_setdownloadPause(item0->m_info.timestamp, 1);
    }
}

void Dialog::slot_downloadResume(bool flat)
{
    qDebug()<<"下载开始";
    int row = ui->table_download->currentRow();
    if (row < 0) return;
    MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_download->item(row, 0);
    if (!item0) return;
    QPushButton * button = (QPushButton *)ui->table_download->cellWidget(row, 5);
    if (button && button->text() == "开始") {
        button->setText("暂停");
        emit SIG_setdownloadPause(item0->m_info.timestamp, 0);
    }
}

void Dialog::slot_uploadAllPause(bool flat)
{
    qDebug()<<"全部上传暂停";
    int rows = ui->table_upload->rowCount();
    for (int i = 0; i < rows; i++) {
        QPushButton * button = (QPushButton *)ui->table_upload->cellWidget(i, 5);
        if (button && button->text() == "暂停") {
            button->setText("开始");
            MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_upload->item(i, 0);
            if (item0) {
                qDebug()<<"暂停文件:"<<item0->m_info.name<<"timestamp:"<<item0->m_info.timestamp;
                emit SIG_setuploadPause(item0->m_info.timestamp, 1);
            }
        }
    }
}

void Dialog::slot_uploadAllResume(bool flat)
{
    qDebug()<<"全部上传开始";
    int rows = ui->table_upload->rowCount();
    for (int i = 0; i < rows; i++) {
        QPushButton * button = (QPushButton *)ui->table_upload->cellWidget(i, 5);
        if (button && button->text() == "开始") {
            button->setText("暂停");
            MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_upload->item(i, 0);
            if (item0) {
                qDebug()<<"开始文件:"<<item0->m_info.name<<"timestamp:"<<item0->m_info.timestamp;
                emit SIG_setuploadPause(item0->m_info.timestamp, 0);
            }
        }
    }
}

void Dialog::slot_downloadAllPause(bool flat)
{
    int rows = ui->table_download->rowCount();
    for (int i = 0; i < rows; i++) {
        QPushButton * button = (QPushButton *)ui->table_download->cellWidget(i, 5);
        if (button && button->text() == "暂停") {
            button->setText("开始");
            MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_download->item(i, 0);
            if (item0) emit SIG_setdownloadPause(item0->m_info.timestamp, 1);
        }
    }
}

void Dialog::slot_downloadAllResume(bool flat)
{
    int rows = ui->table_download->rowCount();
    for (int i = 0; i < rows; i++) {
        QPushButton * button = (QPushButton *)ui->table_download->cellWidget(i, 5);
        if (button && button->text() == "开始") {
            button->setText("暂停");
            MyTableWidgetItem* item0 = (MyTableWidgetItem*)ui->table_download->item(i, 0);
            if (item0) emit SIG_setdownloadPause(item0->m_info.timestamp, 0);
        }
    }
}


#include<QProgressBar>
//插入到上传中
void Dialog::slot_insertUploadFile(FileInfo &info)
{
    //表格插入信息

    //列：文件 大小 时间 速率 进度 按钮
    //1.新增一行 获取当前行+1 设置行数
    int rows=ui->table_upload->rowCount();
    ui->table_upload->setRowCount(rows+1);
    //2.设置这一行的每一列空间（添加对象）
    MyTableWidgetItem *item0 =new MyTableWidgetItem;
    item0->slot_setinfo(info);
    ui->table_upload->setItem(rows,0,item0);
    QTableWidgetItem *item1 =new QTableWidgetItem(FileInfo::getSize(info.size));
    ui->table_upload->setItem(rows,1,item1);
    QTableWidgetItem *item2 =new QTableWidgetItem(info.time);
    ui->table_upload->setItem(rows,2,item2);
    QTableWidgetItem *item3 =new QTableWidgetItem("0kb/s");
    ui->table_upload->setItem(rows,3,item3);
    //进度条
    QProgressBar * progress=new QProgressBar;
    progress->setMaximum(info.size);
    ui->table_upload->setCellWidget(rows,4,progress);
    //按钮
    QPushButton* button=new QPushButton;
    if(info.isPause==0)
    {
        button->setText("暂停");
    }
    else
    {
        button->setText("开始");
    }
    ui->table_upload->setCellWidget(rows,5,button);
}

void Dialog::slot_insertUploadComplete(FileInfo &info)
{
    //表格插入信息

    //列：文件 大小 时间 上传完成
    //1.新增一行 获取当前行+1 设置行数
    int rows=ui->table_complete->rowCount();
    ui->table_complete->setRowCount(rows+1);
    //2.设置这一行的每一列空间（添加对象）
    //文件
    MyTableWidgetItem *item0 =new MyTableWidgetItem;
    item0->slot_setinfo(info);
    ui->table_complete->setItem(rows,0,item0);
    //大小
    QTableWidgetItem *item1 =new QTableWidgetItem(FileInfo::getSize(info.size));
    ui->table_complete->setItem(rows,1,item1);
    //时间
    QTableWidgetItem *item2 =new QTableWidgetItem(info.time);
    ui->table_complete->setItem(rows,2,item2);
    //上传完成
    QTableWidgetItem *item3 =new QTableWidgetItem("上传完成");
    ui->table_complete->setItem(rows,3,item3);
}

void Dialog::slot_updateUploadFileProgress(int timestamp, int pos)
{
    //遍历所有项 第0列
    int row=ui->table_upload->rowCount();
    //取到每一个文件信息的时间戳 看是否一致
    for(int i=0;i<row;i++){

        MyTableWidgetItem *item0=(MyTableWidgetItem*)ui->table_upload->item(i,0);
        if(item0->m_info.timestamp==timestamp)
        {
             //一致 更新进度
            QProgressBar * item4=(QProgressBar *)ui->table_upload->cellWidget(i,4);
            item4->setValue(pos);
            //看是否结束
            if(item4->value()>=item4->maximum())
            {
                 //是 删除这一项 添加到完成
                slot_insertUploadComplete(item0->m_info);
                slot_deleteUploadFileByrow(i);
                //return
                return;
            }
        }




    }


}

void Dialog::slot_updateDownloadFileProgress(int timestamp, int pos)
{
    //遍历所有项 第0列
    int row=ui->table_download->rowCount();
    //取到每一个文件信息的时间戳 看是否一致
    for(int i=0;i<row;i++){

        MyTableWidgetItem *item0=(MyTableWidgetItem*)ui->table_download->item(i,0);
        if(item0->m_info.timestamp==timestamp)
        {
             //一致 更新进度
            QProgressBar * item4=(QProgressBar *)ui->table_download->cellWidget(i,4);
            item4->setValue(pos);
            //看是否结束
            if(item4->value()>=item4->maximum())
            {
                 //是 添加到完成 删除这一项
                slot_insertDownloadComplete(item0->m_info);
                slot_deleteDownloadFileByrow(i);
                //return
                return;
            }
        }
    }
}



void Dialog::slot_insertDownloadComplete(FileInfo &info)
{
    //表格插入信息

    //列：文件 大小 时间 上传完成
    //1.新增一行 获取当前行+1 设置行数
    int rows=ui->table_complete->rowCount();
    ui->table_complete->setRowCount(rows+1);
    //2.设置这一行的每一列空间（添加对象）
    //文件
    MyTableWidgetItem *item0 =new MyTableWidgetItem;
    item0->slot_setinfo(info);
    ui->table_complete->setItem(rows,0,item0);
    //大小
    QTableWidgetItem *item1 =new QTableWidgetItem(FileInfo::getSize(info.size));
    ui->table_complete->setItem(rows,1,item1);
    //时间
    QTableWidgetItem *item2 =new QTableWidgetItem(info.time);
    ui->table_complete->setItem(rows,2,item2);
    //上传完成
    QPushButton* button=new QPushButton;
    connect(button,SIGNAL(clicked(bool)),this,SLOT(slot_openPath(bool)));
    button->setIcon(QIcon(":/images/folder.png"));
    //设置扁平
    button->setFlat(true);
    //tooltip
    button->setToolTip(info.absolutePath);
    ui->table_complete->setCellWidget(rows,3,button);
}
#include<QProcess>
void Dialog::slot_openPath(bool flag)
{
    QPushButton* button=(QPushButton*)QObject::sender();
    QString path=button->toolTip();
    //jiang /转换 \\。。。
    path.replace('/','\\');
    qDebug()<<path;
    //如何打开文件夹
    //explorer /select, 路径

    //通过qt打开进程
    QProcess process;
    QStringList lst;
//    lst<<QString("/select,")<<path;
//    process.startDetached("explorer",lst);
    process.startDetached("explorer ", QStringList() << "/select," + path);
}

void Dialog::slot_deleteAllFileInfo()
{
    //ui->table_file->clear(); //删文字 不用这个 行数不变

    int rows = ui->table_file->rowCount();
    qDebug()<<rows;
    for( int i = rows -1 ; i >= 0 ; i--) {
        ui->table_file->removeRow(i) ;
    }
}

void Dialog::slot_deleteShareAllFileInfo()
{
    //ui->table_myshare->clear(); //删文字 不用这个 行数不变

    int rows = ui->table_myshare->rowCount();
    qDebug()<<rows;
    for( int i = rows -1 ; i >= 0 ; i--) {
        ui->table_myshare->removeRow(i) ;
    }
}
void Dialog::slot_deleteUploadFileByrow(int row)
{
    ui->table_upload->removeRow(row);
}
void Dialog::slot_deleteDownloadFileByrow(int row)
{
    ui->table_download->removeRow(row);
}

void Dialog::slot_insertFileInfo(FileInfo &info)
{
    //表格插入信息

    //列：文件 大小 时间
    //1.新增一行 获取当前行+1 设置行数
    int rows=ui->table_file->rowCount();
    ui->table_file->setRowCount(rows+1);
    //2.设置这一行的每一列空间（添加对象）
    //文件
    MyTableWidgetItem *item0 =new MyTableWidgetItem;
    item0->slot_setinfo(info);
    ui->table_file->setItem(rows,0,item0);
    //大小
    QString strSize;
    if(info.type=="file")
    {
        strSize=FileInfo::getSize(info.size);
    }
    else
    {
        strSize=" ";
    }
    QTableWidgetItem *item1 =new QTableWidgetItem(strSize);
    ui->table_file->setItem(rows,1,item1);
    //时间
    QTableWidgetItem *item2 =new QTableWidgetItem(info.time);
    ui->table_file->setItem(rows,2,item2);
}

void Dialog::slot_insertShareFileInfo(QString name, int size, QString time, int shareLink)
{
    //表格插入信息

    //列：文件 大小 时间 fenxiangma
    //1.新增一行 获取当前行+1 设置行数
    int rows=ui->table_myshare->rowCount();
    ui->table_myshare->setRowCount(rows+1);
    //2.设置这一行的每一列空间（添加对象）
    QTableWidgetItem *item0 =new QTableWidgetItem(name);
    ui->table_myshare->setItem(rows,0,item0);
    QTableWidgetItem *item1 =new QTableWidgetItem(FileInfo::getSize(size));
    ui->table_myshare->setItem(rows,1,item1);
    QTableWidgetItem *item2 =new QTableWidgetItem(time);
    ui->table_myshare->setItem(rows,2,item2);
    QTableWidgetItem *item3 =new QTableWidgetItem(QString::number(shareLink));
    ui->table_myshare->setItem(rows,3,item3);

}

void Dialog::slot_insertDownloadFile(FileInfo &info)
{
    //表格插入信息

    //列：文件 大小 时间 速率 进度 按钮
    //1.新增一行 获取当前行+1 设置行数
    int rows=ui->table_download->rowCount();
    ui->table_download->setRowCount(rows+1);
    //2.设置这一行的每一列空间（添加对象）
    MyTableWidgetItem *item0 =new MyTableWidgetItem;
    item0->slot_setinfo(info);
    ui->table_download->setItem(rows,0,item0);
    QTableWidgetItem *item1 =new QTableWidgetItem(FileInfo::getSize(info.size));
    ui->table_download->setItem(rows,1,item1);
    QTableWidgetItem *item2 =new QTableWidgetItem(info.time);
    ui->table_download->setItem(rows,2,item2);
    QTableWidgetItem *item3 =new QTableWidgetItem("0kb/s");
    ui->table_download->setItem(rows,3,item3);
    //进度条
    QProgressBar * progress=new QProgressBar;
    progress->setMaximum(info.size);
    ui->table_download->setCellWidget(rows,4,progress);
    //按钮
    QPushButton* button=new QPushButton;
    if(info.isPause==0)
    {
        button->setText("暂停");
    }
    else
    {
        button->setText("开始");
    }
    ui->table_download->setCellWidget(rows,5,button);
}

//选中某一行
void Dialog::on_table_file_cellClicked(int row, int column)
{
    //切换勾选和未勾选
    MyTableWidgetItem* item0=(MyTableWidgetItem*)ui->table_file->item(row,0);
    if(item0->checkState()==Qt::Checked)
    {
        item0->setCheckState(Qt::Unchecked);
    }
    else
    {
         item0->setCheckState(Qt::Checked);
    }
}

//表格位置鼠标右键
void Dialog::on_table_file_customContextMenuRequested(const QPoint &pos)
{
    // 选中右键点击的行
    QTableWidgetItem* item = ui->table_file->itemAt(pos);
    if (item) {
        int row = item->row();
        ui->table_file->selectRow(row);
    }
    //弹出菜单
    m_menuFileInfo.exec(QCursor::pos());
}


void Dialog::on_table_file_cellDoubleClicked(int row, int column)
{
    //首先 拿到双击那行的文件名字
    MyTableWidgetItem* item0=(MyTableWidgetItem*)ui->table_file->item(row,0);
    //判断是不是文件夹（未来 是文件可以打开文件）
    if(item0->m_info.type!="file")
    {
        //是文件夹 路径 拼接
        QString dir=ui->lb_path->text()+item0->m_info.name+"/";
        //设置路径 lb_path ->text
        ui->lb_path->setText(dir);
        //发送信号---> 更新当前目录 ->刷新文件列表
        emit SIG_changeDir(dir);
    }

}


void Dialog::on_pb_prev_clicked()
{
    //获取目录
    QString path=ui->lb_path->text();
    //判断”/“结束
    if(path=="/") return;
    //首先找到最右边"/" 从他左边 开始再想右找 找"/"
    path=path.left(path.lastIndexOf("/"));
    //新的目录 就是找到的"/" 以及他左边的所有字符
    path=path.left(path.lastIndexOf("/")+1);
    qDebug()<<path;
    ui->lb_path->setText(path);
    //跳转路径
    emit SIG_changeDir(path);
}


void Dialog::on_table_upload_cellClicked(int row, int column)
{
    //切换勾选和未勾选
    MyTableWidgetItem* item0=(MyTableWidgetItem*)ui->table_upload->item(row,0);
    if(item0->checkState()==Qt::Checked)
    {
        item0->setCheckState(Qt::Unchecked);
    }
    else
    {
         item0->setCheckState(Qt::Checked);
    }
}


void Dialog::on_table_download_cellClicked(int row, int column)
{
    //切换勾选和未勾选
    MyTableWidgetItem* item0=(MyTableWidgetItem*)ui->table_download->item(row,0);
    if(item0->checkState()==Qt::Checked)
    {
        item0->setCheckState(Qt::Unchecked);
    }
    else
    {
         item0->setCheckState(Qt::Checked);
    }
}

bool Dialog::slot_getDownlaodFileInfoByTimesatmp(int timestamp,FileInfo& info)
{
    //遍历所有第0列
    int rows=ui->table_download->rowCount();

    for(int i=0;i<rows;i++)
    {
        MyTableWidgetItem * item0=(MyTableWidgetItem *)ui->table_download->item(i,0);
        if(item0->m_info.timestamp==timestamp)
        {
            info=item0->m_info;
            return true;
        }
    }
    return false;
}

bool Dialog::slot_getUplaodFileInfoByTimesatmp(int timestamp,FileInfo& info)
{
    //遍历所有第0列
    int rows=ui->table_upload->rowCount();

    for(int i=0;i<rows;i++)
    {
        MyTableWidgetItem * item0=(MyTableWidgetItem *)ui->table_upload->item(i,0);
        if(item0->m_info.timestamp==timestamp)
        {
            info=item0->m_info;
            return true;
        }
    }
    return false;
}

