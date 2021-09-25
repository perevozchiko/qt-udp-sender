#include "Dialog.h"
#include "./ui_Dialog.h"
#include <QUdpSocket>
#include <QTimer>
#include <QTime>

// Default settings
const QString DefaultHost = "192.168.0.88";
const QString DefaultPort = "28867";
const unsigned int MaxUdpDataSize = 1440; // byte
const unsigned int RandomDataSize = 100; // byte
const unsigned int CrcSumSize = 1; // byte
const unsigned int PacketNumberSize = 16;
const unsigned int CountPackets = 10;


Dialog::Dialog(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::Dialog),
    host(DefaultHost)
{
    ui->setupUi(this);
    ui->hostLineEdit->setText(DefaultHost);
    ui->portLineEdit->setText(DefaultPort);
    ui->stopButton->setEnabled(false);
    ui->stopListeningButton->setEnabled(false);

    setWindowTitle("UDP");
    resize(320, 370);

    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    connect(ui->sendButton, &QPushButton::clicked, this, &Dialog::startSending);
    connect(ui->stopButton, &QPushButton::clicked, this, &Dialog::stopSending);
    connect(ui->startListenButton, &QPushButton::clicked, this, &Dialog::startListening);
    connect(ui->stopListeningButton, &QPushButton::clicked, this, &Dialog::stopListening);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::startSending()
{    
    readSettingsFromForm();

    const auto speed = ui->speedLineEdit->text().toInt();
    if (speed > 0)
    {
        resendDelay = CountPackets * MaxUdpDataSize * 1000.0 / (speed * 1024 * 1024 / 8.0);
    }
    ui->resendLabel->setText(QString::number(CountPackets) + " packets (every " + QString::number(resendDelay) + " msec)");

    if (resendDelay > 0)
    {
        resendTimer = new QTimer(this);
        resendTimer->setTimerType(Qt::TimerType::PreciseTimer);
        connect(resendTimer, &QTimer::timeout, this, &Dialog::sendData);
        resendTimer->start(resendDelay);
        ui->startListenButton->setEnabled(false);
        ui->stopListeningButton->setEnabled(false);
        ui->stopButton->setEnabled(true);
        ui->sendButton->setEnabled(false);
        ui->sendButton->setText("Sending...");
        setWindowTitle("Sending to " + host + ":" + QString::number(port));
    }
    sender = new QUdpSocket(this);
    sendData();
}

void Dialog::stopSending()
{
    if(resendTimer && resendTimer->isActive())
    {
        resendTimer->stop();
        disconnect(resendTimer, &QTimer::timeout, this, &Dialog::sendData);
    }

    resendDelay = 0;
    ui->startListenButton->setEnabled(true);
    ui->sendButton->setEnabled(true);
    ui->sendButton->setText("Send");
    ui->stopButton->setEnabled(false);
    ui->resendLabel->clear();
    packetNumberSended = 0;

    setWindowTitle("UDP");
    if (sender)
    {
        sender->deleteLater();
    }
}

void Dialog::sendData()
{
    if (!sender)
    {
        qDebug() << "Sender doesn't initialize";
        return;
    }

    auto counter = CountPackets;

    while(counter--)
    {
        QByteArray data;
        QByteArray randomData = getRandomHex(RandomDataSize);
        data.append(randomData);

        // fill byteArray '\0'
        data.resize(MaxUdpDataSize - CrcSumSize - PacketNumberSize);

        packetNumberSended++;
        QByteArray numberByteArray;
        numberByteArray.setNum(packetNumberSended);
        numberByteArray.resize(PacketNumberSize);

        data.append(numberByteArray);
        uchar sum = calcCheckSum(data);
        data.append(sum);
        sender->writeDatagram(data, QHostAddress(host), port);
    }

    ui->sendedPacketsLabel->setText(QString::number(packetNumberSended));
}

void Dialog::updateLabels()
{
    ui->lostPacketLabel->setText(QString::number(lostPacketCount));
    ui->recievedPacketsLabel->setText(QString::number(packetNumberReceived));
    ui->breakPacketsLabel->setText(QString::number(breakPacketCount));
}

void Dialog::onRead()
{
    if (!listener)
    {
        qDebug() << "Listener doesn't initialize";
        return;
    }

    QByteArray data;
    while (listener->hasPendingDatagrams())
    {
        const auto size = listener->pendingDatagramSize();
        if (size > 0 && size == MaxUdpDataSize)
        {
            data.resize(size);
            listener->readDatagram(data.data(), size);
            const auto payloadSize = size - PacketNumberSize - CrcSumSize;

            uchar expectedCrc = data.mid(size - CrcSumSize, CrcSumSize).at(0);
            uchar actualCrc = calcCheckSum(data.mid(0, size - CrcSumSize));

            if (expectedCrc != actualCrc)
            {
                breakPacketCount++;
                continue;
            }

            const auto numArr = data.mid(payloadSize, size-CrcSumSize);
            bool ok = false;
            const auto num = numArr.toLongLong(&ok);
            if (!ok)
            {
                breakPacketCount++;
                continue;
            }

            if (num == 0)
            {
                packetNumberReceived = 0;
            }
            packetNumberReceived++;

            if (packetNumberReceived != num)
            {
                lostPacketCount++;
                packetNumberReceived = num;
            }
        }
    }

    updateLabels();
}

void Dialog::startListening()
{
    readSettingsFromForm();
    ui->sendButton->setEnabled(false);
    ui->stopButton->setEnabled(false);
    ui->startListenButton->setEnabled(false);
    ui->stopListeningButton->setEnabled(true);
    ui->startListenButton->setText("Listening...");
    listener = new QUdpSocket(this);
    listener->bind(port);
    connect(listener, &QUdpSocket::readyRead, this, &Dialog::onRead);
    setWindowTitle("Listening " + host + ":" + QString::number(port));
}

void Dialog::stopListening()
{
    breakPacketCount = 0;
    lostPacketCount = 0;
    packetNumberReceived = 0;
    ui->startListenButton->setEnabled(true);
    ui->startListenButton->setText("Listen");
    ui->stopListeningButton->setEnabled(false);
    ui->sendButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    setWindowTitle("UDP");
    disconnect(listener, &QUdpSocket::readyRead, this, &Dialog::onRead);
    if(listener)
    {
        listener->deleteLater();
    }
}

void Dialog::readSettingsFromForm()
{
    bool ok = false;
    const auto portCurrent = ui->portLineEdit->text().toInt(&ok);
    if (ok)
    {
        port = portCurrent;
    }

    host = ui->hostLineEdit->text();
}


QByteArray Dialog::getRandomHex(unsigned int length)
{
    QByteArray randomHex;

    for(int i = 0; i < length; i++)
    {
        int n = qrand() % 16;
        randomHex.append(QString::number(n, 16));
    }

    return randomHex;
}

uchar Dialog::calcCheckSum(const QByteArray& raw)
{
    int crc = 0;
    for (int i = 0; i < raw.count(); ++i)
    {
        crc += raw.at(i) & 0xff;
    }
    return crc % 256;
}

