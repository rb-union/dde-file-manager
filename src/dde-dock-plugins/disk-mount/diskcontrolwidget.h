/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DISKCONTROLWIDGET_H
#define DISKCONTROLWIDGET_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QPointer>
#include <DDialog>

#include <mutex>

using namespace DTK_NAMESPACE::Widget;
class DiskControlItem;

class DiskControlWidget : public QScrollArea
{
    friend DiskControlItem;
    Q_OBJECT
public:
    explicit DiskControlWidget(QWidget *parent = nullptr);
    void initListByMonitorState();

signals:
    void diskCountChanged(const int count) const;

private slots:
    void onDiskListChanged();
    void onAskStopScanning(const QString &method, const QString &id);
    void onDeviceBusy(int action);

private:
    void initializeUi();
    void initConnection();
    void removeWidgets();
    void paintUi();
    void addSeparateLineUi(int width);
    int addBlockDevicesItems();
    int addProtocolDevicesItems();
    int addItems(const QStringList &list, bool isBlockDevice);
    DDialog *showQueryScanningDialog(const QString &title);
    void handleWhetherScanning(const QString &method, const QString &id);
    void notifyMessage(const QString &msg);
    void notifyMessage(const QString &title, const QString &msg);

    static std::once_flag &initOnceFlag();
    static std::once_flag &retryOnceFlag();

private:
    QVBoxLayout *centralLayout { nullptr };
    QWidget *centralWidget { nullptr };
};

#endif   // DISKCONTROLWIDGET_H
