// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfileinfo.h"
#include "utils/vaultdefine.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"
#include "private/vaultfileinfo_p.h"

#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-io/dfmio_utils.h>

#include <DFileIconProvider>

#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <qplatformdefs.h>

#include <sys/stat.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
namespace dfmplugin_vault {

VaultFileInfoPrivate::VaultFileInfoPrivate(VaultFileInfo *qq)
    : q(qq)
{
}

VaultFileInfoPrivate::~VaultFileInfoPrivate()
{
}

QString VaultFileInfoPrivate::fileDisplayPath() const
{
    QUrl currentUrl = q->fileUrl();
    currentUrl.setHost("");
    QString urlStr = currentUrl.toString();
    QByteArray array = urlStr.toLocal8Bit();
    QString filePath = QUrl::fromPercentEncoding(array);
    return filePath;
}

QString VaultFileInfoPrivate::absolutePath(const QString &path) const
{
    QUrl virtualUrl = VaultHelper::instance()->pathToVaultVirtualUrl(path);
    return virtualUrl.path();
}

QUrl VaultFileInfoPrivate::vaultUrl(const QUrl &url) const
{
    auto tmpurl = VaultHelper::instance()->pathToVaultVirtualUrl(url.path());
    return tmpurl;
}

QUrl VaultFileInfoPrivate::getUrlByNewFileName(const QString &fileName) const
{
    QUrl theUrl = q->urlOf(FileInfo::FileUrlInfoType::kUrl);

    theUrl.setPath(DFMIO::DFMUtils::buildFilePath(q->pathOf(PathInfoType::kAbsolutePath).toStdString().c_str(),
                                                  fileName.toStdString().c_str(), nullptr));

    theUrl.setHost("");

    return theUrl;
}

bool VaultFileInfoPrivate::isRoot() const
{
    bool bRootDir = false;
    const QString &localFilePath = DFMIO::DFMUtils::buildFilePath(kVaultBasePath.toStdString().c_str(), kVaultDecryptDirName, nullptr);
    QString path = q->pathOf(PathInfoType::kFilePath);
    if (localFilePath == path || localFilePath + "/" == path || localFilePath == path + "/") {
        bRootDir = true;
    }
    return bRootDir;
}

VaultFileInfo::VaultFileInfo(const QUrl &url)
    : ProxyFileInfo(url), d(new VaultFileInfoPrivate(this))
{
    QUrl tempUrl = VaultHelper::vaultToLocalUrl(url);
    setProxy(InfoFactory::create<FileInfo>(tempUrl,  Global::CreateFileInfoType::kCreateFileInfoSyncAndCache));
}

VaultFileInfo::~VaultFileInfo()
{
}

VaultFileInfo &VaultFileInfo::operator=(const VaultFileInfo &fileinfo)
{
    ProxyFileInfo::operator=(fileinfo);
    if (!proxy)
        setProxy(fileinfo.proxy);
    else {
        url = fileinfo.url;
        proxy = fileinfo.proxy;
    }
    return *this;
}

bool VaultFileInfo::operator==(const VaultFileInfo &fileinfo) const
{
    return proxy == fileinfo.proxy && url == fileinfo.url;
}

bool VaultFileInfo::operator!=(const VaultFileInfo &fileinfo) const
{
    return !(operator==(fileinfo));
}

QString VaultFileInfo::pathOf(const PathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kAbsolutePath:
        if (!proxy)
            return "";
        return d->absolutePath(proxy->pathOf(type));
    default:
        return ProxyFileInfo::pathOf(type);
    }
}

QUrl VaultFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kUrl:
        if (!proxy)
            return QUrl();
        return d->vaultUrl(proxy->urlOf(type));
    case FileUrlInfoType::kRedirectedFileUrl:
        return VaultHelper::vaultToLocalUrl(url);
    default:
        return ProxyFileInfo::urlOf(type);
    }
}
bool VaultFileInfo::exists() const
{
    if (urlOf(UrlInfoType::kUrl).isEmpty())
        return false;

    return proxy && proxy->exists();
}

