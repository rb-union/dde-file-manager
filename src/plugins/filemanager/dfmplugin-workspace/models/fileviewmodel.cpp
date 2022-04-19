/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fileviewmodel.h"
#include "private/fileviewmodel_p.h"
#include "views/fileview.h"
#include "utils/workspacehelper.h"
#include "utils/fileoperatorhelper.h"
#include "workspace/workspace_defines.h"

#include "dfm-base/utils/fileutils.h"

#include <dfm-framework/framework.h>

#include <QApplication>
#include <QPointer>
#include <QList>
#include <QMimeData>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

FileViewModelPrivate::FileViewModelPrivate(FileViewModel *qq)
    : QObject(qq), q(qq)
{
    nodeManager.reset(new FileNodeManagerThread(qq));

    updateTimer.setInterval(15);
    updateTimer.setSingleShot(true);
    connect(&updateTimer, &QTimer::timeout, qq, &FileViewModel::onFilesUpdated);
}

FileViewModelPrivate::~FileViewModelPrivate()
{
}

// 文件在当前目录下创建、修改、删除都需要顺序处理，并且都是在遍历目录完成后才有序的处理
// 处理方式，收到这3个事件，都加入到事件处理队列
// 判断当前遍历目录是否完成，是-启动异步文件监视事件处理，否-文件遍历完成时，启动异步文件监视事件处理
// 在退出时清理异步事件处理，再次进行fetchmore时（遍历新目录时）清理文件监视事件。
void FileViewModelPrivate::doFileDeleted(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        watcherEvent.enqueue(QPair<QUrl, EventType>(url, kRmFile));
    }
    //    if (isUpdatedChildren)
    //        return;
    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void FileViewModelPrivate::dofileCreated(const QUrl &url)
{
    {
        QMutexLocker lk(&watcherEventMutex);
        watcherEvent.enqueue(QPair<QUrl, EventType>(url, kAddFile));
    }
    // Todo(liyigang):isUpdatedChildren
    //    if (isUpdatedChildren)
    //        return;
    metaObject()->invokeMethod(this, QT_STRINGIFY(doWatcherEvent), Qt::QueuedConnection);
}

void FileViewModelPrivate::doFileUpdated(const QUrl &url)
{
    //     Todo(yanghao): filter .hidden file update
    if (!updateUrlList.containsByLock(url))
        updateUrlList.appendByLock(url);

    if (!updateTimer.isActive())
        updateTimer.start();
}

void FileViewModelPrivate::doFilesUpdated()
{
}

void FileViewModelPrivate::doUpdateChild(const QUrl &child)
{
    nodeManager->insertChild(child);
}

void FileViewModelPrivate::doWatcherEvent()
{
    // Todo(liyigang):isUpdatedChildren
    //    if (isUpdatedChildren)
    //        return;

    if (processFileEventRuning)
        return;

    processFileEventRuning = true;
    while (checkFileEventQueue()) {
        QPair<QUrl, EventType> event;
        {
            QMutexLocker lk(&watcherEventMutex);
            event = watcherEvent.dequeue();
        }
        const QUrl &fileUrl = event.first;

        if (!fileUrl.isValid())
            continue;

        if (fileUrl == root->url()) {
            if (event.second == kAddFile)
                continue;
            //todo:先不做
        }

        // Todo(liyigang): parentUrl
        // if (UrlRoute::urlParent(fileUrl) != root->url())
        //    continue;

        if (event.second == kAddFile) {
            nodeManager->insertChild(fileUrl);
        } else {
            nodeManager->removeChildren(fileUrl);
            dpfInstance.eventDispatcher().publish(DSB_FM_NAMESPACE::Workspace::EventType::kCloseTabs, fileUrl);
        }
    }
    q->childrenUpdated();
    processFileEventRuning = false;
}

bool FileViewModelPrivate::checkFileEventQueue()
{
    QMutexLocker lk(&watcherEventMutex);
    bool isEmptyQueue = watcherEvent.isEmpty();
    return !isEmptyQueue;
}

FileViewModel::FileViewModel(QAbstractItemView *parent)
    : QAbstractItemModel(parent), d(new FileViewModelPrivate(this))
{
}

FileViewModel::~FileViewModel()
{
    clear();
}

