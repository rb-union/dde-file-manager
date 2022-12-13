/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -c DBusDock -p dbusdock1 org.deepin.dde.daemon.Dock1.xml
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef DBUSDOCK1_H
#define DBUSDOCK1_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.deepin.dde.daemon.Dock1
 */

struct DockRect{
    qint32 x;
    qint32 y;
    qint32 width;
    qint32 height;

    operator QRect() const
    {
        return QRect(x, y, width, height);
    }
};
Q_DECLARE_METATYPE(DockRect)
QDBusArgument &operator<<(QDBusArgument &argument, const DockRect &rect);
const QDBusArgument &operator>>(const QDBusArgument &argument, DockRect &rect);
QDebug operator<<(QDebug deg, const DockRect &rect);

class DBusDock: public QDBusAbstractInterface
{
    Q_OBJECT
private slots:
    void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName !="org.deepin.dde.daemon.Dock1")
            return;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        foreach(const QString &prop, changedProps.keys()) {
        const QMetaObject* self = metaObject();
            for (int i=self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
                    Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
   }
public:
    static inline const char *staticInterfaceName()
    { return "org.deepin.dde.daemon.Dock1"; }
    static inline const char *staticServiceName()
    { return "org.deepin.dde.daemon.Dock1";}
    static inline const char *staticObjectPath()
    { return "/org/deepin/dde/daemon/Dock1";}
public:
    DBusDock(QObject *parent = nullptr);

    ~DBusDock();

    Q_PROPERTY(int DisplayMode READ displayMode WRITE setDisplayMode)
    inline int displayMode() const
    { return qvariant_cast< int >(property("DisplayMode")); }
    inline void setDisplayMode(int value)
    { setProperty("DisplayMode", QVariant::fromValue(value)); }

    Q_PROPERTY(QStringList DockedApps READ dockedApps)
    inline QStringList dockedApps() const
    { return qvariant_cast< QStringList >(property("DockedApps")); }

    Q_PROPERTY(QList<QDBusObjectPath> Entries READ entries)
    inline QList<QDBusObjectPath> entries() const
    { return qvariant_cast< QList<QDBusObjectPath> >(property("Entries")); }

    Q_PROPERTY(DockRect FrontendWindowRect READ frontendWindowRect NOTIFY FrontendWindowRectChanged)
    inline DockRect frontendWindowRect() const
    { return qvariant_cast< DockRect >(property("FrontendWindowRect")); }

    Q_PROPERTY(int HideMode READ hideMode WRITE setHideMode NOTIFY HideModeChanged)
    inline int hideMode() const
    { return qvariant_cast< int >(property("HideMode")); }
    inline void setHideMode(int value)
    { setProperty("HideMode", QVariant::fromValue(value)); }

    Q_PROPERTY(int HideState READ hideState)
    inline int hideState() const
    { return qvariant_cast< int >(property("HideState")); }

    Q_PROPERTY(uint HideTimeout READ hideTimeout WRITE setHideTimeout)
    inline uint hideTimeout() const
    { return qvariant_cast< uint >(property("HideTimeout")); }
    inline void setHideTimeout(uint value)
    { setProperty("HideTimeout", QVariant::fromValue(value)); }

    Q_PROPERTY(uint IconSize READ iconSize WRITE setIconSize)
    inline uint iconSize() const
    { return qvariant_cast< uint >(property("IconSize")); }
    inline void setIconSize(uint value)
    { setProperty("IconSize", QVariant::fromValue(value)); }

    Q_PROPERTY(double Opacity READ opacity WRITE setOpacity)
    inline double opacity() const
    { return qvariant_cast< double >(property("Opacity")); }
    inline void setOpacity(double value)
    { setProperty("Opacity", QVariant::fromValue(value)); }

    Q_PROPERTY(int Position READ position WRITE setPosition)
    inline int position() const
    { return qvariant_cast< int >(property("Position")); }
    inline void setPosition(int value)
    { setProperty("Position", QVariant::fromValue(value)); }

    Q_PROPERTY(uint ShowTimeout READ showTimeout WRITE setShowTimeout)
    inline uint showTimeout() const
    { return qvariant_cast< uint >(property("ShowTimeout")); }
    inline void setShowTimeout(uint value)
    { setProperty("ShowTimeout", QVariant::fromValue(value)); }

    Q_PROPERTY(uint WindowSize READ windowSize WRITE setWindowSize)
    inline uint windowSize() const
    { return qvariant_cast< uint >(property("WindowSize")); }
    inline void setWindowSize(uint value)
    { setProperty("WindowSize", QVariant::fromValue(value)); }

    Q_PROPERTY(uint WindowSizeEfficient READ windowSizeEfficient WRITE setWindowSizeEfficient)
    inline uint windowSizeEfficient() const
    { return qvariant_cast< uint >(property("WindowSizeEfficient")); }
    inline void setWindowSizeEfficient(uint value)
    { setProperty("WindowSizeEfficient", QVariant::fromValue(value)); }

    Q_PROPERTY(uint WindowSizeFashion READ windowSizeFashion WRITE setWindowSizeFashion)
    inline uint windowSizeFashion() const
    { return qvariant_cast< uint >(property("WindowSizeFashion")); }
    inline void setWindowSizeFashion(uint value)
    { setProperty("WindowSizeFashion", QVariant::fromValue(value)); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> ActivateWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("ActivateWindow"), argumentList);
    }

    inline QDBusPendingReply<> CancelPreviewWindow()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("CancelPreviewWindow"), argumentList);
    }

    inline QDBusPendingReply<> CloseWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("CloseWindow"), argumentList);
    }

    inline QDBusPendingReply<QStringList> GetDockedAppsDesktopFiles()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetDockedAppsDesktopFiles"), argumentList);
    }

    inline QDBusPendingReply<QStringList> GetEntryIDs()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetEntryIDs"), argumentList);
    }

    inline QDBusPendingReply<QString> GetPluginSettings()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetPluginSettings"), argumentList);
    }

    inline QDBusPendingReply<bool> IsDocked(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("IsDocked"), argumentList);
    }

    inline QDBusPendingReply<bool> IsOnDock(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("IsOnDock"), argumentList);
    }

    inline QDBusPendingReply<> MakeWindowAbove(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("MakeWindowAbove"), argumentList);
    }

    inline QDBusPendingReply<> MaximizeWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("MaximizeWindow"), argumentList);
    }

    inline QDBusPendingReply<> MergePluginSettings(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("MergePluginSettings"), argumentList);
    }

    inline QDBusPendingReply<> MinimizeWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("MinimizeWindow"), argumentList);
    }

    inline QDBusPendingReply<> MoveEntry(int in0, int in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("MoveEntry"), argumentList);
    }

    inline QDBusPendingReply<> MoveWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("MoveWindow"), argumentList);
    }

    inline QDBusPendingReply<> PreviewWindow(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("PreviewWindow"), argumentList);
    }

    inline QDBusPendingReply<QString> QueryWindowIdentifyMethod(uint in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("QueryWindowIdentifyMethod"), argumentList);
    }

    inline QDBusPendingReply<> RemovePluginSettings(const QString &in0, const QStringList &in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("RemovePluginSettings"), argumentList);
    }

    inline QDBusPendingReply<bool> RequestDock(const QString &in0, int in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("RequestDock"), argumentList);
    }

    inline QDBusPendingReply<bool> RequestUndock(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("RequestUndock"), argumentList);
    }

    inline QDBusPendingReply<> SetFrontendWindowRect(int in0, int in1, uint in2, uint in3)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2) << QVariant::fromValue(in3);
        return asyncCallWithArgumentList(QStringLiteral("SetFrontendWindowRect"), argumentList);
    }

    inline QDBusPendingReply<> SetPluginSettings(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetPluginSettings"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void DockAppSettingsSynced();
    void EntryAdded(const QDBusObjectPath &in0, int in1);
    void EntryRemoved(const QString &in0);
    void PluginSettingsSynced();
    void ServiceRestarted();
    void FrontendWindowRectChanged();
    void HideModeChanged();
};

namespace org {
  namespace deepin {
    namespace dde {
      namespace daemon {
        typedef ::DBusDock Dock1;
      }
    }
  }
}
#endif
