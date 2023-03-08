// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebaritemdelegate.h"

#include "sidebaritem.h"
#include "sidebarmodel.h"
#include "sidebarview.h"
#include "events/sidebareventcaller.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/base/device/deviceutils.h"

#include <DPaletteHelper>
#include <DGuiApplicationHelper>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <DPalette>
#include <qdrawutil.h>
#include <QLineEdit>
#include <QImage>
#include <QFontMetrics>
#include <QEvent>
#include <QMouseEvent>
#include <QSignalBlocker>

#include <linux/limits.h>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QT_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static constexpr int kRadius = 8;
static constexpr int kItemMargin = 10;
static constexpr int kItemIconSize = 19;
static constexpr int kEjectIconSize = 16;

namespace GlobalPrivate {
const static char *const kRegPattern { "^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*" };
void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option)
{
    painter->save();

    int yPoint = option.rect.top() + option.rect.height() / 2;
    qDrawShadeLine(painter, 0, yPoint, option.rect.width(), yPoint, option.palette);

    painter->restore();
}
}   // namespace GlobalPrivate

SideBarItemDelegate::SideBarItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

void SideBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return DStyledItemDelegate::paint(painter, option, index);
    painter->save();

    QStyleOptionViewItem opt = option;

    QStyledItemDelegate::initStyleOption(&opt, index);
    painter->setRenderHint(QPainter::Antialiasing);

    DPalette palette(DPaletteHelper::instance()->palette(option.widget));

    auto widgetColor = option.widget->palette().base().color();
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        widgetColor = DGuiApplicationHelper::adjustColor(widgetColor, 0, 0, 5, 0, 0, 0, 0);

    QStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);

    if (!item)
        return DStyledItemDelegate::paint(painter, option, index);
    SideBarItemSeparator *separatorItem = dynamic_cast<SideBarItemSeparator *>(item);
    QRect itemRect = opt.rect;
    QPoint dx = QPoint(kItemMargin, 0);
    QPoint dw = QPoint(-12, 0);
    bool selected = opt.state.testFlag(QStyle::State_Selected);
    QRect r(itemRect.topLeft() + dx, itemRect.bottomRight() + dw);

    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());

    SideBarItem *subItem = dynamic_cast<SideBarItem *>(item);
    bool keepDrawingHighlighted = false;
    bool isUrlEqual = UniversalUtils::urlEquals(index.data(SideBarItem::kItemUrlRole).toUrl(), sidebarView->currentUrl());
    if (!isUrlEqual && subItem) {
        bool foundByCb = subItem->itemInfo().findMeCb && subItem->itemInfo().findMeCb(subItem->url(), sidebarView->currentUrl());
        if (foundByCb || UniversalUtils::urlEquals(subItem->url(), sidebarView->currentUrl()))
            isUrlEqual = true;
    }
    bool isDraggingItemNotHighlighted = selected && !isUrlEqual;
    if (isUrlEqual) {
        // If the dragging and moving source item is not the current highlighted one,
        // the highlighted one must be keep its state.
        keepDrawingHighlighted = true;
    }

    if (selected || keepDrawingHighlighted) {   //Draw selected background
        QPalette::ColorGroup colorGroup = QPalette::Normal;
        QColor bgColor = option.palette.color(colorGroup, QPalette::Highlight);

        if (isDraggingItemNotHighlighted) {
            auto baseColor = palette.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
                bgColor = DGuiApplicationHelper::adjustColor(widgetColor, 0, 0, 5, 0, 0, 0, 0);
            else
                bgColor = baseColor.lighter();
        }

        painter->setBrush(bgColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(r, kRadius, kRadius);
    } else if (opt.state.testFlag(QStyle::State_MouseOver)) {   //Draw mouse over background
        drawMouseHoverBackground(painter, palette, r, widgetColor);
        if (separatorItem)
            drawMouseHoverExpandButton(painter, r, separatorItem->isExpanded());
    }

    //Draw item icon
    QSize iconSize(kItemIconSize, kItemIconSize);
    QSize ejectIconSize(kEjectIconSize, kEjectIconSize);
    QIcon::Mode iconMode = (!isDraggingItemNotHighlighted && (selected || keepDrawingHighlighted)) ? QIcon::Mode::Selected : QIcon::Mode::Normal;
    bool isEjectable = false;
    SideBarItem *sidebarItem = static_cast<SideBarItem *>(item);
    if (sidebarItem) {
        ItemInfo info = sidebarItem->itemInfo();
        isEjectable = info.isEjectable;
        QIcon icon = item->icon();
        drawIcon(painter, icon, itemRect, iconMode, isEjectable);
        //notes: if draw eject icon with `DStyledItemDelegate::paint(painter, option, index)`,
        //could not match the UI style of requirement, so here use `drawIcon()`, but this way would trigger the item-action-event
        //as clicking the eject icon.
        //need some research for this promblem.
        /*
        if (!isEjectable)
            drawIcon(painter, icon, itemRect, iconMode, isEjectable);
        else
            return DStyledItemDelegate::paint(painter, option, index);
        */
    }

    //Draw item text
    QFontMetrics metricsLabel(option.widget->font());
    painter->setPen(separatorItem ? Qt::gray : qApp->palette().color(QPalette::ColorRole::Text));
    if (!isDraggingItemNotHighlighted && (selected || keepDrawingHighlighted))
        painter->setPen("#ffffff");

    QString text = index.data().toString();
    qreal baseValue = itemRect.width() - iconSize.width() - 2 * kItemMargin;
    qreal min = baseValue - 2 * ejectIconSize.width();
    qreal max = baseValue - ejectIconSize.width();

    if (metricsLabel.horizontalAdvance(text) > (isEjectable ? min : max))
        text = QFontMetrics(option.widget->font()).elidedText(text, Qt::ElideRight, (isEjectable ? int(min) : int(max)));
    int rowHeight = itemRect.height();
    qreal txtDx = separatorItem ? 22 : 42;
    qreal txtDy = (itemRect.height() - metricsLabel.lineSpacing()) / 2;
    painter->drawText(QRectF(itemRect.x() + txtDx, itemRect.y() + txtDy, itemRect.width(), rowHeight), Qt::AlignLeft, text);
    painter->restore();
}

