/*!
 * This file is part of WatchFlower.
 * COPYRIGHT (C) 2020 Emeric Grange - All Rights Reserved
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \date      2018
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#include "device_hygrotemp_clock.h"
#include "settingsmanager.h"
#include "utils_versionchecker.h"

#include <cmath>

#include <QBluetoothUuid>
#include <QBluetoothAddress>
#include <QBluetoothServiceInfo>
#include <QLowEnergyService>

#include <QSqlQuery>
#include <QSqlError>

#include <QDateTime>
#include <QTimeZone>

#include <QDebug>

/* ************************************************************************** */

DeviceHygrotempClock::DeviceHygrotempClock(QString &deviceAddr, QString &deviceName, QObject *parent):
    Device(deviceAddr, deviceName, parent)
{
    m_capabilities += DEVICE_TEMPERATURE;
    m_capabilities += DEVICE_HUMIDITY;
    m_capabilities += DEVICE_CLOCK;
}

DeviceHygrotempClock::DeviceHygrotempClock(const QBluetoothDeviceInfo &d, QObject *parent):
    Device(d, parent)
{
    m_capabilities += DEVICE_TEMPERATURE;
    m_capabilities += DEVICE_HUMIDITY;
    m_capabilities += DEVICE_CLOCK;
}

DeviceHygrotempClock::~DeviceHygrotempClock()
{
    delete serviceDatas;
    delete serviceInfos;
}

/* ************************************************************************** */
/* ************************************************************************** */

void DeviceHygrotempClock::serviceScanDone()
{
    //qDebug() << "DeviceHygrotempClock::serviceScanDone(" << m_deviceAddress << ")";

    if (serviceDatas)
    {
        if (serviceDatas->state() == QLowEnergyService::DiscoveryRequired)
        {
            connect(serviceDatas, &QLowEnergyService::stateChanged, this, &DeviceHygrotempClock::serviceDetailsDiscovered_datas);
            //connect(serviceDatas, &QLowEnergyService::descriptorWritten, this, &DeviceHygrotempClock::confirmedDescriptorWrite);
            //connect(serviceDatas, &QLowEnergyService::characteristicRead, this, &DeviceHygrotempClock::bleReadDone);
            connect(serviceDatas, &QLowEnergyService::characteristicChanged, this, &DeviceHygrotempClock::bleReadNotify);

            serviceDatas->discoverDetails();
        }
    }

    if (serviceInfos)
    {
        if (serviceInfos->state() == QLowEnergyService::DiscoveryRequired)
        {
            connect(serviceInfos, &QLowEnergyService::stateChanged, this, &DeviceHygrotempClock::serviceDetailsDiscovered_infos);

            serviceInfos->discoverDetails();
        }
    }
}

void DeviceHygrotempClock::addLowEnergyService(const QBluetoothUuid &uuid)
{
    //qDebug() << "DeviceHygrotempClock::addLowEnergyService(" << uuid.toString() << ")";

    if (uuid.toString() == "{0000180a-0000-1000-8000-00805f9b34fb}") // infos
    {
        delete serviceInfos;

        serviceInfos = controller->createServiceObject(uuid);
        if (!serviceInfos)
            qWarning() << "Cannot create service (infos) for uuid:" << uuid.toString();
    }

    if (uuid.toString() == "{ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6}") // (unknown service) // datas
    {
        delete serviceDatas;

        serviceDatas = controller->createServiceObject(uuid);
        if (!serviceDatas)
            qWarning() << "Cannot create service (datas) for uuid:" << uuid.toString();
    }
}

