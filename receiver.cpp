#include "receiver.h"

int Receiver::getUnderflowCounter() const
{
    return underflowCounter;
}

int Receiver::getLostNetworkPackages() const
{
    return lostNetworkPackages;
}

int Receiver::getOverflowCounter() const
{
    return overflowCounter;
}

void Receiver::setLostNetworkPackages(int value)
{
    lostNetworkPackages = value;
}

void Receiver::setOverflowCounter(int value)
{
    overflowCounter = value;
}

int Receiver::getOutputMode() const
{
    return outputMode;
}

void Receiver::setOutputMode(int value)
{
    outputMode = value;
}

Receiver::Receiver() {
    srPortNumber = 57160;

    srISAddress.sin_family = AF_INET;
    srISAddress.sin_port = htons(srPortNumber);
    srISAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    outputMode = 0;
    underflowCounter = 0;
    overflowCounter = 0;
}

int Receiver::getSrPortNumber() const
{
    return srPortNumber;
}

void Receiver::setSrPortNumber(int value)
{
    srPortNumber = value;
}

int aux_callback_method(jack_nframes_t nframes, void* p) {
    return static_cast<Receiver*>(p)->callback_method(nframes);
}

int Receiver::set_callback_method() {
    jack_set_process_callback(clientfd, aux_callback_method, this);
    return 0;
}

int Receiver::callback_method(jack_nframes_t nframes) {
    //Nframes = Frames/Period = = Buffer de JACK = 1024
    if(nframes >= jackBufferFrames) {
        cout<< "Fatal error. Not enough space!. "
               "JackRF64 buffer frames: "<<jackBufferFrames<<
               " and Frames: "<<nframes<<"\n";
        exit(1);
    }

    //Conversion del RingBuffer a Jack Ports
    //Le dice los punteros out que apunten al mismo sitio que los Jack ports.
    int i, j;
    float *out[channels];
    float localFrame [channels*nframes];

    for(i = 0; i < channels; i++) {
        out[i] = (float *) jack_port_get_buffer(jackPort[i], nframes);
    }


    //Comprueba si tiene informacion para el Jack
    int nsamples = nframes * channels;
    int nbytes = nsamples * sizeof(float);
    int bytes_available = (int) jack_ringbuffer_read_space(ringBuffer);
    //Si no tiene suficiente informacion, reseta los punteros out.
    if(nbytes > bytes_available) {
        //cout<<"Underflow! "<<underflowCounter<<endl;
        underflowCounter++;
        for(i = 0; i < nframes; i++) {
            for(j = 0; j < channels; j++) {
                out[j][i] = (float) 0.0;
            }
        }

    }
    //Pero si tiene suficiente informacion se la da a Jack ports, mediante
    //los punteros out.
    else {
        jack_ringbuffer_read_exactly(nbytes);
        for(i = 0; i < nframes; i++) {
            for(j = 0; j < channels; j++) {
                if (outputMode == 0) {
                    out[j][i] = (float) jackBuffer[(i * channels) + j];
                }
                else if(outputMode == 1){
                    localFrame[(i * channels) + j] =
                            jackBuffer[(i * channels) + j];
                }
                else if (outputMode == 2) {
                    out[j][i] = (float) jackBuffer[(i * channels) + j];
                    localFrame[(i * channels) + j] = out[j][i];
                }
            }

        }
        if (outputMode == 2 || outputMode == 3)
            sndfd.write(localFrame, channels*nframes);
    }
    return 0;
}


void Receiver::create_sr_socket_connection() {
    //int socket(int domain, int type, int protocol)
    srSocketFD = socket(AF_INET, SOCK_DGRAM, 0);
//    if(srSocketFD < 0) {
//        cout<<"Receiver socket creation failed. \n";
//        exit(1);
//    }
}

int Receiver::bind_sr_ISAddress() {
    //int bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
    int code = bind(srSocketFD, (struct sockaddr *)&srISAddress, sizeof(srISAddress));
//    if(code < 0) {
//        cout<<("Receiver could not bind the ISAddress. \n");
//        exit(1);
//    }
//    cout<<"Waiting on port: "<<srPortNumber<<endl;
    return code;
}


void Receiver::receiver_socket_test()
{
    unsigned char buf[2048];
    socklen_t addrlen = sizeof(srISAddress);
    int recvlen;            /* # bytes received */

    /* now loop, receiving data and printing what we received */
    while(1) {
        printf("waiting on port %d\n", srPortNumber);
        recvlen = recvfrom(srSocketFD, buf, 2048,
                           0, (struct sockaddr *)&srISAddress, &addrlen);
        printf("received %d bytes\n", recvlen);
        if (recvlen > 0) {
            buf[recvlen] = 0;
            printf("received message: \"%s\"\n", buf);
        }
    }
    /* never exits */
}

//RECEIVER
// Read data from UDP port and write to ring buffer.
void *Receiver::receiver_thread(void *arg) {
    Receiver *receiver = (Receiver *) arg;
    networkPacket p;                       //Paquete P = Network
    uint32_t nextPacket = 0;

    while(1) {
        //Llama al metodo para recibir 1 paquete de P.
        receiver->packet_recvfrom(&p, receiver->getSrSocketFD());

        //Comprobaciones del indice y numero de canales
        if((p.index != nextPacket) && (nextPacket != 0)) {
            cout<<"Receiver: Out or order package arrival. Expected: "<<nextPacket
               <<"  Arrived: "<<p.index<<endl;

            int auxLNP = p.index - nextPacket;
            receiver->setLostNetworkPackages(receiver->getLostNetworkPackages()
                                             +auxLNP);
        }
        if(p.channels != receiver->getChannels()) {
            cout<<"Receiver: Number of channels mismatch. Expected: "
               <<receiver->getChannels()<<"  Arrived: "<<p.channels<<endl;
            exit(1);
        }

        //Comprueba el espacio que tiene para escribir en el RingBuffer
        int bytes_available = (int) jack_ringbuffer_write_space(receiver->getRingBuffer());
        //Si no hay espacio, avisa.
        if(receiver->getPayloadBytes() > bytes_available) {
            receiver->setOverflowCounter(receiver->getOverflowCounter()+1);
        } else {
            receiver->jack_ringbuffer_write_exactly((char *) p.data,
                                           (size_t) receiver->getPayloadBytes());
        }

        //Actualiza el indice del paquete que debe llegar.
        nextPacket = p.index + 1;
    }
}

void Receiver::finish()
{
    cout<<"Cerrando y liberando. \n";
    close(srSocketFD);
    jack_ringbuffer_free(ringBuffer);
    jack_client_close(clientfd);
    close(comPipe[0]);
    close(comPipe[1]);
    free(jackBuffer);
}



