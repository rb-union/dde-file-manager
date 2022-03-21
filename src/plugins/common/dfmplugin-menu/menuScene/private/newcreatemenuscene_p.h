/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef NEWCREATEMENUSCENE_P_H
#define NEWCREATEMENUSCENE_P_H
#include "menuScene/newcreatemenuscene.h"

#include "interfaces/private/abstractmenuscene_p.h"

DPMENU_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

class NewCreateMenuScenePrivate : public AbstractMenuScenePrivate
{
public:
    friend class NewCreateMenuScene;
    explicit NewCreateMenuScenePrivate(NewCreateMenuScene *qq);

private:
    const QString newFolder { "New folder" };
    const QString newDoc { "New document" };
    const QString officeText { "Office Text" };
    const QString spreadsheets { "Spreadsheets" };
    const QString presentation { "Presentation" };
    const QString plainText { "Plain Text" };
};

DPMENU_END_NAMESPACE

#endif   // NEWCREATEMENUSCENE_P_H
