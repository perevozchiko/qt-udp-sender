#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QUdpSocket;
class QTimer;

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT    

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    void startSending();
    void stopSending();
    void sendData();
    void onRead();
    void startListening();
    void stopListening();
    void readSettingsFromForm();
    QByteArray getRandomHex(unsigned int length);
    uchar calcCheckSum(const QByteArray& raw);

    QUdpSocket* listener{nullptr};
    QUdpSocket* sender{nullptr};
    Ui::Dialog* ui{nullptr};
    QString host;
    unsigned int port{28867};
    unsigned int resendDelay{0};
    QTimer* resendTimer{nullptr};
    unsigned long long lostPacketCount{0};
    unsigned long long breakPacketCount{0};
    unsigned long long packetNumberSended{0};
    unsigned long long packetNumberReceived{0};
};
#endif // DIALOG_H
