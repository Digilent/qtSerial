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

    bool open(QString portName, qint32 baudRate);
    bool open(QSerialPortInfo portInfo, qint32 baudRate);
    bool isOpen();
    bool assertReset();
    bool write(const char *data, int numBytes);
    bool write(QByteArray data);
    int bytesAvailable();
    bool waitForBytesAvailable(int numBytes, int timeout);
    int getBaudRate();
    QString getName();
    bool setBaudRate(int baudRate);
    QByteArray read(qint64 numBytes);
    QByteArray read();
    void close();


    static QList<QSerialPortInfo> refreshSerialPortInfo();
    static void delay(int ms);

private:
    QSerialPort port;

};

#endif // SERIAL_H
