/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "computerview.h"
#include "private/computerview_p.h"
#include "models/computermodel.h"
#include "delegate/computeritemdelegate.h"
#include "utils/computerutils.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/utils/devicemanager.h"

#include <services/common/dialog/dialogservice.h>
#include <dfm-framework/framework.h>

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QtConcurrent>

DPCOMPUTER_BEGIN_NAMESPACE

ComputerView::ComputerView(const QUrl &url, QWidget *parent)
    : DListView(parent),
      dp(new ComputerViewPrivate(this))
{
    initView();
    initConnect();

    setRootUrl(url);
}

ComputerView::~ComputerView()
{
}

QWidget *ComputerView::widget() const
{
    return const_cast<ComputerView *>(this);
}

QUrl ComputerView::rootUrl() const
{
    return ComputerUtils::rootUrl();
}

dfmbase::AbstractBaseView::ViewState ComputerView::viewState() const
{
    return dfmbase::AbstractBaseView::ViewState::kViewIdle;
}

bool ComputerView::setRootUrl(const QUrl &url)
{
    // TODO(xust)
    return true;
}

QList<QAction *> ComputerView::toolBarActionList() const
{
    return {};
}

void ComputerView::refresh()
{
    // TODO(xust)
}

QList<QUrl> ComputerView::selectedUrlList() const
{
    // TODO(xust)
    return {};
}

bool ComputerView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease && watched == this->viewport()) {
        auto me = static_cast<QMouseEvent *>(event);
        const auto &idx = this->indexAt(me->pos());
        if (me->button() == Qt::MouseButton::LeftButton && (!idx.isValid() || !(idx.flags() & Qt::ItemFlag::ItemIsEnabled))) {
            this->selectionModel()->clearSelection();
        }
        return false;
    } else if (event->type() == QEvent::KeyPress && watched == this) {
        auto ke = static_cast<QKeyEvent *>(event);
        if (ke->modifiers() == Qt::Modifier::ALT) {
            this->event(event);
            return true;
        }
        if (ke->key() == Qt::Key::Key_Enter || ke->key() == Qt::Key::Key_Return) {
            const auto &idx = this->selectionModel()->currentIndex();
            if (idx.isValid()) {
                if (!this->model()->data(idx, ComputerModel::DataRoles::kItemIsEditing).toBool()) {
                    Q_EMIT enterPressed(idx);
                    return true;
                }
            }
        }
    }
    return DListView::eventFilter(watched, event);
}

void ComputerView::showEvent(QShowEvent *event)
{
    auto model = qobject_cast<ComputerModel *>(this->model());
    if (model)
        model->startConnect();
    DListView::showEvent(event);
}

void ComputerView::hideEvent(QHideEvent *event)
{
    auto model = qobject_cast<ComputerModel *>(this->model());
    if (model)
        model->stopConnect();
    DListView::hideEvent(event);
}

void ComputerView::initView()
{
    dp->model = new ComputerModel(this);
    this->setModel(dp->model);
    this->setItemDelegate(new ComputerItemDelegate(this));
    qobject_cast<QListView *>(this)->setWrapping(true);
    qobject_cast<QListView *>(this)->setFlow(QListView::Flow::LeftToRight);
    this->setSpacing(10);
    this->setResizeMode(QListView::ResizeMode::Adjust);
    this->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    this->setEditTriggers(QListView::EditKeyPressed | QListView::SelectedClicked);
    //    this->setIconSize(QSize(iconsizes[m_statusbar->scalingSlider()->value()], iconsizes[m_statusbar->scalingSlider()->value()]));
    this->setIconSize(QSize(48, 48));   // TODO(xust)
    this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    this->setFrameShape(QFrame::Shape::NoFrame);
    this->viewport()->setAutoFillBackground(false);

    this->installEventFilter(this);
    this->viewport()->installEventFilter(this);
}

void ComputerView::initConnect()
{
    const int kEnterBySingleClick = 0;
    const int kEnterByDoubleClick = 1;
    const int kEnterByEnter = 2;
    auto enter = [this](const QModelIndex &idx, int type) {
        int mode = dfmbase::Application::appAttribute(dfmbase::Application::ApplicationAttribute::kOpenFileMode).toInt();
        if (type == mode || type == kEnterByEnter)
            this->cdTo(idx);
    };
    connect(this, &QAbstractItemView::clicked, this, std::bind(enter, std::placeholders::_1, kEnterBySingleClick));
    connect(this, &QAbstractItemView::doubleClicked, this, std::bind(enter, std::placeholders::_1, kEnterByDoubleClick));
    connect(this, &ComputerView::enterPressed, this, &ComputerView::cdTo);

    connect(this, &ComputerView::customContextMenuRequested, this, &ComputerView::onMenuRequest);
}