void VaultFileInfo::refresh()
{
    if (!proxy) {
        return;
    }

    proxy->refresh();
}

bool VaultFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsFile:
        [[fallthrough]];
    case FileIsType::kIsDir:
        [[fallthrough]];
    case FileIsType::kIsReadable:
        [[fallthrough]];
    case FileIsType::kIsWritable:
        [[fallthrough]];
    case FileIsType::kIsExecutable:
        [[fallthrough]];
    case FileIsType::kIsSymLink:
        [[fallthrough]];
    case FileIsType::kIsHidden:
        return !proxy || proxy->isAttributes(type);
    default:
        return ProxyFileInfo::isAttributes(type);
    }
}

bool VaultFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDrop:
        if (VaultHelper::instance()->state(PathManager::vaultLockPath()) != VaultState::kUnlocked) {
            return false;
        }

        return !proxy || proxy->canAttributes(type);
    case FileCanType::kCanRedirectionFileUrl:
        return proxy;
    default:
        return ProxyFileInfo::canAttributes(type);
    }
}

QVariantHash VaultFileInfo::extraProperties() const
{
    if (!proxy)
        ProxyFileInfo::extraProperties();
    return proxy->extraProperties();
}

QUrl VaultFileInfo::getUrlByType(const UrlInfoType type, const QString &fileName) const
{
    switch (type) {
    case FileUrlInfoType::kGetUrlByNewFileName:
        return d->getUrlByNewFileName(fileName);
    default:
        return ProxyFileInfo::getUrlByType(type, fileName);
    }
}

QIcon VaultFileInfo::fileIcon()
{
    if (d->isRoot()) {
        QString iconName = "dfm_safebox";   // 如果是根目录，用保险柜图标
        return QIcon::fromTheme(iconName);
    }

    if (!proxy)
        ProxyFileInfo::fileIcon();
    return proxy->fileIcon();
}

qint64 VaultFileInfo::size() const
{
    if (!proxy)
        ProxyFileInfo::size();
    return proxy->size();
}

int VaultFileInfo::countChildFile() const
{
    if (isAttributes(OptInfoType::kIsDir)) {
        QDir dir(pathOf(PathInfoType::kAbsoluteFilePath));
        QStringList entryList = dir.entryList(QDir::AllEntries | QDir::System
                                              | QDir::NoDotAndDotDot | QDir::Hidden);
        return entryList.size();
    }

    return -1;
}

QVariant VaultFileInfo::extendAttributes(const ExtInfoType type) const
{
    switch (type) {
    case FileExtendedInfoType::kSizeFormat:
        if (!proxy)
            return ProxyFileInfo::extendAttributes(type);
        return proxy->extendAttributes(type);
    default:
        return ProxyFileInfo::extendAttributes(type);
    }
}

QString VaultFileInfo::nameOf(const NameInfoType type) const
{

    switch (type) {
    case NameInfoType::kFileCopyName:
        return displayOf(DisPlayInfoType::kFileDisplayName);
    case NameInfoType::kIconName: {
        QString iconName = "dfm_safebox";   // 如果是根目录，用保险柜图标
        if (isRoot())
            return iconName;
        else {
            if (!proxy)
                return const_cast<VaultFileInfo *>(this)->fileMimeType(QMimeDatabase::MatchDefault).iconName();
            else
                proxy->nameOf(NameInfoType::kIconName);
        }
        return QString();
    }
    default:
        return ProxyFileInfo::nameOf(type);
    }
}

QString VaultFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (DisPlayInfoType::kFileDisplayName == type) {
        if (d->isRoot()) {
            return QObject::tr("My Vault");
        }
        if (proxy)
            return proxy->displayOf(DisPlayInfoType::kFileDisplayName);
    } else if (DisPlayInfoType::kFileDisplayPath == type) {
        return d->fileDisplayPath();
    }

    return ProxyFileInfo::displayOf(type);
}
}