# Debug Session: upload-pause-resume-bug

## Status: [FIXED - 待用户验证]

## Symptoms
- 点击单个文件暂停 → 所有文件都暂停
- 点击全部暂停 → 所有文件暂停
- 暂停后，点击任何文件的"开始" → 无反应，文件不会继续上传

## Root Cause（已通过静态分析确认）

暂停/恢复机制使用了「忙等死循环」写在数据响应处理函数里：

```cpp
// slot_dealFileContentRS（上传）/ slot_dealFileContentRQ（下载）中
while(info.isPause)
{
    QThread::msleep(100);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    if(m_quit) return;
}
```

导致：
1. **单个暂停→全部暂停**：某个文件暂停后，主线程卡在这个死循环里反复
   `msleep(100)` + `processEvents()`，把整个上传事件循环拖住，其它文件的
   数据响应只能在这个循环的 `processEvents` 窗口里被处理，吞吐被严重限流，
   表现为"所有文件都暂停"。
2. **恢复不生效**：恢复靠死循环"自己发现 isPause 标志变化"，但重入嵌套的
   事件循环使恢复点击处理不可靠。

## Fix（已实现）

改为事件驱动，彻底去掉忙等：

1. `slot_dealFileContentRS`（上传）：`while(info.isPause){...}` → `if(info.isPause) return;`
   —— 暂停时不再发送下一个文件块。
2. `slot_dealFileContentRQ`（下载）：同上改为 `if(info.isPause) return;`
   —— 暂停时不写入、不回复。
3. `slot_setuploadPause`：恢复（isPause==0）且文件在内存中时，额外发送
   `STRU_CONTINUE_UPLOAD_RQ` 续传协议，服务器回位置后继续。
4. `slot_setdownloadPause`：恢复（isPause==0）且文件在内存中时，额外发送
   `STRU_CONTINUE_DOWNLOAD_RQ` 续传协议。
5. 右键菜单改用 `indexAt(pos).row()`（替代 `itemAt`），保证右键点击到按钮/
   进度条等 cellWidget 时也能正确选中行。
6. 清理 `slot_uploadPause` 中残留的调试写文件代码。

## 修改文件
- c:\wangpan\NetDisk\ckernel.cpp
- c:\wangpan\NetDisk\maindialog.cpp

## 待验证
用户在 Qt Creator 中重新编译（确认输出到 build_release/release/NetDisk.exe），
重新运行客户端测试：单个暂停只暂停单个文件，恢复后继续上传。