QModelIndex FileViewModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (row < 0 || column < 0 || d->nodeManager->childrenCount() <= row)
        return QModelIndex();

    return createIndex(row, column, d->nodeManager->childByIndex(row).data());
}

const FileViewItem *FileViewModel::itemFromIndex(const QModelIndex &index) const
{
    if (index.internalPointer() == d->root.data())
        return d->root.data();

    if (0 > index.row() || index.row() >= d->nodeManager->childrenCount())
        return nullptr;

    FileNodePointer child = d->nodeManager->childByIndex(index.row());
    if (child.isNull())
        return nullptr;

    return d->nodeManager->childByIndex(index.row()).data();
}

QModelIndex FileViewModel::setRootUrl(const QUrl &url)
{
    clear();

    if (!d->root.isNull() && d->root->url() == url) {
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
        QModelIndex root = createIndex(0, 0, d->root.data());

        d->canFetchMoreFlag = true;
        //        fetchMore(root);

        return root;
    }

    if (!url.isValid())
        return QModelIndex();

    d->root.reset(new FileViewItem(nullptr, url));
    d->nodeManager->setRootNode(d->root);

    QModelIndex root = createIndex(0, 0, d->root.data());

    if (d->column == 0)
        d->column = 4;

    if (!d->watcher.isNull()) {
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileDeleted,
                   d.data(), &FileViewModelPrivate::doFileDeleted);
        disconnect(d->watcher.data(), &AbstractFileWatcher::subfileCreated,
                   d.data(), &FileViewModelPrivate::dofileCreated);
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
                   d.data(), &FileViewModelPrivate::doFileUpdated);
        disconnect(d->watcher.data(), &AbstractFileWatcher::fileRename,
                   d.data(), &FileViewModelPrivate::dofileMoved);
    }
    d->watcher = WatcherFactory::create<AbstractFileWatcher>(url);
    if (!d->watcher.isNull()) {
        connect(d->watcher.data(), &AbstractFileWatcher::fileDeleted,
                d.data(), &FileViewModelPrivate::doFileDeleted);
        connect(d->watcher.data(), &AbstractFileWatcher::subfileCreated,
                d.data(), &FileViewModelPrivate::dofileCreated);
        connect(d->watcher.data(), &AbstractFileWatcher::fileAttributeChanged,
                d.data(), &FileViewModelPrivate::doFileUpdated);
        connect(d->watcher.data(), &AbstractFileWatcher::fileRename,
                d.data(), &FileViewModelPrivate::dofileMoved);

        d->watcher->startWatcher();
    }

    d->canFetchMoreFlag = true;
    return root;
}

QUrl FileViewModel::rootUrl() const
{
    return d->root->fileInfo()->url();
}

QModelIndex FileViewModel::rootIndex() const
{
    return createIndex(0, 0, d->root.data());
}

const FileViewItem *FileViewModel::rootItem() const
{
    return d->root.data();
}

AbstractFileInfoPointer FileViewModel::fileInfo(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    if (index.row() < 0 || d->nodeManager->childrenCount() <= index.row())
        return nullptr;
    FileNodePointer child = d->nodeManager->childByIndex(index.row());
    if (child.isNull())
        return nullptr;
    return child->fileInfo();
}

QModelIndex FileViewModel::parent(const QModelIndex &child) const
{
    FileViewItem *parentItem = static_cast<FileViewItem *>(child.internalPointer());
    if (!parentItem || parentItem == d->nodeManager->rootNode().data())
        return QModelIndex();

    return createIndex(0, 0, parentItem);
}

int FileViewModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->nodeManager->childrenCount();
}

int FileViewModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->column;
}

QVariant FileViewModel::data(const QModelIndex &index, int role) const
{
    auto item = itemFromIndex(index);
    if (item)
        return item->data(role);
    return QVariant();
}

void FileViewModel::clear()
{
    d->nodeManager->disconnect(d->nodeManager.data());
    d->nodeManager->stop();
    beginRemoveRows(rootIndex(), 0, d->nodeManager->childrenCount() - 1);
    d->nodeManager->clearChildren();
    endRemoveRows();
}
/*!
 * \brief FileViewModel::rowCountMaxShow
 * \return int 当前view最多展示多少行的item项
 */
