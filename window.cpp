#include "window.h"
#include "ui_window.h"
#include "receiver.h"
#include "sender.h"

Receiver recvObj;

struct argStruct {
    Receiver *auxRecvObj;
    Ui::Window *auxUi;
};


Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    deactivate =  false;
    ui->setupUi(this);
}

Window::~Window()
{
    delete ui;
}


int Window::on_connectButton_clicked() {
    int localCode;
    QString auxMessage;

    recvObj.setSrPortNumber(ui->socketPort->text().toInt());

    recvObj.create_sr_socket_connection();
    if(recvObj.getSrSocketFD() < 0) {
        ui->messagesPanel->appendPlainText("Receiver socket creation failed.");
        ui->labelPort->setText("Error!");
        return 0;
    }

    localCode = recvObj.bind_sr_ISAddress();
    if(localCode < 0) {
        ui->messagesPanel->appendPlainText("Receiver could not bind the ISAddress.");
        ui->labelPort->setText("Error!");
        return 0;
    }

    auxMessage=QString::number(recvObj.getSrPortNumber());
    ui->messagesPanel->appendPlainText("Waiting on port: "+auxMessage);
    ui->labelPort->setText("Ok!");

    recvObj.init_payload(ui->spinBoxPayload->value());
    auxMessage=QString::number(recvObj.getPayloadSamples());
    ui->messagesPanel->appendPlainText("Package audio payload (Samples) :"+auxMessage);
    auxMessage=QString::number(recvObj.getPayloadBytes());
    ui->messagesPanel->appendPlainText("Package audio payload (Bytes) :"+auxMessage);

    ui->connectButton->setEnabled(false);
    return 1;
}


int Window::on_createButton_clicked()
{
    bool createFileResult;

    recvObj.setSoundFileName(ui->fileName->text().toStdString());
    recvObj.setFileSampleRate(ui->fileSampleRate->text().toInt());
    recvObj.setFileNChannels(ui->fileChannels->text().toInt());
    QString localFileName = QString::fromStdString(recvObj.getSoundFileName());


    ui->messagesPanel->appendPlainText("Creating file with name: "
                                       +localFileName);

    createFileResult = recvObj.create_file(SF_FORMAT_RF64 | SF_FORMAT_PCM_24);

    if (createFileResult == true) {
        ui->messagesPanel->appendPlainText("File created.");
        ui->labelFileCreation->setText("Ok!");
    }
    else {
        ui->messagesPanel->appendPlainText("Error creating the file.");
        ui->labelFileCreation->setText("Error!");
        return 0;
    }

    ui->createButton->setEnabled(false);
    return 1;
}

void *update_receiver_counters(void *arg) {
    argStruct parameters = *((argStruct*) arg);
    Receiver *receiver = parameters.auxRecvObj;

    while(1) {
        QString counter1 = QString::number(receiver->getLostNetworkPackages());
        parameters.auxUi->LNPCounter->setText(counter1);
        QString counter2 = QString::number(receiver->getOverflowCounter());
        parameters.auxUi->overflowCounter->setText(counter2);
        QString counter3 = QString::number(receiver->getUnderflowCounter());
        parameters.auxUi->underflowCounter->setText(counter3);
    }
}

int Window::on_activateButton_clicked() {
    pthread_t netcomThread, netcomThread2;
    int threadCheck;

    argStruct argStructObj;
    argStructObj.auxRecvObj = &recvObj;
    argStructObj.auxUi = ui;

    recvObj.setOutputMode(ui->comboBox->currentIndex());

    if (recvObj.open_jack_client("receiver_client") == true) {
        ui->messagesPanel->appendPlainText("JACK client opened.");
        QString aux1 = QString::fromStdString(recvObj.getClientName());
        QString aux2 = QString::number(recvObj.getSampleRate());
        ui->clientName->setText(aux1);
        ui->sampleRate->setText(aux2);
    }
    else {
        ui->messagesPanel->appendPlainText("Error opening the JACK client.");
        ui->messagesPanel->appendPlainText("Is it JACK server activated? !!!");
        ui->clientName->setText("Error!");
        ui->sampleRate->setText("Error!");
        return 0;
    }

//    Tell the Jack server to call @a process_callback whenever there is
//    work be done, passing @a arg as the second argument.
    recvObj.set_callback_method();
    ui->messagesPanel->appendPlainText("JACK client: Assigned callback function.");
    recvObj.jack_port_make_standard(recvObj.getChannels());
    ui->messagesPanel->appendPlainText("JACK client: Ports created.");
    recvObj.jack_client_activate(recvObj.getClientfd());
    ui->messagesPanel->appendPlainText("JACK client: Activated.");


    ui->messagesPanel->appendPlainText("Creating receiver threads.");

    threadCheck = pthread_create(&netcomThread,
                                 NULL, recvObj.receiver_thread, &recvObj);
    if (threadCheck != 0)
        ui->messagesPanel->appendPlainText("Receiver 1st thread error.");

    threadCheck = pthread_create(&netcomThread2,
                                 NULL, update_receiver_counters, &argStructObj);
    if (threadCheck != 0)
        ui->messagesPanel->appendPlainText("Receiver 2nd thread error.");

    ui->messagesPanel->appendPlainText("Working...");

    while (deactivate == false) {
        QCoreApplication::processEvents();
    }

//    pthread_join(netcomThread, NULL);
//    pthread_join(netcomThread2, NULL);
    pthread_cancel(netcomThread);
    pthread_cancel(netcomThread2);
    recvObj.finish();

    ui->messagesPanel->appendPlainText("Client deactivated. Please, restart"
                                       " the program.");

    return 0;
}





void Window::on_deactivateButton_clicked()
{
    deactivate = true;
}

void Window::on_createButton_2_clicked()
{
    recvObj.setChannels(ui->fileChannels->text().toInt());
    ui->messagesPanel->appendPlainText("Number of channels: "
                                       +ui->fileChannels->text());
}
