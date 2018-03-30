#ifndef CONTACTTREEWIDGET_H
#define CONTACTTREEWIDGET_H

#include <memory>
#include <QString>
#include <QTreeWidget>

//////////////////////////////////////////////////////////////////////////
///<summary>联系人左侧树类 </summary>
///
///<remarks> 2018.03.28 --朱正天  </remarks>/////////////////////////////
///////////////////////////////////////////////////////////////////////////
class ContactSheet;
class ContactPerson;
class ContactGroup;
class QTreeWidgetItem;
class QMouseEvent;
class QContextMenuEvent;

class ContactTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    ContactTreeWidget();
    ~ContactTreeWidget();
signals:
    //显示联系人信息事件
    void ShowContactPerson(const QString &address);

    //组修改信号（增删改）
    void GroupModifyFinishedSignal();
public slots:
    //执行查询（会选中符合查询的联系人）
    void QueryPersonSlots(const QString &queryString);

    //添加联系人（外部添加联系人，自动查询所属分组，添加节点）
    void AddPersonSlots(const QString &address);
private:
    bool addGroup(const QString &groupName = "NewGroup");
    bool editGroup(QTreeWidgetItem *topItem);
    bool delGroup(QTreeWidgetItem *topItem);

    void addPerson(const std::shared_ptr<ContactPerson> &person = nullptr,QTreeWidgetItem *groupItem = nullptr);
    void editPerson(QTreeWidgetItem *personItem);
    void delPerson(QTreeWidgetItem *personItem);

    void moveToGroup(QTreeWidgetItem *sourceItem,QTreeWidgetItem *targetItem);

private slots:
    void addGroupSlots(bool checked);
    void editGroupSlots(bool checked);
    void delGroupSlots(bool checked);

    void editPersonSlots(bool checked);
    void delPersonSlots(bool checked);

    void moveToGroupSlots(bool checked);

    void editGroupFinishSlots();
public:
    //////////////////////////////////////////////////////////////////////////
    ///<summary>设置联系人数据 </summary>
    ///
    ///<remarks> 2018.03.28 --朱正天  </remarks>/////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    void initContactSheet(std::shared_ptr<ContactSheet> data);
private:
    //////////////////////////////////////////////////////////////////////////
    ///<summary>初始化数据，界面 </summary>
    ///
    ///<remarks> 2018.03.28 --朱正天  </remarks>/////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    void initTreeData();
    void initTreeStyle();

    //////////////////////////////////////////////////////////////////////////
    ///<summary>初始化右键菜单 </summary>
    ///
    ///<remarks> 2018.03.28 --朱正天  </remarks>/////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    void initContextMenu();
protected:
    void contextMenuEvent(QContextMenuEvent *event);

    void mousePressEvent(QMouseEvent *event);
private:
    //////////////////////////////////////////////////////////////////////////
    ///<summary>根据数据生成节点 </summary>
    ///
    ///<remarks> 2018.03.29 --朱正天  </remarks>/////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    QTreeWidgetItem *createItemWithPerson(const std::shared_ptr<ContactPerson> &person)const;
    QTreeWidgetItem *createItemWithGroup(const std::shared_ptr<ContactGroup> &group)const;
private:
    class ContactTreeWidgetPrivate;
    ContactTreeWidgetPrivate *_p;
};

#endif // CONTACTTREEWIDGET_H