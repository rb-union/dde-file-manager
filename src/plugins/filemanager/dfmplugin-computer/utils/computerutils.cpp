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
#include "computerutils.h"
#include "fileentity/appentryfileentity.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/standardpaths.h"

#include <dfm-framework/framework.h>
#include <services/filemanager/windows/windowsservice.h>

DPCOMPUTER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QUrl ComputerUtils::makeBlockDevUrl(const QString &id)
{
    QUrl devUrl;
    devUrl.setScheme(SchemeTypes::kEntry);
    auto shortenBlk = id;
    shortenBlk.remove(QString(DeviceId::kBlockDeviceIdPrefix));   // /org/freedesktop/UDisks2/block_devices/sda1 -> sda1
    auto path = QString("%1.%2").arg(shortenBlk).arg(SuffixInfo::kBlock);   // sda1.blockdev
    devUrl.setPath(path);   // entry:sda1.blockdev
    return devUrl;
}

QString ComputerUtils::getBlockDevIdByUrl(const QUrl &url)
{
    if (url.scheme() != SchemeTypes::kEntry)
        return "";
    if (!url.path().endsWith(SuffixInfo::kBlock))
        return "";

    QString suffix = QString(".%1").arg(SuffixInfo::kBlock);
    QString id = QString("%1%2").arg(DeviceId::kBlockDeviceIdPrefix).arg(url.path().remove(suffix));
    return id;
}

QUrl ComputerUtils::makeProtocolDevUrl(const QString &id)
{
    QUrl devUrl;
    devUrl.setScheme(SchemeTypes::kEntry);
    auto path = id.toUtf8().toBase64();
    QString encodecPath = QString("%1.%2").arg(QString(path)).arg(SuffixInfo::kProtocol);
    devUrl.setPath(encodecPath);
    return devUrl;
}

QString ComputerUtils::getProtocolDevIdByUrl(const QUrl &url)
{
    if (url.scheme() != SchemeTypes::kEntry)
        return "";
    if (!url.path().endsWith(SuffixInfo::kProtocol))
        return "";

    QString suffix = QString(".%1").arg(SuffixInfo::kProtocol);
    QString encodecId = url.path().remove(suffix);
    QString id = QByteArray::fromBase64(encodecId.toUtf8());
    return id;
}

QUrl ComputerUtils::makeAppEntryUrl(const QString &filePath)
{
    if (!filePath.startsWith(StandardPaths::location(StandardPaths::kExtensionsAppEntryPath)))
        return {};
    if (!filePath.endsWith(".desktop"))
        return {};

    QString fileName = filePath.mid(filePath.lastIndexOf("/") + 1);
    fileName.remove(".desktop");
    QString newPath = QString("%1.%2").arg(fileName).arg(SuffixInfo::kAppEntry);

    QUrl url;
    url.setScheme(SchemeTypes::kEntry);
    url.setPath(newPath);
    return url;
}

QUrl ComputerUtils::getAppEntryFileUrl(const QUrl &entryUrl)
{
    if (!entryUrl.isValid())
        return {};
    if (!entryUrl.path().endsWith(SuffixInfo::kAppEntry))
        return {};

    QString fileName = entryUrl.path().remove("." + QString(SuffixInfo::kAppEntry));
    QUrl origUrl;
    origUrl.setScheme(SchemeTypes::kFile);
    origUrl.setPath(QString("%1/%2.%3").arg(StandardPaths::location(StandardPaths::kExtensionsAppEntryPath)).arg(fileName).arg("desktop"));
    return origUrl;
}

quint64 ComputerUtils::getWinId(QWidget *widget)
{
    auto &ctx = dpfInstance.serviceContext();
    auto winServ = ctx.service<DSB_FM_NAMESPACE::WindowsService>(DSB_FM_NAMESPACE::WindowsService::name());
    return winServ->findWindowId(widget);
}

dfm_service_filemanager::SideBarService *ComputerUtils::sbIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::SideBarService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::SideBarService>(DSB_FM_NAMESPACE::SideBarService::name());
}

bool ComputerUtils::shouldSystemPartitionHide()
{
    return Application::instance()->genericAttribute(Application::kHiddenSystemPartition).toBool();
}