void DeviceHygrotempClock::serviceDetailsDiscovered_datas(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << "DeviceHygrotempClock::serviceDetailsDiscovered_datas(" << m_deviceAddress << ") > ServiceDiscovered";

        if (serviceDatas)
        {
            SettingsManager *sm = SettingsManager::getInstance();

            // Characteristic "Units" // 1 byte READ WRITE // 0x00 - F, 0x01 - C    READ WRITE
            {
                QBluetoothUuid u(QString("EBE0CCBE-7A0A-4B0C-8A1A-6FF2997DA3A6")); // handler 0x??
                QLowEnergyCharacteristic chu = serviceDatas->characteristic(u);

                const quint8 *unit = reinterpret_cast<const quint8 *>(chu.value().constData());
                //qDebug() << "Units (0xFF: CELSIUS / 0x01: FAHRENHEIT) > " << chu.value();
                if (unit[0] == 0xFF && sm->getTempUnit() == "F")
                {
                    serviceDatas->writeCharacteristic(chu, QByteArray::fromHex("01"), QLowEnergyService::WriteWithResponse);
                }
                else if (unit[0] == 0x01&& sm->getTempUnit() == "C")
                {
                    serviceDatas->writeCharacteristic(chu, QByteArray::fromHex("FF"), QLowEnergyService::WriteWithResponse);
                }
            }

            // History
            //UUID_HISTORY = 'EBE0CCBC-7A0A-4B0C-8A1A-6FF2997DA3A6'   # Last idx 152          READ NOTIFY

            // Characteristic "Time" // 5 bytes READ WRITE
            {
                QBluetoothUuid a(QString("EBE0CCB7-7A0A-4B0C-8A1A-6FF2997DA3A6")); // handler 0x??
                QLowEnergyCharacteristic cha = serviceDatas->characteristic(a);
                //serviceDatas->readCharacteristic(cha); // trigger a new time read, not necessary

                const qint8 *timedata = reinterpret_cast<const qint8 *>(cha.value().constData());
                int8_t timezone_read = timedata[4]; Q_UNUSED(timezone_read)
                int32_t epoch_read = timedata[0];
                epoch_read += (timedata[1] << 8);
                epoch_read += (timedata[2] << 16);
                epoch_read += (timedata[3] << 24);
/*
                QDateTime time_read;
                time_read.setSecsSinceEpoch(epoch_read);
                qDebug() << "epoch READ: " << epoch_read;
                qDebug() << "QDateTime READ: " << time_read;
                qDebug() << "QTimeZone READ: " << timezone_read;
*/
                int32_t epoch_now = static_cast<int32_t>(QDateTime::currentSecsSinceEpoch()); // This device clock will not handle the year 2038...
                int8_t offset_now = static_cast<int8_t>(QDateTime::currentDateTime().offsetFromUtc() / 3600);
/*
                qDebug() << "QDateTime NOW: " << QDateTime::currentDateTime();
                qDebug() << "QTimeZone NOW: " << offset_now;
                qDebug() << "epoch NOW: " << epoch_now;
*/
                // Note: the device doesn't update its "Time" characteristic value often
                // So we don't use a single minute mismatch, but 5, to avoid reseting clock everytime
                if (std::abs(epoch_read - epoch_now) > 5*60)
                {
                    //qDebug() << "CLOCK TIME NEEDS AN UPDATE (diff: " << std::abs(epoch_read - epoch_now);

                    QByteArray timedatas_write;
                    timedatas_write.resize(5);
                    timedatas_write[0] = static_cast<char>((epoch_now      ) & 0xFF);
                    timedatas_write[1] = static_cast<char>((epoch_now >>  8) & 0xFF);
                    timedatas_write[2] = static_cast<char>((epoch_now >> 16) & 0xFF);
                    timedatas_write[3] = static_cast<char>((epoch_now >> 24) & 0xFF);
                    timedatas_write[4] = offset_now;

                    //qDebug() << "QDateTime WRITE:" << timedatas_write << " size:" << timedatas_write.size();
                    serviceDatas->writeCharacteristic(cha, timedatas_write, QLowEnergyService::WriteWithResponse);
                }
            }

            // Characteristic "Temp&Humi" // 3 bytes, READ NOTIFY
            {
                QBluetoothUuid b(QString("EBE0CCC1-7A0A-4B0C-8A1A-6FF2997DA3A6")); // handler 0x??
                QLowEnergyCharacteristic chb = serviceDatas->characteristic(b);
                m_notificationDesc = chb.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                serviceDatas->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
            }
        }
    }
}

void DeviceHygrotempClock::serviceDetailsDiscovered_infos(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << "DeviceHygrotempClock::serviceDetailsDiscovered_infos(" << m_deviceAddress << ") > ServiceDiscovered";

        if (serviceInfos)
        {
            // Characteristic "Firmware Revision String"
            QBluetoothUuid c(QString("00002a26-0000-1000-8000-00805f9b34fb")); // handler 0x06
            QLowEnergyCharacteristic chc = serviceInfos->characteristic(c);
            if (chc.value().size() > 0)
            {
               m_firmware = chc.value();
            }

            if (m_firmware.size() == 10)
            {
                if (Version(m_firmware) >= Version(LATEST_KNOWN_FIRMWARE_HYGROTEMP_CLOCK))
                {
                    m_firmware_uptodate = true;
                    Q_EMIT sensorUpdated();
                }
            }
        }
    }
}

/* ************************************************************************** */