QSize SideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return DStyledItemDelegate::sizeHint(option, index);
}

void SideBarItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *edt = nullptr;
    if ((edt = dynamic_cast<QLineEdit *>(editor)) && edt->isModified()) {
        QByteArray n = editor->metaObject()->userProperty().name();
        if (!n.isEmpty())
            Q_EMIT rename(index, editor->property(n).toString());
    }

    return;
}

QWidget *SideBarItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!this->parent())
        return nullptr;
    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
    if (!sidebarView)
        return nullptr;
    SideBarModel *sidebarModel = dynamic_cast<SideBarModel *>(sidebarView->model());
    if (!sidebarModel)
        return nullptr;
    SideBarItem *tgItem = sidebarModel->itemFromIndex(index);
    if (!tgItem)
        return nullptr;
    auto sourceInfo = InfoFactory::create<AbstractFileInfo>(tgItem->url());
    if (!sourceInfo)
        return nullptr;
    if (!sourceInfo->exists())
        return nullptr;
    QWidget *editor = DStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit *qle = nullptr;
    if ((qle = dynamic_cast<QLineEdit *>(editor))) {
        QRegularExpression regx(GlobalPrivate::kRegPattern);
        QValidator *validator = new QRegularExpressionValidator(regx, qle);
        qle->setValidator(validator);

        connect(qle, &QLineEdit::textChanged, this, [this, sourceInfo](const QString &text) {
            onEditorTextChanged(text, sourceInfo);
        });
    }

    return editor;
}

void SideBarItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    DStyledItemDelegate::updateEditorGeometry(editor, option, index);
    //When DTK calculates the width of editor, it does not care about the icon width, so adjust the width of editor here.
    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
    editor->setFixedWidth(sidebarView->width() - 50);
    QRect rect = editor->geometry();
    rect.setHeight(rect.height() + 2);
    rect.moveTo(40, rect.top());
    editor->setGeometry(rect);
}

