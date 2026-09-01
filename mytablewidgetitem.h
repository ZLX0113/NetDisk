#ifndef MYTABLEWIDGETITEM_H
#define MYTABLEWIDGETITEM_H

#include <QTableWidgetItem>
#include"common.h"
class CKernel;
class Dialog;
class MyTableWidgetItem : public QTableWidgetItem
{

public:
    MyTableWidgetItem();
public slots:
    void slot_setinfo(FileInfo& info);
private:
    FileInfo m_info;
    friend class CKernel;
    friend class Dialog;
};

#endif // MYTABLEWIDGETITEM_H
