/*
 * Copyright (C) 2016  Aditya Dev Sharma <aditya.sharma156696@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "datahandler.h"

#include <QtCore/QMap>
#include <QtCore/QDebug>
#include <QtDBus/QDBusReply>
#include <QtCore/QMetaObject>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>

DataHandler::DataHandler(QObject* parent): QObject(parent) {

    connect(this, &DataHandler::credentialsInsideWallet, this, &DataHandler::startDaemon);
    QMetaObject::invokeMethod(this, "checkWallet", Qt::QueuedConnection);

    QDBusConnection dbus = QDBusConnection::sessionBus();

    dbus.connect("org.kde.kdenow", "/Event", "org.kde.kdenow.event",
                 "update", this, SLOT(onEventMapReceived(QStringList, QStringList)));
    dbus.connect("org.kde.kdenow", "/Event", "org.kde.kdenow.event",
                 "loadedEventPlugin", this, SLOT(onLoadedEventPlugin()));

    dbus.connect("org.kde.kdenow", "/Flight", "org.kde.kdenow.flight",
                 "update", this, SLOT(onFlightMapReceived(QStringList, QStringList)));
    dbus.connect("org.kde.kdenow", "/Flight", "org.kde.kdenow.flight",
                 "loadedFlightPlugin", this, SLOT(onLoadedFlightPlugin()));

    dbus.connect("org.kde.kdenow", "/Hotel", "org.kde.kdenow.hotel",
                 "update", this, SLOT(onHotelMapReceived(QStringList, QStringList)));
    dbus.connect("org.kde.kdenow", "/Hotel", "org.kde.kdenow.hotel",
                 "loadedHotelPlugin", this, SLOT(onLoadedHotelPlugin()));

    dbus.connect("org.kde.kdenow", "/Restaurant", "org.kde.kdenow.restaurant",
                 "update", this, SLOT(onRestaurantMapReceived(QStringList, QStringList)));
    dbus.connect("org.kde.kdenow", "/Restaurant", "org.kde.kdenow.restaurant",
                 "loadedRestaurantPlugin", this, SLOT(onLoadedRestaurantPlugin()));
}

void DataHandler::onEventMapReceived(QStringList keys, QStringList values)
{
    QVariantMap map;
    QStringList::iterator keysIter = keys.begin();
    QStringList::iterator valuesIter = values.begin();
    while (keysIter != keys.end()) {
        map.insert((*keysIter), (*valuesIter));
        keysIter++;
        valuesIter++;
    }
    qDebug() << map << "\n";
    emit eventDataReceived(map);
}

void DataHandler::onFlightMapReceived(QStringList keys, QStringList values)
{
    QVariantMap map;
    QStringList::iterator keysIter = keys.begin();
    QStringList::iterator valuesIter = values.begin();
    while (keysIter != keys.end()) {
        map.insert((*keysIter), (*valuesIter));
        keysIter++;
        valuesIter++;
    }
    qDebug() << map << "\n";
    emit flightDataReceived(map);
}

void DataHandler::onHotelMapReceived(QStringList keys, QStringList values)
{
    QVariantMap map;
    QStringList::iterator keysIter = keys.begin();
    QStringList::iterator valuesIter = values.begin();
    while (keysIter != keys.end()) {
        map.insert((*keysIter), (*valuesIter));
        keysIter++;
        valuesIter++;
    }
    qDebug() << map << "\n";
    emit hotelDataReceived(map);
}

void DataHandler::onRestaurantMapReceived(QStringList keys, QStringList values)
{
    QVariantMap map;
    QStringList::iterator keysIter = keys.begin();
    QStringList::iterator valuesIter = values.begin();
    while (keysIter != keys.end()) {
        map.insert((*keysIter), (*valuesIter));
        keysIter++;
        valuesIter++;
    }
    qDebug() << map << "\n";
    emit restaurantDataReceived(map);
}

void DataHandler::onCredentialsInput(QString imapServer, QString imapPort, QString username,
                              QString password)
{
    m_wallet = KWallet::Wallet::openWallet("KDENowWallet",
                                            0,
                                            KWallet::Wallet::Synchronous
                                          );
    if (m_wallet->createFolder("KDENow") && m_wallet->setFolder("KDENow")) {
        qDebug() << "Created and set the folder in KWallet";
    }
    else {
        qDebug() << "Could not create or set the folder";
        return;
    }

    QMap<QString, QString> walletMap;
    walletMap.insert("imapServer", imapServer);
    walletMap.insert("imapPort", imapPort);
    walletMap.insert("username", username);
    walletMap.insert("password", password);

    int fail = m_wallet->writeMap(username, walletMap);
    if (fail) {
        qDebug() << "Could not write map";
        return;
    }
    else {
        qDebug() << "Succesfully written map";
    }
    KWallet::Wallet::closeWallet("KDENowWallet", true);
    emit credentialsInsideWallet();
}

void DataHandler::startDaemon()
{
    QDBusInterface* interface = new QDBusInterface("org.kde.kdenow", "/KDENow");

    //Call a method, to start the kdenowd daemon if it hasn't yet started
    QDBusReply< QString > reply = interface->call("startDaemon");
    if (reply.isValid()) {
        qDebug() << "Valid Reply received from org.kde.kdenow /KDENow";
        qDebug() << reply.value();
    }
    else {
        qDebug() << "Did not receive a valid reply from org.kde.kdenow /KDENow";
        return;
    }
}

void DataHandler::checkWallet()
{
    m_wallet = KWallet::Wallet::openWallet("KDENowWallet",
                                            0,
                                            KWallet::Wallet::Synchronous
                                          );
    if (!m_wallet->hasFolder("KDENow")) {
        emit credentialsInput();
    }
    else {
        emit credentialsInsideWallet();
    }
    KWallet::Wallet::closeWallet("KDENowWallet", true);
}

void DataHandler::onLoadedEventPlugin()
{
    QDBusInterface* interface = new QDBusInterface("org.kde.kdenow", "/Event");
    QDBusReply< void > reply = interface->call("getDatabaseRecordsOverDBus");
}

void DataHandler::onLoadedFlightPlugin()
{
    QDBusInterface* interface = new QDBusInterface("org.kde.kdenow", "/Flight");
    QDBusReply< void > reply = interface->call("getDatabaseRecordsOverDBus");
}

void DataHandler::onLoadedHotelPlugin()
{
    QDBusInterface* interface = new QDBusInterface("org.kde.kdenow", "/Hotel");
    QDBusReply< void > reply = interface->call("getDatabaseRecordsOverDBus");
}

void DataHandler::onLoadedRestaurantPlugin()
{
    QDBusInterface* interface = new QDBusInterface("org.kde.kdenow", "/Restaurant");
    QDBusReply< void > reply = interface->call("getDatabaseRecordsOverDBus");
}
