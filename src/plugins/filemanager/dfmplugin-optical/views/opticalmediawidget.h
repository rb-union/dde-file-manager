/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef OPTICALMEDIAWIDGET_H
#define OPTICALMEDIAWIDGET_H

#include "dfmplugin_optical_global.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QSvgWidget>
#include <DPushButton>

DPOPTICAL_BEGIN_NAMESPACE

class OpticalMediaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OpticalMediaWidget(QWidget *parent = nullptr);

private:
    void initializeUi();
    void initConnect();

private:
    QHBoxLayout *layout { nullptr };
    QLabel *lbMediatype { nullptr };
    QLabel *lbAvailable { nullptr };
    QLabel *lbUdsupport { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *pbBurn { nullptr };
    QSvgWidget *iconCaution { nullptr };
};

DPOPTICAL_END_NAMESPACE

#endif   // OPTICALMEDIAWIDGET_H