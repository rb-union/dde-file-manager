/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "devicebasicwidget.h"
#include "dfm-base/base/schemefactory.h"

DWIDGET_USE_NAMESPACE
DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE
DeviceBasicWidget::DeviceBasicWidget(QWidget *parent)
    : ExtendedControlDrawerView(parent)
{
    initUI();
}

DeviceBasicWidget::~DeviceBasicWidget()
{
}

void DeviceBasicWidget::initUI()
{
    setExpandedSeparatorVisible(false);
    setSeparatorVisible(false);

    deviceInfoFrame = new QFrame(this);

    deviceType = new KeyValueLabel(this);
    deviceType->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    deviceType->setLeftValue(tr("Device type"));
    deviceTotalSize = new KeyValueLabel(this);
    deviceTotalSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    deviceTotalSize->setLeftValue(tr("Total space"));
    fileSystem = new KeyValueLabel(this);
    fileSystem->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileSystem->setLeftValue(tr("File system"));
    fileCount = new KeyValueLabel(this);
    fileCount->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileCount->setLeftValue(tr("Contains"));
    freeSize = new KeyValueLabel(this);
    freeSize->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    freeSize->setLeftValue(tr("Free space"));

    QGridLayout *glayout = new QGridLayout;
    glayout->addWidget(deviceType, 0, 0, 1, 6);
    glayout->addWidget(deviceTotalSize, 0, 0, 1, 6);
    glayout->addWidget(fileSystem, 0, 0, 1, 6);
    glayout->addWidget(fileCount, 0, 0, 1, 6);
    glayout->addWidget(freeSize, 0, 0, 1, 6);
    glayout->setColumnStretch(0, 1);
    deviceInfoFrame->setLayout(glayout);

    setContent(deviceInfoFrame);
}

void DeviceBasicWidget::setSelectFileUrl(const QUrl &url)
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    fileCount->setRightValue(QString::number(info->countChildFile()));
    fileCount->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
}

void DeviceBasicWidget::setSelectFileInfo(const DeviceInfo &info)
{
    deviceType->setRightValue(info.deviceType);
    deviceType->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    deviceTotalSize->setRightValue(info.totalCapacity);
    deviceTotalSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    fileSystem->setRightValue(info.fileSystem);
    fileSystem->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
    freeSize->setRightValue(info.availableSpace);
    freeSize->setRightFontSizeWeight(DFontSizeManager::SizeType::T7);
}