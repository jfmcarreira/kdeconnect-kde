/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LANLINKPROVIDER_H
#define LANLINKPROVIDER_H

#include <QObject>
#include <QTcpServer>
#include <QUdpSocket>

#include "../linkprovider.h"
#include "netaddress.h"

class LanLinkProvider
    : public LinkProvider
{
    Q_OBJECT

public:
    LanLinkProvider();
    ~LanLinkProvider();

    QString name() override { return "LanLinkProvider"; }
    int priority() override { return PRIORITY_HIGH; }

public Q_SLOTS:
    virtual void onNetworkChange() override;
    virtual void onStart() override;
    virtual void onStop() override;
    void connected();
    void connectError();

private Q_SLOTS:
    void newUdpConnection();
    void newConnection();
    void dataReceived();
    void deviceLinkDestroyed(QObject* destroyedDeviceLink);
    void onNetworkConfigurationChanged(const QNetworkConfiguration &config);

private:
    static void configureSocket(QTcpSocket* socket);

    QTcpServer* mTcpServer;
    QUdpSocket* mUdpServer;
    QUdpSocket mUdpSocket;
    const static quint16 port = 1714;
    quint16 mTcpPort;

    QMap<QString, DeviceLink*> mLinks;

    struct PendingConnect {
        NetworkPackage* np;
        QHostAddress sender;
    };
    QMap<QTcpSocket*, PendingConnect> receivedIdentityPackages;
    QNetworkConfiguration m_lastConfig;

};

#endif
