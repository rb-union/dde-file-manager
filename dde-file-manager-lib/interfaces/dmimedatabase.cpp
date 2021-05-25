/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "dmimedatabase.h"
#include "shutil/fileutils.h"
#include "dstorageinfo.h"

#include <QFileInfo>

DFM_BEGIN_NAMESPACE

DMimeDatabase::DMimeDatabase()
{

}

QMimeType DMimeDatabase::mimeTypeForFile(const QString &fileName, QMimeDatabase::MatchMode mode) const
{
    return mimeTypeForFile(QFileInfo(fileName), mode);
}
//判读ios手机，传输慢，需要特殊处理优化
QMimeType DMimeDatabase::mimeTypeForFileOptimise(const QString &fileName, QMimeDatabase::MatchMode mode) const
{
    QList<QMimeType> results = QMimeDatabase::mimeTypesForFileName(fileName);
    if (!results.isEmpty()) {
        return results.first();
    }
    return mimeTypeForFile(fileName,mode);
}

QMimeType DMimeDatabase::mimeTypeForFile(const QFileInfo &fileInfo, QMimeDatabase::MatchMode mode) const
{
    // 如果是低速设备，则先从扩展名去获取mime信息；对于本地文件，保持默认的获取策略
    QMimeType result;
    QString path = fileInfo.path();
    //fix bug 29124 #CPM2020070800063# pg
    // 系统盘右键打开属性窗口，关闭or不关闭，之后在文件管理器随意点击文件，文件管理器卡死。(一般+必现+不常用功能)（使用MatchExtension去获取mimetype）
    bool bMatchExtension = (fileInfo.absoluteFilePath() == QString("/sys/kernel/security/apparmor/revision") ||
                            fileInfo.absoluteFilePath() == QString("/sys/power/wakeup_count"))?
                true : false;
    //fix bug 35448 【文件管理器】【5.1.2.2-1】【sp2】预览ftp路径下某个文件夹后，文管卡死,访问特殊系统文件卡死
    if (!bMatchExtension && (fileInfo.fileName().endsWith(".pid") || path.endsWith("msg.lock")
            || fileInfo.fileName().endsWith(".lock") || fileInfo.fileName().endsWith("lockfile"))) {
        QRegularExpression regExp("^/run/user/\\d+/gvfs/(?<scheme>\\w+(-?)\\w+):\\S*",
                                             QRegularExpression::DotMatchesEverythingOption
                                             | QRegularExpression::DontCaptureOption
                                             | QRegularExpression::OptimizeOnFirstUsageOption);

        const QRegularExpressionMatch &match = regExp.match(path, 0, QRegularExpression::NormalMatch,
                                                            QRegularExpression::DontCheckSubjectStringMatchOption);

        bMatchExtension = match.hasMatch();
    }
    if (DStorageInfo::isLowSpeedDevice(path) || bMatchExtension) {
        result = QMimeDatabase::mimeTypeForFile(fileInfo, QMimeDatabase::MatchExtension);
    } else {
        result = QMimeDatabase::mimeTypeForFile(fileInfo, mode);
    }

    // temporary dirty fix, once WPS get installed, the whole mimetype database thing get fscked up.
    // we used to patch our Qt to fix this issue but the patch no longer works, we don't have time to
    // look into this issue ATM.
    // https://bugreports.qt.io/browse/QTBUG-71640
    // https://codereview.qt-project.org/c/qt/qtbase/+/244887
    // `file` command works but libmagic didn't even comes with any pkg-config support..
    static QStringList officeSuffixList {
        "docx", "xlsx", "pptx", "doc", "ppt", "xls"
    };
    static QStringList wrongMimeTypeNames {
        "application/x-ole-storage", "application/zip"
    };
    if (officeSuffixList.contains(fileInfo.suffix()) && wrongMimeTypeNames.contains(result.name())) {
        QList<QMimeType> results = QMimeDatabase::mimeTypesForFileName(fileInfo.fileName());
        if (!results.isEmpty()) {
            return results.first();
        }
    }

    return result;
}

QMimeType DMimeDatabase::mimeTypeForUrl(const QUrl &url) const
{
    if (url.isLocalFile())
        return mimeTypeForFile(url.toLocalFile());

    return QMimeDatabase::mimeTypeForUrl(url);
}

DFM_END_NAMESPACE