void ComputerView::onMenuRequest(const QPoint &pos)
{
    // TODO(xust) tem code
    QModelIndex index = indexAt(pos);
    if (!index.isValid())
        return;

    if (index.data(ComputerModel::DataRoles::kItemShapeTypeRole).toInt() == ComputerItemData::kSplitterItem)
        return;

    qDebug() << "menu requested: " << index.data(Qt::DisplayRole).toString();
    int row = index.row();
    auto computerModel = qobject_cast<ComputerModel *>(this->model());
    auto menu = computerModel->items.at(row).info->createMenu();
    if (menu)
        menu->exec(QCursor::pos());
    delete menu;
}

void ComputerView::mountAndEnterBlockDevice(const QModelIndex &index)
{
    // if is encrypted, unlock first
    bool isEncrypted = index.data(ComputerModel::DataRoles::kDeviceIsEncrypted).toBool();
    bool isUnlocked = index.data(ComputerModel::DataRoles::kDeviceIsUnlocked).toBool();
    auto devUrl = index.data(ComputerModel::DataRoles::kDeviceUrlRole).toUrl();
    QString shellDevId = ComputerItemWatcher::getBlockDevIdByUrl(devUrl);

    if (isEncrypted) {
        if (!isUnlocked) {
            auto &ctx = dpfInstance.serviceContext();
            auto dialogServ = ctx.service<DSC_NAMESPACE::DialogService>(DSC_NAMESPACE::DialogService::name());
            QString passwd = dialogServ->askPasswordForLockedDevice();
            if (passwd.isEmpty())
                return;

            QPointer<ComputerView> guard(this);
            DeviceManagerInstance.unlockAndDo(shellDevId, passwd, [guard, dialogServ](const QString &newID) {
                if (newID.isEmpty())
                    dialogServ->showErrorDialog(tr("Unlock device failed"), tr("Wrong password is inputed"));
                else if (guard)
                    guard->mountAndEnterBlockDevice(newID);
                else
                    qCritical() << "ComputerView is released somewhere.";
            });
        } else {
            auto realDevId = index.data(ComputerModel::DataRoles::kDeviceClearDevId).toString();
            mountAndEnterBlockDevice(realDevId);
        }
    } else {
        mountAndEnterBlockDevice(shellDevId);
    }
}

void ComputerView::mountAndEnterBlockDevice(const QString &blkId)
{
    QFuture<QString> fu = QtConcurrent::run(&DeviceManagerInstance, &DeviceManager::invokeMountBlockDevice, blkId);
    QFutureWatcher<QString> *watcher = new QFutureWatcher<QString>();
    connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher] {
        QString mpt = watcher->result();
        qDebug() << "mount and cd to: " << mpt;   // TODO(xust)
        watcher->deleteLater();
    });
    watcher->setFuture(fu);
}

void ComputerView::cdTo(const QModelIndex &index)
{
    // TODO(xust)
    int r = index.row();
    if (r < 0 || r >= model()->rowCount()) {
        return;
    }

    typedef ComputerItemData::ShapeType ItemType;
    ItemType type = ItemType(index.data(ComputerModel::DataRoles::kItemShapeTypeRole).toInt());
    if (type == ItemType::kSplitterItem)
        return;
    auto url = index.data(ComputerModel::DataRoles::kRealUrlRole).toUrl();

    if (url.isValid()) {
        qDebug() << "cd to: " + index.data(Qt::DisplayRole).toString() << url;   // TODO(xust)
    } else {
        QString suffix = index.data(ComputerModel::DataRoles::kSuffixRole).toString();
        if (suffix == dfmbase::SuffixInfo::kBlock) {
            mountAndEnterBlockDevice(index);
        } else if (suffix == dfmbase::SuffixInfo::kProtocol) {

        } else if (suffix == dfmbase::SuffixInfo::kStashedRemote) {
        }
    }
}

ComputerViewPrivate::ComputerViewPrivate(ComputerView *qq)
    : q(qq)
{
}

DPCOMPUTER_END_NAMESPACE