bool SideBarItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.isValid()) {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *e = static_cast<QMouseEvent *>(event);
            if (e->button() == Qt::LeftButton) {
                QStandardItem *item = qobject_cast<const SideBarModel *>(model)->itemFromIndex(index);
                SideBarItem *sidebarItem = static_cast<SideBarItem *>(item);
                SideBarItemSeparator *separatorItem = dynamic_cast<SideBarItemSeparator *>(item);
                bool ejectable = false;
                if (sidebarItem) {
                    ItemInfo info = sidebarItem->itemInfo();
                    ejectable = info.isEjectable;
                }
                QRect expandBtRect(option.rect.width() - 40, option.rect.topRight().y() + 4, 24, 24);
                QRect ejectBtRect(option.rect.bottomRight() + QPoint(-28, -26), option.rect.bottomRight() + QPoint(-kItemMargin, -kItemMargin));
                QPoint pos = e->pos();
                SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
                if (event->type() != QEvent::MouseButtonRelease && separatorItem && expandBtRect.contains(pos)) {   //The expand/unexpand icon is pressed.
                    SideBarView *sidebarView = dynamic_cast<SideBarView *>(this->parent());
                    if (sidebarView)
                        Q_EMIT changeExpandState(index, !sidebarView->isExpanded(index));

                    event->accept();
                    return true;
                } else if (event->type() == QEvent::MouseButtonRelease && ejectable && ejectBtRect.contains(pos)) {   //The eject icon is pressed.
                    if (sidebarItem) {
                        QUrl url = sidebarItem->itemInfo().url;
                        SideBarEventCaller::sendEject(url);
                        //onItemActived() slot function would be triggered with mouse clicking,
                        //in order to avoid mount device again, we set item action to disable state as a mark.
                        DViewItemActionList list = sidebarItem->actionList(Qt::RightEdge);
                        if (list.count() > 0 && sidebarView) {
                            list.first()->setDisabled(true);
                            //fix bug: #185137, save the current url and highlight it in `SideBarWidget::onItemActived`
                            //that is triggered by cliking eject icon.
                            //this is the temporary solution.
                            list.first()->setProperty("currentItem", sidebarView->currentUrl());
                        }
                    }
                    event->accept();
                    return true;
                }
            }
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void SideBarItemDelegate::onEditorTextChanged(const QString &text, const AbstractFileInfoPointer &info) const
{
    QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
    if (!editor)
        return;

    int maxLen = INT_MAX;
    int textLen = 0;
    bool useCharCount = false;
    const QString &fs = info->extraProperties()[GlobalServerDefines::DeviceProperty::kFileSystem].toString();
    if (fs.isEmpty()) {
        const auto &url = info->urlOf(AbstractFileInfo::FileUrlInfoType::kUrl);
        if (FileUtils::isLocalFile(url)) {
            maxLen = NAME_MAX;
            const auto &path = url.path();
            useCharCount = DeviceUtils::isSubpathOfDlnfs(path);
            textLen = textLength(text, path.isEmpty() ? false : useCharCount);
        }
    } else {
        maxLen = FileUtils::supportedMaxLength(fs);
        textLen = textLength(text, false);
    }

    QString dstText = text;
    int currPos = editor->cursorPosition();
    processLength(maxLen, textLen, useCharCount, dstText, currPos);

    if (text != dstText) {
        QSignalBlocker blocker(editor);
        editor->setText(dstText);
        editor->setCursorPosition(currPos);
        editor->setModified(true);
    }
}

void SideBarItemDelegate::drawIcon(QPainter *painter, const QIcon &icon, const QRect &itemRect, QIcon::Mode iconMode, bool isEjectable) const
{
    Q_UNUSED(iconMode);
    QSize iconSize(kItemIconSize, kItemIconSize);
    QSize ejectIconSize(kEjectIconSize, kEjectIconSize);
    qreal iconDx = 2 * kItemMargin;
    qreal iconDy = (itemRect.height() - iconSize.height()) / 2 + 1;
    QPointF iconTopLeft = itemRect.topLeft() + QPointF(iconDx, iconDy);
    QPointF iconBottomRight = iconTopLeft + QPointF(iconSize.width(), iconSize.height());
    auto px = icon.pixmap(iconSize);
    px.setDevicePixelRatio(qApp->devicePixelRatio());
    painter->drawPixmap(QRect(iconTopLeft.toPoint(), iconBottomRight.toPoint()), px);
    if (isEjectable) {
        QPoint ejectIconTopLeft = itemRect.bottomRight() + QPoint(0 - ejectIconSize.width() * 2, 0 - (itemRect.height() + ejectIconSize.height()) / 2);
        QPoint ejectIconBottomRight = ejectIconTopLeft + QPoint(ejectIconSize.width(), ejectIconSize.height());
        QIcon ejectIcon = QIcon::fromTheme("media-eject-symbolic");
        auto px = ejectIcon.pixmap(iconSize);
        px.setDevicePixelRatio(qApp->devicePixelRatio());
        painter->drawPixmap(QRect(ejectIconTopLeft, ejectIconBottomRight), px);
    }
}

void SideBarItemDelegate::drawMouseHoverBackground(QPainter *painter, const DPalette &palette, const QRect &r, const QColor &widgetColor) const
{
    auto mouseHoverColor = palette.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        mouseHoverColor = DGuiApplicationHelper::adjustColor(widgetColor, 0, 0, 5, 0, 0, 0, 0);
    else
        mouseHoverColor = mouseHoverColor.lighter();

    painter->setBrush(mouseHoverColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(r, kRadius, kRadius);
}

void SideBarItemDelegate::drawMouseHoverExpandButton(QPainter *painter, const QRect &r, bool isExpanded) const
{
    QSize expandIconSize(12, 12);
    QColor c(Qt::lightGray);
    painter->setBrush(c);
    painter->setPen(Qt::NoPen);
    QPoint tl = r.topRight() + QPoint(-26, kItemMargin);
    QPoint br = r.topRight() + QPoint(0 - kItemMargin, 27);
    QRect gRect(tl, br);
    painter->drawRoundedRect(gRect, kRadius, kRadius);
    QPixmap pixmap = QIcon::fromTheme(isExpanded ? "go-up" : "go-down").pixmap(expandIconSize);
    painter->drawPixmap(gRect.topRight() + QPointF(-14, 3), pixmap);
}

int SideBarItemDelegate::textLength(const QString &text, bool useCharCount) const
{
    return useCharCount ? text.size() : text.toLocal8Bit().size();
}

void SideBarItemDelegate::processLength(int maxLen, int textLen, bool useCharCount, QString &text, int &pos) const
{
    QString srcText = text;
    int srcPos = pos;
    int editTextRangeOutLen = textLen - maxLen;
    if (editTextRangeOutLen > 0 && maxLen != INT_MAX) {
        QVector<uint> list = srcText.toUcs4();
        QString tmp = srcText;
        while (textLength(tmp, useCharCount) > maxLen && srcPos > 0) {
            list.removeAt(--srcPos);
            tmp = QString::fromUcs4(list.data(), list.size());
        }

        text = tmp;
        pos = srcPos;
    }
}