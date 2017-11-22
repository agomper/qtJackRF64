#include "soundfile.h"

SndfileHandle SoundFile::getSndfd() const
{
    return sndfd;
}

string SoundFile::getSoundFileName() const
{
    return soundFileName;
}

void SoundFile::setSoundFileName(const string &value)
{
    soundFileName = value;
}

int SoundFile::getFileNChannels() const
{
    return fileNChannels;
}

void SoundFile::setFileNChannels(int value)
{
    fileNChannels = value;
}

int SoundFile::getFileSampleRate() const
{
    return fileSampleRate;
}

void SoundFile::setFileSampleRate(int value)
{
    fileSampleRate = value;
}

SoundFile::SoundFile()
{

}

//Last argument: Format | Subtype
bool SoundFile::create_file(int fFormat){
    fileFormat = fFormat;

    sndfd = SndfileHandle (soundFileName, SFM_WRITE, fileFormat,
                           fileNChannels, fileSampleRate) ;

    if (sndfd != NULL) {
        //        cout<<"File created. \n ";
        return true;
    }
    else {
        //        cout<<"File creation error. \n";
        //        printf("%s\n",sf_strerror(NULL));
        //        exit(1);
        return false;
    }
}

void SoundFile::open_file() {
    sndfd = SndfileHandle(soundFileName);

    if (sndfd != NULL) {
        cout<<"File opened \n";
        cout<<"Name: "<< soundFileName  <<endl;
        cout<<"Sample rate: "<< sndfd.samplerate() <<endl;
        //cout<<"Format: "<< sndfd.format() <<endl;
        cout<<"Channels: "<< sndfd.channels() <<endl;
    }
    else {
        cout<<"File creation error. \n";
        printf("%s\n",sf_strerror(NULL));
        exit(1);
    }
}
