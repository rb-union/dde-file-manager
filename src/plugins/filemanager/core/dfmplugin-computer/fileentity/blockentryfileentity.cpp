// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blockentryfileentity.h"
#include "utils/computerdatastruct.h"
#include "utils/computerutils.h"

#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>

#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QTimer>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

namespace IconName {
static constexpr char kRootBlock[] { "drive-harddisk-root" };
static constexpr char kInnerBlock[] { "drive-harddisk" };
static constexpr char kEncryptedInnerBlock[] { "drive-harddisk-encrypted" };
static constexpr char kRemovableBlock[] { "drive-removable-media-usb" };
static constexpr char kEncryptedRemovableBlock[] { "drive-removable-media-encrypted" };
static constexpr char kOpticalBlock[] { "media-optical" };
static constexpr char kOpticalBlockExternal[] { "media-external" };
}   // namespace IconName

using namespace GlobalServerDefines;

/*!
 * \class BlockEntryFileEntity
 * \brief class that present block devices
 */
BlockEntryFileEntity::BlockEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    if (!url.path().endsWith(SuffixInfo::kBlock)) {
        qWarning() << "wrong suffix in" << __FUNCTION__ << "url";
        abort();
    }

    auto refreshInfo = [this](const QString &id) {
        if (id == ComputerUtils::getBlockDevIdByUrl(entryUrl))
            refresh();
    };

    connect(DevProxyMng, &DeviceProxyManager::blockDevMounted, this, refreshInfo);
    connect(DevProxyMng, &DeviceProxyManager::blockDevUnmounted, this, refreshInfo);

    loadDiskInfo();
}

QString BlockEntryFileEntity::displayName() const
{
    if (datas.contains(WinVolTagKeys::kWinLabel))
        return datas.value(WinVolTagKeys::kWinLabel).toString();
    return DeviceUtils::convertSuitableDisplayName(datas);
}

QIcon BlockEntryFileEntity::icon() const
{
    bool isRootDisk = datas.value(DeviceProperty::kMountPoint).toString() == "/";
    if (isRootDisk)
        return QIcon::fromTheme(IconName::kRootBlock);

    bool isOptical = datas.value(DeviceProperty::kOptical).toBool() || datas.value(DeviceProperty::kOpticalDrive).toBool();
    bool isRemovable = datas.value(DeviceProperty::kRemovable).toBool();
    if (isOptical)
        return isRemovable ? QIcon::fromTheme(IconName::kOpticalBlockExternal) : QIcon::fromTheme(IconName::kOpticalBlock);

    bool isSysDisk = datas.value(DeviceProperty::kHintSystem).toBool();
    bool isEncrypted = datas.value(DeviceProperty::kIsEncrypted).toBool();
    if (isSysDisk)
        return isEncrypted ? QIcon::fromTheme(IconName::kEncryptedInnerBlock) : QIcon::fromTheme(IconName::kInnerBlock);

    return isEncrypted ? QIcon::fromTheme(IconName::kEncryptedRemovableBlock) : QIcon::fromTheme(IconName::kRemovableBlock);
}