int FileViewModel::rowCountMaxShow()
{
    // 优化思路，初始化计算行数，model设置相关行数，随后向下拉view时进行动态insert和界面刷新。
    // 避免一次性载入多个文件的长时间等待。
    auto view = qobject_cast<QAbstractItemView *>(QObject::parent());
    auto beginIndex = view->indexAt(QPoint { 1, 1 });
    auto currViewHeight = view->size().height();
    auto currIndexHeight = beginIndex.data(Qt::SizeHintRole).toSize().height();
    if (currIndexHeight <= 0)
        return -1;
    return (currViewHeight / currIndexHeight) + 1;
}

void FileViewModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)

    if (!canFetchMore(parent))
        return;
    d->canFetchMoreFlag = false;

    auto url = d->root->url();
    auto prehandler = WorkspaceHelper::instance()->viewRoutePrehandler(url.scheme());
    if (prehandler) {
        QPointer<FileViewModel> guard(this);
        prehandler(url, [=]() {
            if (guard)
                this->traversCurrDir(); });
    } else {
        traversCurrDir();
    }
}

bool FileViewModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->canFetchMoreFlag;
}

Qt::ItemFlags FileViewModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    const FileViewItem *item = itemFromIndex(index);
    if (!item)
        return flags;

    flags |= Qt::ItemIsDragEnabled;

    const AbstractFileInfoPointer &fileInfo = item->fileInfo();
    if (fileInfo && fileInfo->canRename()) {
        flags |= Qt::ItemIsEditable;
    }
    if (fileInfo && fileInfo->isWritable()) {
        if (fileInfo->canDrop())
            flags |= Qt::ItemIsDropEnabled;
        else
            flags |= Qt::ItemNeverHasChildren;
    }

    return flags;
}

QStringList FileViewModel::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

QMimeData *FileViewModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    QSet<QUrl> urlsSet;
    QList<QModelIndex>::const_iterator it = indexes.begin();

    for (; it != indexes.end(); ++it) {
        if ((*it).column() == 0) {
            const AbstractFileInfoPointer &fileInfo = this->fileInfo(*it);
            const QUrl &url = fileInfo->url();

            if (urlsSet.contains(url))
                continue;

            urls << url;
            urlsSet << url;
        }
    }

    QMimeData *data = new QMimeData();
    data->setUrls(urls);

    return data;
}

bool FileViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row)
    Q_UNUSED(column)

    if (!parent.isValid())
        return false;

    QUrl targetUrl = itemFromIndex(parent)->url();
    AbstractFileInfoPointer targetFileInfo = itemFromIndex(parent)->fileInfo();
    const QList<QUrl> dropUrls = data->urls();

    if (targetFileInfo->isSymLink()) {
        // TODO: trans 'targetUrl' to source url
    }

    bool ret { true };

    switch (action) {
    case Qt::CopyAction:
        if (dropUrls.count() > 0) {
            // call copy
            FileView *view = qobject_cast<FileView *>(qobject_cast<QObject *>(this)->parent());
            FileOperatorHelperIns->dropFiles(view, Qt::CopyAction, targetUrl, dropUrls);
        }
        break;
    case Qt::MoveAction:
        if (dropUrls.count() > 0) {
            // call move
            FileView *view = qobject_cast<FileView *>(qobject_cast<QObject *>(this)->parent());
            FileOperatorHelperIns->dropFiles(view, Qt::MoveAction, targetUrl, dropUrls);
        }
        break;
    default:
        break;
    }

    return ret;
}

