/*
 * Copyright (C) 2016 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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
*/

#include <gtest/gtest.h>

#include "textpreviewplugin.h"

DFM_USE_NAMESPACE

class TestTextPreviewPlugin :public testing::Test
{
public:
     void SetUp() override
     {
        m_testPreviewPlugin = new TextPreviewPlugin();
     }

     void TearDown() override
     {
         delete m_testPreviewPlugin;
         m_testPreviewPlugin = nullptr;
     }

public:
     TextPreviewPlugin * m_testPreviewPlugin;
};

TEST_F(TestTextPreviewPlugin, use_create)
{
    EXPECT_TRUE(m_testPreviewPlugin->create("dde-file-manager") != nullptr);
}