bool BlockEntryFileEntity::exists() const
{
    const QString &&id = datas.value(DeviceProperty::kId).toString();
    bool hintIgnore { qvariant_cast<bool>(datas.value(DeviceProperty::kHintIgnore)) };
    bool hasFileSystem { qvariant_cast<bool>(datas.value(DeviceProperty::kHasFileSystem)) };
    bool opticalDrive { qvariant_cast<bool>(datas.value(DeviceProperty::kOpticalDrive)) };
    bool isEncrypted { qvariant_cast<bool>(datas.value(DeviceProperty::kIsEncrypted)) };
    QString cryptoBackingDevice { qvariant_cast<QString>(datas.value(DeviceProperty::kCryptoBackingDevice)) };
    bool hasPartitionTable { qvariant_cast<bool>(datas.value(DeviceProperty::kHasPartitionTable)) };
    bool hasPartition { qvariant_cast<bool>(datas.value(DeviceProperty::kHasPartition)) };
    bool hasExtendedPartition { qvariant_cast<bool>(datas.value(DeviceProperty::kHasExtendedPatition)) };
    bool isLoopDevice { qvariant_cast<bool>(datas.value(DeviceProperty::kIsLoopDevice)) };

    if (isLoopDevice && !hasFileSystem) {
        qInfo() << "loop device has no filesystem so hide it: " << id;
        return false;
    }

    if (hintIgnore && !isLoopDevice) {
        qInfo() << "block device is ignored by hintIgnore:" << id;
        return false;
    }

    if (!hasFileSystem && !opticalDrive && !isEncrypted) {
        bool removable { qvariant_cast<bool>(datas.value(DeviceProperty::kRemovable)) };
        if (!removable) {   // 满足外围条件的本地磁盘，直接遵循以前的处理直接 continue
            qInfo() << "block device is ignored by wrong removeable set for system disk:" << id;
            return false;
        }
    }

    if (cryptoBackingDevice.length() > 1) {
        qInfo() << "block device is ignored by crypted back device:" << id;
        return false;
    }

    // 是否是设备根节点，设备根节点无须记录
    if (hasPartitionTable) {   // 替换 FileUtils::deviceShouldBeIgnore
        qInfo() << "block device is ignored by parent node:" << id;
        return false;
    }

    if (hasPartition && hasExtendedPartition) {
        qInfo() << "block device is ignored by extended partion type" << id;
        return false;
    }

    quint64 blkSize = { qvariant_cast<quint64>(datas.value(DeviceProperty::kSizeTotal)) };
    if (blkSize < 1024 && !opticalDrive && !hasFileSystem) {
        qInfo() << "block device is ignored cause tiny size: " << blkSize << id;
        return false;
    }

    return true;
}

bool BlockEntryFileEntity::showProgress() const
{
    return showSizeAndProgress();
}

bool BlockEntryFileEntity::showTotalSize() const
{
    return showSizeAndProgress();
}

bool BlockEntryFileEntity::showUsageSize() const
{
    return showSizeAndProgress();
}

EntryFileInfo::EntryOrder BlockEntryFileEntity::order() const
{
    if (datas.value(DeviceProperty::kHintSystem).toBool()) {
        if (datas.value(DeviceProperty::kMountPoint).toString() == "/")
            return EntryFileInfo::EntryOrder::kOrderSysDiskRoot;
        if (datas.value(DeviceProperty::kIdLabel).toString().startsWith("_dde_"))
            return EntryFileInfo::EntryOrder::kOrderSysDiskData;
        return EntryFileInfo::EntryOrder::kOrderSysDisks;
    }

    if (datas.value(DeviceProperty::kOptical).toBool() || datas.value(DeviceProperty::kOpticalDrive).toBool()) {
        return EntryFileInfo::EntryOrder::kOrderOptical;
    }

    return EntryFileInfo::EntryOrder::kOrderRemovableDisks;
}

quint64 BlockEntryFileEntity::sizeTotal() const
{
    return getProperty(DeviceProperty::kSizeTotal).toULongLong();
}

quint64 BlockEntryFileEntity::sizeUsage() const
{
    return getProperty(DeviceProperty::kSizeUsed).toULongLong();
}

void BlockEntryFileEntity::refresh()
{
    loadDiskInfo();
}

QUrl BlockEntryFileEntity::targetUrl() const
{
    return mountPoint();
}

bool BlockEntryFileEntity::isAccessable() const
{
    if (datas.value(DeviceProperty::kIsEncrypted).toBool())
        return true;
    return datas.value(DeviceProperty::kHasFileSystem).toBool();
}

bool BlockEntryFileEntity::renamable() const
{
    if (datas.value(DeviceProperty::kOpticalDrive).toBool())
        return false;

    if (datas.value(DeviceProperty::kIsEncrypted).toBool()
        && datas.value(DeviceProperty::kCleartextDevice).toString() == "/")
        return false;

    return isAccessable();
}

