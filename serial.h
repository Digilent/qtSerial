#ifndef SERIAL_H
#define SERIAL_H

#include <QDebug>
#include <QList>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>

class Serial
{

public:
    Serial();

    //Open
    bool open(QString portName, qint32 baudRate);
    bool open(QSerialPortInfo portInfo, qint32 baudRate);

    //Write
    bool write(const char *data, int numBytes);
    bool write(QByteArray data);

    //Read
    QByteArray read(qint64 numBytes);
    QByteArray read();
    int bytesAvailable();
    bool waitForBytesAvailable(int numBytes, int timeout);

    //Close
    void close();

    //Utility
    bool assertReset();
    int getBaudRate();
    QString getName();
    bool isOpen();
    bool setBaudRate(int baudRate);

    static QList<QSerialPortInfo> getSerialPortInfo();
    static void delay(int ms);

private:
    QSerialPort port;

};

#endif // SERIAL_H
