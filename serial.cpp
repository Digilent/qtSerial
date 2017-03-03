#include "serial.h"

#include <QCoreApplication>
#include <QMutex>
#include <QTime>
#include <QThread>

Serial::Serial(QObject *parent) : QObject(parent)
{
    qDebug() << "Serial::Serial()" << "thread: " << QThread::currentThread();
    this->port = new QSerialPort(this);
}

Serial::~Serial() {
    qDebug() << "Serial::~Serial() - Begin" << "thread: " << QThread::currentThread();
    close();
    delete this->port;
    qDebug() << "Serial::~Serial() - End" << "thread: " << QThread::currentThread();
}

//Open the specified port with the specified baud rate.  Returns true if successful, false otherwise.
bool Serial::open(QString portName, qint32 baudRate){
    //QSerialPort port;
    this->port->setPortName(portName);
    //Set the baud rate to 9600 before opening to grease the wheels for Mac
    this->setBaudRate(9600);

    if(this->port->open(QIODevice::ReadWrite)) {
        //Update to the actual desired baud rate
        this->port->setBaudRate(baudRate);
        return true;
    } else {
        return false;
    }
}

//Open the specified port with the specified baud rate.  Returns true if successful, false otherwise.
bool Serial::open(QSerialPortInfo portInfo, qint32 baudRate){
    //QSerialPort port;
    this->port->setPort(portInfo);

    //Set the baud rate to 9600 before opening to grease the wheels for Mac
    this->setBaudRate(9600);

    if(this->port->open(QIODevice::ReadWrite)) {
        //Update to the actual desired baud rate
        this->port->setBaudRate(baudRate);
        return true;
    } else {
        qDebug() << "Failed to open" << portInfo.portName() << " : " << this->port->error();
        return false;
    }
}

//Returns true if the serial port is open
bool Serial::isOpen() {
    return this->port->isOpen();
}

//Write data to the port.  Returns true if numBytes were successfully written to the port, false otherwise.
bool Serial::write(const char *data, int numBytes) {
    if(!this->port->isOpen())
    {
        return false;
    }
    else
    {
        if(this->port->write(data, numBytes) != numBytes)
        {
            return false;
        }
        return true;
    }
}

