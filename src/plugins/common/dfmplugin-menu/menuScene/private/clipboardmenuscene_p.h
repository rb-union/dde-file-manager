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
#ifndef CLIPBOARDMENUSCENE_P_H
#define CLIPBOARDMENUSCENE_P_H
#include "menuScene/clipboardmenuscene.h"

#include "interfaces/private/abstractmenuscene_p.h"

DPMENU_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

class ClipBoardMenuScenePrivate : public AbstractMenuScenePrivate
{

public:
    friend class ClipBoardMenuScene;
    explicit ClipBoardMenuScenePrivate(AbstractMenuScene *qq);

private:
    const QString paste { "Paste" };
    const QString cut { "Cut" };
    const QString copy { "Copy" };
};

DPMENU_END_NAMESPACE

#endif   // CLIPBOARDMENUSCENE_P_H
