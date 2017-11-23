#include "window.h"
#include "ui_window.h"
#include "receiver.h"

Receiver recvObj;

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    deactivate =  false;
    ui->setupUi(this);
    ui->createButton_2->setEnabled(false);
    ui->createButton->setEnabled(false);
    ui->activateButton->setEnabled(false);
    ui->deactivateButton->setEnabled(false);
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
    ui->socketPort->setEnabled(false);
    ui->spinBoxPayload->setEnabled(false);
    return 1;
}


int Window::on_createButton_clicked()
{
    bool createFileResult;

    recvObj.setChannels(ui->spinBoxChannels->value());

    recvObj.setSoundFileName(ui->fileName->text().toStdString());
    recvObj.setFileSampleRate(ui->fileSampleRate->text().toInt());
    recvObj.setFileNChannels(ui->spinBoxChannels->value());
    QString localFileName = QString::fromStdString(recvObj.getSoundFileName());


    ui->messagesPanel->appendPlainText("Creating file with name: "
                                       +localFileName);

    createFileResult = recvObj.create_file(SF_FORMAT_WAV | SF_FORMAT_PCM_24);

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

int Window::on_activateButton_clicked() {
    pthread_t netcomThread;
    QString counter;
    int threadCheck;

    recvObj.setOutputMode(ui->comboBox->currentIndex());
    ui->activateButton->setEnabled(false);
    ui->deactivateButton->setEnabled(true);

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
    recvObj.jack_port_make_standard(2);
    ui->messagesPanel->appendPlainText("JACK client: Ports created.");
    recvObj.jack_client_activate(recvObj.getClientfd());
    ui->messagesPanel->appendPlainText("JACK client: Activated.");


    ui->messagesPanel->appendPlainText("Creating receiver thread.");

    threadCheck = pthread_create(&netcomThread,
                                 NULL, recvObj.receiver_thread, &recvObj);
    if (threadCheck != 0)
        ui->messagesPanel->appendPlainText("Receiver 1st thread error.");


    ui->messagesPanel->appendPlainText("Working...");

    while (deactivate == false) {
        counter = QString::number(recvObj.getLostNetworkPackages());
        ui->LNPCounter->setText(counter);
        counter = QString::number(recvObj.getOverflowCounter());
        ui->overflowCounter->setText(counter);
        counter = QString::number(recvObj.getUnderflowCounter());
        ui->underflowCounter->setText(counter);
        counter = QString::number(recvObj.getIndexControl());
        ui->labelIndexControl->setText(counter);

        QCoreApplication::processEvents();
    }

    pthread_join(netcomThread, NULL);
    recvObj.finish();

    ui->messagesPanel->appendPlainText("Client deactivated. Please, restart"
                                       " the program.");

    return 0;
}


void Window::on_deactivateButton_clicked()
{
    deactivate = true;
    recvObj.setDeactivateSignal(true);
    ui->deactivateButton->setEnabled(false);
}

void Window::on_createButton_2_clicked()
{
    recvObj.setChannels(ui->spinBoxChannels->value());
    ui->messagesPanel->appendPlainText("Number of channels: "
                                       +ui->spinBoxChannels->text());
    ui->createButton_2->setEnabled(false);
    ui->spinBoxChannels->setEnabled(false);
}

void Window::on_comboBox_currentIndexChanged(int index)
{
    if (index == 0) { //Loudspeakers;
        ui->createButton_2->setEnabled(true);
        ui->spinBoxChannels->setEnabled(true);
        ui->fileSampleRate->setEnabled(false);
        ui->fileName->setEnabled(false);
        ui->createButton->setEnabled(false);
        ui->activateButton->setEnabled(true);
    }
    else if (index == 1) { //File
        ui->spinBoxChannels->setEnabled(true);
        ui->fileSampleRate->setEnabled(true);
        ui->fileName->setEnabled(true);
        ui->createButton->setEnabled(true);
        ui->createButton_2->setEnabled(false);
        ui->activateButton->setEnabled(true);
    }
    else if (index == 2) { //Both
        ui->spinBoxChannels->setEnabled(true);
        ui->fileSampleRate->setEnabled(true);
        ui->fileName->setEnabled(true);
        ui->createButton->setEnabled(true);
        ui->createButton_2->setEnabled(false);
        ui->activateButton->setEnabled(true);
    }
    else if (index == 3) { //Nothing
        ui->spinBoxChannels->setEnabled(false);
        ui->fileSampleRate->setEnabled(false);
        ui->fileName->setEnabled(false);
        ui->createButton->setEnabled(false);
        ui->createButton_2->setEnabled(false);
        ui->activateButton->setEnabled(false);
    }

}
