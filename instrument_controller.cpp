/*
 * Copyright 2014 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "instrument_controller.h"
#include <QDebug>

InstrumentController::InstrumentController(QObject *parent)
    : QObject(parent)
    , m_instrument(nullptr)
    , m_interface("")
    , m_connected(false)
{
}

InstrumentController::~InstrumentController()
{
}

bool InstrumentController::connectToInstrument(const QString &interface)
{
    if (m_instrument) {
        // Disconnect existing instrument first
        disconnectFromInstrument();
    }
    
    m_interface = interface;
    m_instrument = std::make_unique<M2550_access>(m_interface);
    m_connected = m_instrument->isConnected();
    
    if (m_connected) {
        // Connect to instrument signals
        connect(m_instrument.get(), &M2550_access::M2550_ack_received, 
                this, &InstrumentController::instrumentAcknowledgeReceived);
    }
    
    emit connectionStateChanged(m_connected);
    return m_connected;
}

bool InstrumentController::isConnected() const
{
    return m_connected;
}

void InstrumentController::disconnectFromInstrument()
{
    if (m_instrument) {
        m_instrument->disconnect();
        m_connected = false;
        emit connectionStateChanged(false);
    }
}

void InstrumentController::startSingleMeasurement()
{
    if (m_instrument && m_connected) {
        m_instrument->getMeasurement();
    }
}

void InstrumentController::startContinuousMeasurement()
{
    if (m_instrument && m_connected) {
        // Continuous measurement would be handled by the timer
        m_instrument->getMeasurement();
    }
}

void InstrumentController::stopMeasurement()
{
    // Stop measurement logic - could include sending stop commands if needed
}

void InstrumentController::setMeasurementType(const QString &type)
{
    if (m_instrument && m_connected) {
        m_instrument->setMeasurement(type);
    }
}

void InstrumentController::setResolution(int resolution)
{
    if (m_instrument && m_connected) {
        m_instrument->setResolution(resolution);
    }
}

void InstrumentController::setDCAC(const QString &dcac)
{
    if (m_instrument && m_connected) {
        m_instrument->setDCAC(dcac);
    }
}

void InstrumentController::getIDN()
{
    if (m_instrument && m_connected) {
        m_instrument->getIDN();
    }
}

void InstrumentController::getSettings()
{
    if (m_instrument && m_connected) {
        m_instrument->getSettings();
    }
}

QString InstrumentController::getCurrentMessage() const
{
    if (m_instrument) {
        return m_instrument->sl_msg_m2550.join("\n");
    }
    return QString();
}

QStringList InstrumentController::getDebugMessages() const
{
    if (m_instrument) {
        return m_instrument->sl_dbg_msg_m2550;
    }
    return QStringList();
}

QStringList InstrumentController::getErrorMessages() const
{
    if (m_instrument) {
        return m_instrument->sl_err_msg_m2550;
    }
    return QStringList();
}

void InstrumentController::handleInstrumentSignal()
{
    // This method would be called when instrument signals are received
    // It could forward signals to the UI layer or process them internally
}