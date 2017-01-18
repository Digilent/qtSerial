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
    //Set the baud rate to 9600 before opening to grease the wheels for Mac
    this->setBaudRate(9600);

    if(this->port.open(QIODevice::ReadWrite)) {
        //Update to the actual desired baud rate
        this->port.setBaudRate(baudRate);
        return true;
    } else {
        return false;
    }
}


//Open the specified port with the specified baud rate.  Returns true if successful, false otherwise.
bool Serial::open(QSerialPortInfo portInfo, qint32 baudRate){
    //QSerialPort port;
    this->port.setPort(portInfo);

    //Set the baud rate to 9600 before opening to grease the wheels for Mac
    this->setBaudRate(9600);

    if(this->port.open(QIODevice::ReadWrite)) {
        //Update to the actual desired baud rate
        this->port.setBaudRate(baudRate);
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

//Write the specified data to the serial port.  Read response data until the number of ms specified in timeout ellaspes between response bytes, then return response data.
QByteArray Serial::writeRead(QByteArray data, int timeout){
  return writeRead(data, timeout, timeout);
}

//Write the specified data to the serial port.  Delay up to the specified delay ms for the first response byte,
//then read response data until the number of ms specified in timeout ellaspes between response bytes, then return response data.
QByteArray Serial::writeRead(QByteArray data, int delay, int timeout) {
    QByteArray resp;

    this->write(data);
    if(this->port.waitForReadyRead(delay)){
       //Read first incoming data
        resp.append(this->port.readAll());

        //Continue reading until timout expires
        while(this->port.waitForReadyRead(timeout)) {
            resp.append(this->port.readAll());
        }
        return resp;
    }
    return resp;
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
    //Set the baud rate back to 9600 before closing to grease the wheels for Mac
    this->setBaudRate(9600);
    this->port.close();
}

//Refresh the system serial port info and return it.
QList<QSerialPortInfo> Serial::getSerialPortInfo() {
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


