// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/fileview.h"
#include "views/private/fileview_p.h"
#include "models/fileviewmodel.h"
#include "utils/filesortworker.h"

#include <gtest/gtest.h>

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/fileutils.h>

DPWORKSPACE_USE_NAMESPACE
using namespace dfmbase;

TEST(UT_WorkspacePluginBugTest, bug_169379_NotInit)
{
    bool isInit { false };
    stub_ext::StubExt stub;
    stub.set_lamda(&FileView::itemDelegate, [] { return nullptr; });
    stub.set_lamda(&FileView::setItemDelegate, [] {});
    stub.set_lamda(&FileViewPrivate::initListModeView, [ &isInit ] { isInit = true; });
    typedef QVariant(*FuncType)(Application::ApplicationAttribute);
    stub.set_lamda(static_cast<FuncType>(&Application::appAttribute), [] {return QVariant(false); });
    stub.set_lamda(&FileView::initializeModel, [] {});
    stub.set_lamda(&FileView::initializeDelegate, [] {});
    stub.set_lamda(&FileView::initializeStatusBar, [] {});
    stub.set_lamda(&FileView::initializeConnect, [] {});


    FileView view(QUrl("/home"));
    view.setViewMode(Global::ViewMode::kListMode);
    EXPECT_TRUE(isInit);
}

TEST(UT_WorkspacePluginBugTest, bug_140799_desktopFilter)
{
    bool isExcuDesktopFileCheck { false };
    stub_ext::StubExt stub;
    typedef QVariant(*FuncType)(Application::ApplicationAttribute);
    stub.set_lamda(static_cast<FuncType>(&Application::appAttribute), [] {return QVariant(false); });
    stub.set_lamda(&FileUtils::isDesktopFile, [ &isExcuDesktopFileCheck ] { isExcuDesktopFileCheck = true; return ""; });

    FileViewModel model;
    FileInfoPointer info(new FileInfo(QUrl("file:///home")));
    FileSortWorker *work = new FileSortWorker(QUrl("file:///home"), "1234");
    work->nameFilters = QStringList { "*.desktop" };
    model.filterSortWorker.reset(work);
    model.passNameFilters(info);
    EXPECT_TRUE(isExcuDesktopFileCheck);
}