QVariant BlockEntryFileEntity::getProperty(const char *const key) const
{
    if (datas.value(DeviceProperty::kIsEncrypted).toBool()) {
        if (datas.contains(BlockAdditionalProperty::kClearBlockProperty))
            return datas.value(BlockAdditionalProperty::kClearBlockProperty).toMap().value(key);
        return {};
    }
    return datas.value(key);
}

bool BlockEntryFileEntity::showSizeAndProgress() const
{
    if (getProperty(DeviceProperty::kMountPoint).toString().isEmpty())
        return false;

    if (datas.value(DeviceProperty::kIsEncrypted).toBool()) {
        if (datas.contains(BlockAdditionalProperty::kClearBlockProperty))
            return true;
        return false;
    }

    return true;
}

QUrl BlockEntryFileEntity::mountPoint() const
{
    auto mptList = getProperty(DeviceProperty::kMountPoints).toStringList();
    QUrl target;
    if (mptList.isEmpty())
        return target;

    // when enter DataDisk, enter Home directory.
    for (const auto &mpt : mptList) {
        if (mpt != QDir::rootPath()) {
            const QString &userHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            const QString &homeBindPath = FileUtils::bindPathTransform(userHome, true);
            if (userHome != homeBindPath && homeBindPath.startsWith(mpt))
                return QUrl::fromLocalFile(homeBindPath);
        }
    }

    target.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kFile);
    target.setPath(mptList.first());
    return target;
}

void BlockEntryFileEntity::loadDiskInfo()
{
    auto id = QString(DeviceId::kBlockDeviceIdPrefix)
            + entryUrl.path().remove("." + QString(SuffixInfo::kBlock));

    datas = UniversalUtils::convertFromQMap(DevProxyMng->queryBlockInfo(id));
    auto clearBlkId = datas.value(DeviceProperty::kCleartextDevice).toString();
    if (datas.value(DeviceProperty::kIsEncrypted).toBool() && clearBlkId.length() > 1) {
        auto clearBlkData = DevProxyMng->queryBlockInfo(clearBlkId);
        datas.insert(BlockAdditionalProperty::kClearBlockProperty, clearBlkData);
    }

    if (mountPoint().isValid())
        loadWindowsVoltag();
    else
        resetWindowsVolTag();
}

void BlockEntryFileEntity::loadWindowsVoltag()
{
    static const QString &kWinVolInfoConfig = "/UOSICON/diskinfo.json";
    static const QString &kDiskInfoKey = "DISKINFO";
    static const QString &kUUIDKey = "uuid";
    static const QString &kDriveKey = "drive";
    static const QString &kLabelKey = "label";

    QUrl cfgUrl = QUrl::fromLocalFile(mountPoint().path() + kWinVolInfoConfig);

    DFMIO::DFile f(cfgUrl);
    if (!f.exists())
        return;

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (doc.isNull() || err.error != QJsonParseError::NoError) {
        qDebug() << "Cannot parse file: " << cfgUrl << err.errorString() << err.error;
        return;
    }

    if (doc.isObject()) {
        auto obj = doc.object();
        if (obj.contains(kDiskInfoKey) && obj.value(kDiskInfoKey).isArray()) {
            auto arr = obj.value(kDiskInfoKey).toArray();
            for (int i = 0; i < arr.size(); ++i) {
                auto itemObj = arr[i].toObject();

                if (itemObj.contains(kUUIDKey))
                    datas.insert(WinVolTagKeys::kWinUUID, itemObj.value(kUUIDKey).toString());
                if (itemObj.contains(kDriveKey))
                    datas.insert(WinVolTagKeys::kWinDrive, itemObj.value(kDriveKey).toString());
                if (itemObj.contains(kLabelKey))
                    datas.insert(WinVolTagKeys::kWinLabel, itemObj.value(kLabelKey).toString());
            }
        }
    }
}

void BlockEntryFileEntity::resetWindowsVolTag()
{
    datas.remove(WinVolTagKeys::kWinUUID);
    datas.remove(WinVolTagKeys::kWinDrive);
    datas.remove(WinVolTagKeys::kWinLabel);
}