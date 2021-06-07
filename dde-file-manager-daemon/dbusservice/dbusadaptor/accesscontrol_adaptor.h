/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -i accesscontrol/accesscontrolmanager.h -c AccessControlAdaptor -l AccessControlManager -a dbusadaptor/accesscontrol_adaptor accesscontrol.xml
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef ACCESSCONTROL_ADAPTOR_H
#define ACCESSCONTROL_ADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "accesscontrol/accesscontrolmanager.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.filemanager.daemon.AccessControlManager
 */
class AccessControlAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.AccessControlManager")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.filemanager.daemon.AccessControlManager\">\n"
"    <signal name=\"AccessPolicySetFinished\">\n"
"      <arg direction=\"out\" type=\"a{sv}\" name=\"policy\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
"    </signal>\n"
"    <signal name=\"DeviceAccessPolicyChanged\">\n"
"      <arg direction=\"out\" type=\"av\" name=\"policy\"/>\n"
"    </signal>\n"
"    <method name=\"SetAccessPolicy\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\" name=\"policy\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
"    </method>\n"
"    <method name=\"QueryAccessPolicy\">\n"
"      <arg direction=\"out\" type=\"av\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    AccessControlAdaptor(AccessControlManager *parent);
    virtual ~AccessControlAdaptor();

    inline AccessControlManager *parent() const
    { return static_cast<AccessControlManager *>(QObject::parent()); }

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QVariantList QueryAccessPolicy();
    QString SetAccessPolicy(const QVariantMap &policy);
Q_SIGNALS: // SIGNALS
    void AccessPolicySetFinished(const QVariantMap &policy);
    void DeviceAccessPolicyChanged(const QVariantList &policy);
};

#endif
