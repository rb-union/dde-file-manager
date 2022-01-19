/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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
#include "devicepropertydialog.h"

#include <QFormLayout>

#include <DDrawer>
#include <denhancedwidget.h>
#include <DArrowLineDrawer>
#include <DApplicationHelper>

const static int kArrowExpandHight = 30;
const static int kArrowExpandSpacing = 10;

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE
DevicePropertyDialog::DevicePropertyDialog(QWidget *parent)
    : DDialog(parent)
{
    iniUI();
    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

DevicePropertyDialog::~DevicePropertyDialog()
{
}

void DevicePropertyDialog::iniUI()
{
    deviceIcon = new DLabel(this);
    deviceIcon->setFixedHeight(128);

    deviceName = new DLabel(this);
    deviceName->setAlignment(Qt::AlignCenter);

    QFrame *basicInfoFrame = new QFrame(this);

    basicInfo = new KeyValueLabel(this);
    devicesProgressBar = new DColoredProgressBar(this);
    devicesProgressBar->addThreshold(0, QColor(0xFF0080FF));
    devicesProgressBar->addThreshold(7000, QColor(0xFFFFAE00));
    devicesProgressBar->addThreshold(9000, QColor(0xFFFF0000));
    devicesProgressBar->setMaximum(10000);
    devicesProgressBar->setMaximumHeight(8);
    devicesProgressBar->setTextVisible(false);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(basicInfo);
    vlayout->addWidget(devicesProgressBar);
    basicInfoFrame->setLayout(vlayout);

    QVBoxLayout *vlayout1 = new QVBoxLayout();
    vlayout1->setMargin(0);
    vlayout1->setSpacing(0);
    vlayout1->addWidget(deviceIcon, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayout1->addWidget(deviceName, 0, Qt::AlignHCenter | Qt::AlignTop);

    QFrame *frame = new QFrame(this);
    frame->setLayout(vlayout1);
    addContent(frame);

    scrollArea = new QScrollArea();
    scrollArea->setObjectName("PropertyDialog-QScrollArea");
    QPalette palette = scrollArea->viewport()->palette();
    palette.setBrush(QPalette::Background, Qt::NoBrush);
    scrollArea->viewport()->setPalette(palette);
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);

    QFrame *infoframe = new QFrame;
    QVBoxLayout *scrollWidgetLayout = new QVBoxLayout;
    // 修复BUG-47113 UI显示不对
    scrollWidgetLayout->setContentsMargins(10, 0, 10, 20);
    scrollWidgetLayout->setSpacing(kArrowExpandSpacing);
    infoframe->setLayout(scrollWidgetLayout);

    scrollArea->setWidget(infoframe);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    QVBoxLayout *scrolllayout = new QVBoxLayout;
    scrolllayout->addWidget(scrollArea);
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());
    layout->addLayout(scrolllayout, 1);

    deviceBasicWidget = new DeviceBasicWidget(this);

    setFixedWidth(350);
}

int DevicePropertyDialog::contentHeight() const
{
    int expandsHeight = kArrowExpandSpacing;
    for (const QWidget *expand : extendedControl) {
        expandsHeight += expand->height();
    }

#define DIALOG_TITLEBAR_HEIGHT 50
    return (DIALOG_TITLEBAR_HEIGHT
            + deviceIcon->height()
            + deviceName->height()
            + expandsHeight
            + 40);
}

void DevicePropertyDialog::setSelectDeviceInfo(const DSC_NAMESPACE::DeviceInfo &info)
{
    currentFileUrl = info.deviceUrl;
    deviceIcon->setPixmap(info.icon.pixmap(128, 128));
    deviceName->setText(info.deviceName);
    deviceBasicWidget->setSelectFileInfo(info);
    setProgressBar(info.totalCapacity, info.availableSpace);
    addExtendedControl(deviceBasicWidget, true);
}

void DevicePropertyDialog::handleHeight(int height)
{
    QRect rect = geometry();
    rect.setHeight(contentHeight() + kArrowExpandSpacing * 2);
    setGeometry(rect);
}

void DevicePropertyDialog::setProgressBar(QString totalSize, QString freeSize)
{
    devicesProgressBar->setMaximum(totalSize.toInt());
    devicesProgressBar->setValue(freeSize.toInt() / totalSize.toInt());
    devicesProgressBar->setMaximumHeight(8);
    devicesProgressBar->setTextVisible(false);

    // fix bug#47111 在浅色模式下，手动设置进度条背景色
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        DPalette palette = devicesProgressBar->palette();
        palette.setBrush(DPalette::ObviousBackground, QColor("#ededed"));
        DApplicationHelper::instance()->setPalette(devicesProgressBar, palette);
    }

    // 进度条背景色跟随主题变化而变化
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=](DGuiApplicationHelper::ColorType type) {
        DPalette palette = devicesProgressBar->palette();
        if (type == DGuiApplicationHelper::LightType) {
            palette.setBrush(DPalette::ObviousBackground, QColor("#ededed"));
            DApplicationHelper::instance()->setPalette(devicesProgressBar, palette);
        } else {
            palette.setBrush(DPalette::ObviousBackground, QColor("#4e4e4e"));
            DApplicationHelper::instance()->setPalette(devicesProgressBar, palette);
        }
    });
}

void DevicePropertyDialog::insertExtendedControl(int index, ExtendedControlView *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    vlayout->insertWidget(index, widget, 0, Qt::AlignTop);
    widget->setSelectFileUrl(currentFileUrl);
    QMargins cm = vlayout->contentsMargins();
    QRect rc = contentsRect();
    widget->setFixedWidth(rc.width() - cm.left() - cm.right());
    extendedControl.append(widget);
}

void DevicePropertyDialog::addExtendedControl(ExtendedControlView *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    insertExtendedControl(vlayout->count() - 1, widget);
}

void DevicePropertyDialog::insertExtendedControl(int index, ExtendedControlDrawerView *widget, bool expansion)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    widget->setSelectFileUrl(currentFileUrl);
    widget->setFixedHeight(kArrowExpandHight);
    widget->setExpand(expansion);
    QMargins cm = vlayout->contentsMargins();
    QRect rc = contentsRect();
    widget->setFixedWidth(rc.width() - cm.left() - cm.right());
    vlayout->insertWidget(index, widget, 0, Qt::AlignTop);
    extendedControl.append(widget);
    connect(widget, &ExtendedControlDrawerView::heightChanged, this, &DevicePropertyDialog::handleHeight);
}

void DevicePropertyDialog::addExtendedControl(ExtendedControlDrawerView *widget, bool expansion)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    insertExtendedControl(vlayout->count() - 1, widget, expansion);
}

void DevicePropertyDialog::showEvent(QShowEvent *event)
{
    DDialog::showEvent(event);

    //展示时须设置弹窗尺寸，防止最小化后再次展示时窗口大小异常
    QRect rc = geometry();
    rc.setHeight(contentHeight() + kArrowExpandSpacing * 2);
    setGeometry(rc);
}

void DevicePropertyDialog::closeEvent(QCloseEvent *event)
{
    DDialog::closeEvent(event);
    emit closed(currentFileUrl);
}