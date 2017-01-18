#include "serial.h"

#include <QCoreApplication>
#include <QTime>

Serial::Serial()
{

}

//Open the specified port with the specified baud rate.  Returns true if successful, false otherwise.
bool Serial::open(QString portName, qint32 baudRate){
    //QSerialPort port;
    this->port.setPortName(portName);
    this->port.setBaudRate(baudRate);
    return this->port.open(QIODevice::ReadWrite);
}

//Open the specified port with the specified baud rate.  Returns true if successful, false otherwise.
bool Serial::open(QSerialPortInfo portInfo, qint32 baudRate){
    //QSerialPort port;
    this->port.setPort(portInfo);
    this->port.setBaudRate(baudRate);
    if(this->port.open(QIODevice::ReadWrite)) {
        return true;
    } else {
        qDebug() << "Failed to open" << portInfo.portName() << " : " << this->port.error();
        return false;
    }
}

//Returns true if the serial port is open
bool Serial::isOpen() {
    return this->port.isOpen();
}

//Write data to the port.  Returns true if numBytes were successfully written to the port, false otherwise.
bool Serial::write(const char *data, int numBytes) {
    if(!this->port.isOpen())
    {
        return false;
    }
    else
    {
        if(this->port.write(data, numBytes) != numBytes)
        {
            return false;
        }
        return true;
    }
}

//Write data to the port.  Returns true if all bytes were successfully written, false otherwise.
bool Serial::write(QByteArray data) {
    if(!this->port.isOpen())
    {
        return false;
    }
    else
    {
        if(this->port.write(data) == data.length())
        {
            return true;
        }
        return false;
    }
}



//Returns the number of bytes available at the port or -1 if the port is not open
int Serial::bytesAvailable() {    
    this->port.waitForReadyRead(0);
    if(!this->port.isOpen())
    {
        return -1;
    }
    else{
        return this->port.bytesAvailable();
    }
}

//Wait for the specified number of bytes to be avialable or timeout.  Returns true if the specified number of bytes are available, false otherwise.
bool Serial::waitForBytesAvailable(int numBytes, int timeout) {

    QTime startTime = QTime::currentTime();
    QTime doneTime = startTime.addMSecs(timeout);
    while(QTime::currentTime() < doneTime) {
        if(bytesAvailable() >= numBytes) {
            return true;
        }
    }
    return false;
}

/*
//Read the specified number of bytes from the port into the provided buffer.  This function returns true if numBytes were succesfully read and false otherwise.
bool Serial::read(char* rxBuffer, int numBytes) {
    this->port.waitForReadyRead(0);
    if(!this->port.isOpen())
    {
        //Port not open
        return false;
    }
    else if(this->port.bytesAvailable() < numBytes)
    {
        //Not enough bytes available
        return false;
    }
    else {
        if(this->port.read(rxBuffer, numBytes) < 0){
            return false;
        }
        return true;
    }
}
*/

//Read the specified number of bytes from the serial buffer.  Data is returned as a byte array.
QByteArray Serial::read(qint64 numBytes) {
    this->port.waitForReadyRead(0);
    return this->port.read(numBytes);
}

//Read all available bytes from the serial buffer.  Data is returned as a byte array.
QByteArray Serial::read() {
    if(!this->port.isOpen()) {
        return NULL;
    }
    this->port.waitForReadyRead(0);
    return this->port.readAll();
}


//Closes the serial port.
void Serial::close() {
    this->port.close();
}

//Refresh the system serial port info.
QList<QSerialPortInfo> Serial::refreshSerialPortInfo() {
    return QSerialPortInfo::availablePorts();
}

//Assert a reset by setting RTS and DTR high for 100ms, DTR low for 50ms then DTR high for 100ms.  Returns true on success false otherwise.
bool Serial::assertReset() {
    //Set RTS and DTR
    if(this->port.setRequestToSend(true) && this->port.setDataTerminalReady(true)){
        delay(100);
        if(!this->port.setDataTerminalReady(false)) {
            return false;
        } else {
            delay(50);
            if(!this->port.setDataTerminalReady(true)) {
                return false;
            } else {
                delay(100);
                return true;
            }
        }
    } else {
        return false;
    }
}

//Delay the specified number of ms.
void Serial::delay(int ms){
    QTime stopWatch;
    stopWatch.start();
    QTime startTime = QTime::currentTime();
    QTime doneTime = startTime.addMSecs(ms);
    while (QTime::currentTime() < doneTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
    }
}


int Serial::getBaudRate() {
    return this->port.baudRate();
}

//Set the serial port baud rate.  Returns true on success, false otherwise.
bool Serial::setBaudRate(int baudRate) {
    return this->port.setBaudRate(baudRate);
}

//Return the current port name;
QString Serial::getName(){
    return this->port.portName();
}


