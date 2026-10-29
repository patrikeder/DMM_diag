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

#ifndef INSTRUMENT_CONTROLLER_H
#define INSTRUMENT_CONTROLLER_H

#include <QObject>
#include <QString>
#include <memory>
#include "M2550/m2550_access.h"

class InstrumentController : public QObject
{
    Q_OBJECT

public:
    explicit InstrumentController(QObject *parent = nullptr);
    ~InstrumentController();

    // Connection and communication methods
    bool connectToInstrument(const QString &interface);
    bool isConnected() const;
    void disconnectFromInstrument();
    
    // Measurement methods
    void startSingleMeasurement();
    void startContinuousMeasurement();
    void stopMeasurement();
    
    // Settings methods
    void setMeasurementType(const QString &type);
    void setResolution(int resolution);
    void setDCAC(const QString &dcac);
    
    // Query methods
    void getIDN();
    void getSettings();
    
    // Status information
    QString getCurrentMessage() const;
    QStringList getDebugMessages() const;
    QStringList getErrorMessages() const;

signals:
    void measurementReceived(const QString &value);
    void connectionStateChanged(bool connected);
    void messageReceived(const QString &message);
    void debugMessageReceived(const QString &debugMessage);
    void errorMessageReceived(const QString &errorMessage);
    void instrumentAcknowledgeReceived();

private:
    std::unique_ptr<M2550_access> m_instrument;
    QString m_interface;
    bool m_connected;
    
    // Helper methods
    void handleInstrumentSignal();
};

#endif // INSTRUMENT_CONTROLLER_H