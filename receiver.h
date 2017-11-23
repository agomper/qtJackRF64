#ifndef RECEIVER_H
#define RECEIVER_H
#include "jackclient.h"
#include "netclient.h"
#include "soundfile.h"

class Receiver : public JackClient, public NetClient, public SoundFile{
    int srPortNumber;                   //Puerto comunicacion.
    int outputMode;
    int underflowCounter;
    int overflowCounter;
    int lostNetworkPackages;
    int indexControl;
    pthread_t netcomThread;
public:
    Receiver();
    void create_sr_socket_connection();
    int bind_sr_ISAddress();
    void receiver_socket_test();
    void finish();
    int getSrPortNumber() const;
    void setSrPortNumber(int value);
    int set_callback_method();
    int callback_method(jack_nframes_t nframes);
    static void *receiver_thread(void *arg);
    int getUnderflowCounter() const;
    int getOverflowCounter() const;
    void setOverflowCounter(int value);
    int getLostNetworkPackages() const;
    void setLostNetworkPackages(int value);
    int getOutputMode() const;
    void setOutputMode(int value);
    int getIndexControl() const;
    void setIndexControl(int value);
    void setUnderflowCounter(int value);
};

#endif // RECEIVER_H