void DeviceHygrotempClock::bleWriteDone(const QLowEnergyCharacteristic &, const QByteArray &)
{
    //qDebug() << "DeviceHygrotempClock::bleWriteDone(" << m_deviceAddress << ")";
}

void DeviceHygrotempClock::bleReadDone(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    Q_UNUSED(c)
    Q_UNUSED(value)
/*
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    qDebug() << "DeviceHygrotempClock::bleReadDone(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    qDebug() << "WE HAVE DATAS: 0x" \
               << hex << data[0] << hex << data[1] << hex << data[2] << hex << data[3] << hex << data[4];
*/
    if (c.uuid().toString().toUpper() == "{EBE0CCB7-7A0A-4B0C-8A1A-6FF2997DA3A6}")
    {
        // timedate // handler 0x??

        if (value.size() == 5)
        {
            const qint8 *timedata = reinterpret_cast<const qint8 *>(value.constData());
            qint8 timezone_read = timedata[4];
            int32_t epoch_read = timedata[0];
            epoch_read += (timedata[1] << 8);
            epoch_read += (timedata[2] << 16);
            epoch_read += (timedata[3] << 24);

            QDateTime time_read;
            time_read.setSecsSinceEpoch(epoch_read);
            qDebug() << "QDateTime READ: " << time_read;
            qDebug() << "QTimeZone READ: " << timezone_read;
        }
    }
}

void DeviceHygrotempClock::bleReadNotify(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
/*
    qDebug() << "DeviceHygrotempClock::bleReadNotify(" << m_deviceAddress << ") on" << c.name() << " / uuid" << c.uuid() << value.size();
    qDebug() << "WE HAVE DATAS: 0x" \
               << hex << data[0] << hex << data[1] << hex << data[2] << hex << data[3] << hex << data[4];
*/
    if (c.uuid().toString().toUpper() == "{EBE0CCC1-7A0A-4B0C-8A1A-6FF2997DA3A6}")
    {
        // sensor datas // handler 0x??

        if (value.size() == 3)
        {
            m_temp = static_cast<int16_t>(data[0] + (data[1] << 8)) / 100.f;
            m_hygro = data[2];

            m_lastUpdate = QDateTime::currentDateTime();

#ifndef QT_NO_DEBUG
            qDebug() << "* DeviceHygrotempClock update:" << getAddress();
            qDebug() << "- m_firmware:" << m_firmware;
            qDebug() << "- m_battery:" << m_battery;
            qDebug() << "- m_temp:" << m_temp;
            qDebug() << "- m_hygro:" << m_hygro;
#endif

            controller->disconnectFromDevice();

            //if (m_db)
            {
                // SQL date format YYYY-MM-DD HH:MM:SS
                QString tsStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:00:00");
                QString tsFullStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

                QSqlQuery addDatas;
                addDatas.prepare("REPLACE INTO datas (deviceAddr, ts, ts_full, temp, hygro)"
                                 " VALUES (:deviceAddr, :ts, :ts_full, :temp, :hygro)");
                addDatas.bindValue(":deviceAddr", getAddress());
                addDatas.bindValue(":ts", tsStr);
                addDatas.bindValue(":ts_full", tsFullStr);
                addDatas.bindValue(":temp", m_temp);
                addDatas.bindValue(":hygro", m_hygro);
                if (addDatas.exec() == false)
                    qWarning() << "> addDatas.exec() ERROR" << addDatas.lastError().type() << ":" << addDatas.lastError().text();

                QSqlQuery updateDevice;
                updateDevice.prepare("UPDATE devices SET deviceFirmware = :firmware, deviceBattery = :battery WHERE deviceAddr = :deviceAddr");
                updateDevice.bindValue(":firmware", m_firmware);
                updateDevice.bindValue(":battery", m_battery);
                updateDevice.bindValue(":deviceAddr", getAddress());
                if (updateDevice.exec() == false)
                    qWarning() << "> updateDevice.exec() ERROR" << updateDevice.lastError().type() << ":" << updateDevice.lastError().text();
            }

            refreshDatasFinished(true);
        }
    }
}

void DeviceHygrotempClock::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    //qDebug() << "DeviceHygrotempClock::confirmedDescriptorWrite!";

    if (d.isValid() && d == m_notificationDesc && value == QByteArray::fromHex("0000"))
    {
        qDebug() << "confirmedDescriptorWrite() disconnect?!";

        //disabled notifications -> assume disconnect intent
        //m_control->disconnectFromDevice();
        //delete m_service;
        //m_service = nullptr;
    }
}
