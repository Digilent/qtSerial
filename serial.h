#ifndef SERIAL_H
#define SERIAL_H

#include <QDebug>
#include <QList>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>

class Serial : public QObject
{
    Q_OBJECT

public:
    explicit Serial(QObject *parent = 0);
    ~Serial();

    //Open
    bool open(QString portName, qint32 baudRate);
    bool open(QSerialPortInfo portInfo, qint32 baudRate);

    //Write
    bool write(const char *data, int numBytes);
    bool write(QByteArray data);

    QByteArray writeRead(QByteArray data, int timeout);
    QByteArray writeRead(QByteArray data, int delay, int timeout);

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
    int flushInputBuffer();

    static QList<QSerialPortInfo> getSerialPortInfo();
    static void delay(int ms);

signals:
    fastWriteReadResponse(QByteArray response);
    softResetResponse(bool status);

public slots:
    QByteArray fastWriteRead(QByteArray data, int delay, int timeout);
    bool softReset();


private:
    QSerialPort* port;


};

#endif // SERIAL_H