//Write data to the port.  Returns true if all bytes were successfully written, false otherwise.
bool Serial::write(QByteArray data) {
    if(!this->port->isOpen())
    {
        return false;
    }
    else
    {
        if(this->port->write(data) == data.length())
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
    if(this->port->waitForReadyRead(delay)){
       //Read first incoming data
        resp.append(this->port->readAll());

        //Continue reading until timout expires
        while(this->port->waitForReadyRead(timeout)) {
            resp.append(this->port->readAll());
        }
        return resp;
    }
    return resp;
}

//Write the specified data to the serial port.  Wait up to delay ms for the first byte to be returned.  Return an empty array if delay expires.
//While data is being returned wait up to timeout ms for the inoming byte.  Return data if timeout expires.
//Return data immediatly if a full playload or JSON or OSJB is returned
QByteArray Serial::fastWriteRead(QByteArray data, int delay, int timeout) {    
    QMutex mutex;
    mutex.lock();

    qDebug() << "Serial::fastWriteRead()" << "thread: " << QThread::currentThread() << "Send:" << data;
    QByteArray resp;
    QTime stopWatch;
    stopWatch.start();

    //Clear incoming buffer before writing new command
    qDebug() << "Flushed " << this->port->readAll().length() << "in" << stopWatch.elapsed() << "ms";

    //Write Command
    this->write(data);

    //Wait for resposne to start
    stopWatch.restart();
    if(this->port->waitForReadyRead(delay)){
        qDebug() << "Waited" << stopWatch.elapsed() << "ms for first byte";
       //Read first incoming data
        stopWatch.restart();
        resp.append(this->port->readAll());
         qDebug() << "Took" << stopWatch.elapsed() << "ms to read all data";

        //Checking if incoming data is a JSON object, OSJB, or other.
         stopWatch.restart();

        if(resp[0] == '{') {
            //---------- JSON ----------
            qDebug() <<"Incoming Data Looks Like JSON";
            int openBracketCount = 0;

            //Process initial data
            for(int i=0; i< resp.length(); i++) {
                if(resp[i] == '{') {
                    openBracketCount++;
                } else if(resp[i] == '}') {
                    openBracketCount--;
                }
                if(openBracketCount <= 0) {
                    qDebug() << "Serial::fastWriteRead()" << "thread: " << QThread::currentThread() << "Found the end in " << stopWatch.elapsed() << "- Response:" << resp;
                    emit fastWriteReadResponse(resp);
                    mutex.unlock();
                    return resp;
                }
            }

            //Continue reading until timeout expires
            stopWatch.restart();

            while(this->port->waitForReadyRead(timeout)) {
                qDebug() << "waiting for timeout for" << stopWatch.elapsed() << "ms";
                while(this->port->bytesAvailable() > 0) {
                    char respByte = this->port->read(1)[0];
                    resp.append(respByte);

                    if(respByte == '{') {
                        openBracketCount++;
                    } else if(respByte == '}') {
                        openBracketCount--;
                    }
                    if(openBracketCount <= 0) {
                        qDebug() << "Serial::fastWriteRead()" << "thread: " << QThread::currentThread() << "Response:" << resp;
                        emit fastWriteReadResponse(resp);
                        mutex.unlock();
                        return resp;
                    }
                }
            }
        }
        //else if() {
            //---------- OSJB ----------
        //}
        else {
            //---------- UNKNOWN ----------
            //Continue reading until timout expires
            qDebug() <<  "Unknown response type (probably chunked)";
                     stopWatch.restart();
            //while(this->port->waitForReadyRead(timeout)) {
            int previousReadCount = -1;
            bool waitForMoreBytes = true;
            while(waitForMoreBytes) {

                this->port->waitForReadyRead(0);
                QByteArray newData = this->port->readAll();

                resp.append(newData);

                if(newData.length() == 0) {
                    if(previousReadCount == 0) {
                        waitForMoreBytes = false;
                    } else {
                        this->delay(timeout);
                    }
                    previousReadCount = newData.length();
                }
                    qDebug() << "Read" << newData.length() << "more byte after" << stopWatch.elapsed() << "ms";
                stopWatch.restart();
                resp.append(this->port->readAll());
            }
            qDebug() << "Serial::fastWriteRead()" << "thread: " << QThread::currentThread() << "Unknown response read in" << stopWatch.elapsed() << "ms" << "Response:" << resp;
            emit fastWriteReadResponse(resp);
            mutex.unlock();
            return resp;
        }
    }
    qDebug() << "Serial::fastWriteRead()" << "thread: " << QThread::currentThread() << "Timeout - Response:" << resp;
    emit fastWriteReadResponse(resp);
    mutex.unlock();
    return resp;
}

//Returns the number of bytes available at the port or -1 if the port is not open
int Serial::bytesAvailable() {
    QTime stopWatch;
    if(!this->port->isOpen())
    {
        return -1;
    }
    else{
        stopWatch.restart();
        this->port->waitForReadyRead(0);
        int numBytes = this->port->bytesAvailable();
        //qDebug() << "Took" << stopWatch.elapsed() << "ms" << "and found" << numBytes << "bytes";
        return this->port->bytesAvailable();
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
    this->port->waitForReadyRead(0);    //This is required on Mac for bytes to be readable.  Do not change.
    return this->port->read(numBytes);
}

//Read all available bytes from the serial buffer.  Data is returned as a byte array.
QByteArray Serial::read() {
    if(!this->port->isOpen()) {
        return NULL;
    }
    QTime stopWatch;
    stopWatch.restart();
    this->port->waitForReadyRead(0);    //This is required on Mac for bytes to be readable.  Do not change.
    if(stopWatch.elapsed() > 2){
        qDebug() << "Read took took long" << stopWatch.elapsed();
    }
    return this->port->readAll();
}

//Close the serial port.
void Serial::close() {
    qDebug() << "Serial::close()" << "thread: " << QThread::currentThread();

    //Set the baud rate back to 9600 before closing to grease the wheels for Mac
    this->setBaudRate(9600);
    if(port->isOpen()) {
        this->port->close();
    }
}

//Refresh the system serial port info and return it.
QList<QSerialPortInfo> Serial::getSerialPortInfo() {
    return QSerialPortInfo::availablePorts();
}

//Assert a reset by setting RTS and DTR high for 100ms, DTR low for 50ms then DTR high for 100ms.  Returns true on success false otherwise.
bool Serial::assertReset() {
    //Set RTS and DTR
    if(this->port->setRequestToSend(true) && this->port->setDataTerminalReady(true)){
        delay(100);
        if(!this->port->setDataTerminalReady(false)) {
            return false;
        } else {
            delay(50);
            if(!this->port->setDataTerminalReady(true)) {
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
    return this->port->baudRate();
}

//Set the serial port baud rate.  Returns true on success, false otherwise.
bool Serial::setBaudRate(int baudRate) {
    return this->port->setBaudRate(baudRate);
}

//Return the current port name;
QString Serial::getName(){
    return this->port->portName();
}

//Clear all bytes from the UART input buffer and return the number of bytes returned
int Serial::flushInputBuffer(){
 qDebug() << "Serial::flushInputBuffer() Begin";
    //Byte are not available until waitForReadyRead() is called
    if(this->port != 0)    {

        QTime stopWatch;
        stopWatch.start();
       this->port->waitForReadyRead(1);
        qDebug() << "waitForReadyRead() took " << stopWatch.elapsed();
        stopWatch.restart();
       int flushCount = this->port->readAll().length();
        qDebug() << "readAll() took " << stopWatch.elapsed();

       qDebug() << "Serial::flushInputBuffer()" << flushCount;
       return flushCount;
    }else {
        return 0;
    }
}

//Soft reset the serial port.  This funciton closes and reopens the serial port with the same settings.
bool Serial::softReset(){
    QString name = this->getName();
    int baudRate = this->getBaudRate();
    this->close();
    bool success = this->open(name, baudRate);
    emit softResetResponse(success);
    return success;
}