Qt::DropActions FileViewModel::supportedDragActions() const
{
    if (d->root)
        return d->root->fileInfo()->supportedDragActions();

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions FileViewModel::supportedDropActions() const
{
    if (d->root)
        return d->root->fileInfo()->supportedDropActions();

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

AbstractFileWatcherPointer FileViewModel::fileWatcher() const
{
    return d->watcher;
}

void FileViewModel::beginInsertRows(const QModelIndex &parent, int first, int last)
{
    QAbstractItemModel::beginInsertRows(parent, first, last);
}

void FileViewModel::endInsertRows()
{
    QAbstractItemModel::endInsertRows();
}

void FileViewModel::beginRemoveRows(const QModelIndex &parent, int first, int last)
{
    QAbstractItemModel::beginRemoveRows(parent, first, last);
}

void FileViewModel::endRemoveRows()
{
    QAbstractItemModel::endRemoveRows();
}

void FileViewModel::beginResetModel()
{
    QAbstractItemModel::beginResetModel();
}

void FileViewModel::endResetModel()
{
    QAbstractItemModel::endResetModel();
}

FileViewModel::State FileViewModel::state() const
{
    return d->currentState;
}

void FileViewModel::setState(FileViewModel::State state)
{
    if (d->currentState == state)
        return;

    d->currentState = state;

    emit stateChanged();
}

void FileViewModel::childrenUpdated()
{
    emit modelChildrenUpdated();
}

void FileViewModel::traversCurrDir()
{
    d->isUpdatedChildren = false;
    if (!d->traversalThread.isNull()) {
        d->traversalThread.data()->disconnect();
        d->traversalThread->stopAndDeleteLater();
        d->traversalThread->setParent(nullptr);
    }

    d->traversalThread = new TraversalDirThread(
            d->root->url(), QStringList(),
            QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden,
            QDirIterator::NoIteratorFlags);

    d->traversalThread->setParent(this);

    if (d->traversalThread.isNull()) {
        d->isUpdatedChildren = true;
        return;
    }

    d->nodeManager->connect(d->traversalThread.data(), &TraversalDirThread::updateChild,
                            d->nodeManager.data(), &FileNodeManagerThread::onHandleAddFile,
                            Qt::QueuedConnection);
    d->nodeManager->connect(d->traversalThread.data(), &QThread::finished,
                            d->nodeManager.data(), &FileNodeManagerThread::onHandleTraversalFinished,
                            Qt::QueuedConnection);

    setState(Busy);
    d->traversalThread->start();
}

void FileViewModel::onFilesUpdated()
{
    emit updateFiles();
}

FileNodeManagerThread::FileNodeManagerThread(FileViewModel *parent)
    : QThread(parent)
{
}

FileNodeManagerThread::~FileNodeManagerThread()
{
    stop();
}

void FileNodeManagerThread::onHandleAddFile(const QUrl url)
{
    {
        QMutexLocker lk(&fileQueueMutex);
        fileQueue.enqueue(url);
    }
    if (!isRunning()) {
        stoped = false;
        start();
    }
}

void FileNodeManagerThread::onHandleTraversalFinished()
{
    isTraversalFinished = true;
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

    if (!isRunning())
        model()->setState(FileViewModel::Idle);
}

void FileNodeManagerThread::removeFile(const QUrl &url)
{
    QMutexLocker lk(&fileQueueMutex);
    fileQueue.removeAll(url);
}

void FileNodeManagerThread::stop()
{
    {
        QMutexLocker lk(&fileQueueMutex);
        fileQueue.clear();
    }
    stoped = true;
    wait();
    cacheChildren = false;
    isTraversalFinished = false;
    stoped = false;

    model()->setState(FileViewModel::Idle);
}

void FileNodeManagerThread::clearChildren()
{
    childrenMutex.lock();
    children.clear();
    visibleChildren.clear();
    childrenMutex.unlock();
}

FileNodePointer FileNodeManagerThread::rootNode() const
{
    return root;
}

void FileNodeManagerThread::insertChild(const QUrl &url)
{
    int row = -1;
    {
        QMutexLocker lk(&childrenMutex);
        if (children.contains(url))
            return;
        row = children.count();
    }
    model()->beginInsertRows(QModelIndex(), row, row);
    FileNodePointer item(new FileViewItem(root.data(), url));
    {
        QMutexLocker lk(&childrenMutex);
        if (cacheChildren)
            childrenAddMap.insert(url, item);
        visibleChildren.append(item);
        children.insert(url, item);
    }
    model()->endInsertRows();
}

bool FileNodeManagerThread::insertChildren(QList<QUrl> &urls)
{
    cacheChildren = true;
    childrenMutex.lock();
    QList<FileNodePointer> tempVisible = visibleChildren;
    QMap<QUrl, FileNodePointer> tempChildren = children;
    childrenMutex.unlock();

    if (stoped)
        return false;

    for (const auto &url : urls) {
        if (stoped)
            return false;
        FileNodePointer needNode(new FileViewItem(root.data(), url));
        const AbstractFileInfoPointer &needNodeInfo = needNode->fileInfo();
        if (needNodeInfo.isNull())
            continue;
        tempVisible.append(needNode);
        tempChildren.insert(needNodeInfo->url(), needNode);
    }

    model()->beginResetModel();

    childrenMutex.lock();
    cacheChildren = false;
    for (const auto &key : childrenAddMap.keys()) {
        if (tempChildren.contains(key))
            continue;
        tempChildren.insert(key, childrenAddMap.value(key));
        tempVisible.append(childrenAddMap.value(key));
    }
    for (const auto &key : childrenRemoveMap.keys()) {
        tempChildren.remove(key);
        tempVisible.removeOne(childrenRemoveMap.value(key));
    }
    visibleChildren = tempVisible;
    children = tempChildren;
    childrenMutex.unlock();

    model()->endResetModel();

    urls.clear();

    return !stoped;
}

void FileNodeManagerThread::insertAllChildren(const QList<QUrl> &urls)
{
    childrenMutex.lock();
    QList<FileNodePointer> tempVisible = visibleChildren;
    QMap<QUrl, FileNodePointer> tempChildren = children;
    childrenMutex.unlock();
    int row = -1;
    for (const auto &url : urls) {
        if (tempChildren.contains(url))
            continue;
        FileNodePointer needNode(new FileViewItem(root.data(), url));
        const AbstractFileInfoPointer &needNodeInfo = needNode->fileInfo();
        if (needNodeInfo.isNull())
            continue;
        row = tempVisible.count();
        tempVisible.insert(row, needNode);
        tempChildren.insert(needNodeInfo->url(), needNode);
    }

    model()->beginResetModel();

    childrenMutex.lock();
    visibleChildren = tempVisible;
    children = tempChildren;
    childrenMutex.unlock();

    model()->endResetModel();
}

void FileNodeManagerThread::removeChildren(const QUrl &url)
{
    int fileIndex = visibleChildren.indexOf(children.value(url));
    if (fileIndex == -1)
        return;

    model()->beginRemoveRows(QModelIndex(), fileIndex, fileIndex);
    {
        QMutexLocker lk(&childrenMutex);
        if (cacheChildren)
            childrenRemoveMap.insert(url, children.value(url));
        visibleChildren.removeOne(children.take(url));
    }
    model()->endRemoveRows();
}

int FileNodeManagerThread::childrenCount()
{
    QMutexLocker lk(&childrenMutex);
    return visibleChildren.count();
}

FileNodePointer FileNodeManagerThread::childByIndex(const int &index)
{
    FileNodePointer child { nullptr };
    if (index >= 0 && index < visibleChildren.size()) {
        QMutexLocker lk(&childrenMutex);
        child = visibleChildren.at(index);
    }
    return child;
}

bool FileNodeManagerThread::fileQueueEmpty()
{
    QMutexLocker lk(&fileQueueMutex);
    return fileQueue.isEmpty();
}

int FileNodeManagerThread::fileQueueCount()
{
    QMutexLocker lk(&fileQueueMutex);
    return fileQueue.count();
}

QUrl FileNodeManagerThread::dequeueFileQueue()
{
    QMutexLocker lk(&fileQueueMutex);
    if (fileQueue.isEmpty())
        return QUrl();
    return fileQueue.dequeue();
}

void FileNodeManagerThread::run()
{
    if (stoped)
        return;

    if (!insertChildrenByCeiled())
        return;

    QUrl url;

    QList<QUrl> needInsertList;

    while (!fileQueueEmpty() || !isTraversalFinished) {
        if (stoped)
            return;

        url = dequeueFileQueue();
        if (!url.isValid())
            continue;

        if (!isTraversalFinished) {
            insertChild(url);
        } else if (isTraversalFinished && !fileQueueEmpty()) {
            needInsertList.append(url);
        } else {
            needInsertList.append(url);
            if (!insertChildren(needInsertList)) {
                model()->setState(FileViewModel::Idle);
                return;
            }
        }
    }
    if (needInsertList.count() > 0) {
        insertChildren(needInsertList);
    }

    model()->setState(FileViewModel::Idle);
}

bool FileNodeManagerThread::insertChildrenByCeiled()
{
    if (stoped)
        return false;
    QTime ceilTime;
    ceilTime.start();
    QList<QUrl> files;
    while (!isTraversalFinished && ceilTime.elapsed() < timeCeiling && files.count() < countCeiling) {
        if (stoped)
            return false;
        QUrl file = dequeueFileQueue();
        if (file.isValid())
            files << file;
    }

    return insertChildren(files);
}
