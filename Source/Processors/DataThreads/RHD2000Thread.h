/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#ifndef __RHD2000THREAD_H_2C4CBD67__
#define __RHD2000THREAD_H_2C4CBD67__


#include "../../../JuceLibraryCode/JuceHeader.h"

#include <stdio.h>
#include <string.h>

#include "rhythm-api/rhd2000evalboard.h"
#include "rhythm-api/rhd2000registers.h"
#include "rhythm-api/rhd2000datablock.h"
#include "rhythm-api/okFrontPanelDLL.h"

#include "DataThread.h"
#include "../GenericProcessor/GenericProcessor.h"

#define MAX_NUM_DATA_STREAMS 8

class SourceNode;

/**

  Communicates with the RHD2000 Evaluation Board from Intan Technologies

  @see DataThread, SourceNode

*/

class RHD2000Thread : public DataThread, public Timer

{
public:
    RHD2000Thread(SourceNode* sn);
    ~RHD2000Thread();

    bool foundInputSource();
    int getNumChannels();
    float getSampleRate();
    float getBitVolts(int chan);
    float getADCBitVolts(int chan);

    bool isHeadstageEnabled(int hsNum);

    bool enableHeadstage(int hsNum, bool enabled);
    void setCableLength(int hsNum, float length);
    void setNumChannels(int hsNum, int nChannels);

    void setSampleRate(int index, bool temporary = false);

    double setUpperBandwidth(double upper); // set desired BW, returns actual BW
    double setLowerBandwidth(double lower);

    double setDspCutoffFreq(double freq);
    double getDspCutoffFreq();
    
    void setDSPOffset(bool state);

    int setNoiseSlicerLevel(int level);
    void runImpedanceTest(Array<int> &stream, Array<int> &channel, Array<float> &magnitude, Array<float> &phase);
    void setFastTTLSettle(bool state, int channel);
    void setTTLoutputMode(bool state);
    void setDAChpf(float cutoff, bool enabled);

    void scanPorts();
    float updateImpedanceFrequency(float desiredImpedanceFreq, bool &impedanceFreqValid);
    int loadAmplifierData(queue<Rhd2000DataBlock> &dataQueue,
                                       int numBlocks, int numDataStreams);
    void measureComplexAmplitude(std::vector<std::vector<std::vector<double>>> &measuredMagnitude,
                                            std::vector<std::vector<std::vector<double>>> &measuredPhase,
                                              int capIndex, int stream, int chipChannel, int numBlocks,
                                              double sampleRate, double frequency, int numPeriods);
    void amplitudeOfFreqComponent(double &realComponent, double &imagComponent,
                                               const std::vector<double> &data, int startIndex,
                                               int endIndex, double sampleRate, double frequency);
    int getNumEventChannels();
    int getNumADCchannels();

    void assignAudioOut(int dacChannel, int dataChannel);
    void enableAdcs(bool);

    bool isAcquisitionActive();
    
    virtual int modifyChannelGain(channelType t, int str, int ch, float gain);
    virtual int modifyChannelName(channelType t, int str, int k, String newName);
    virtual void getChannelsInfo(StringArray &Names, Array<channelType> &type, Array<int> &stream, Array<int> &originalChannelNumber, Array<float> &gains);
    virtual void getEventChannelNames(StringArray &Names);
    void updateChannelNames();
    Array<int> getDACchannels();
    void setDACchannel(int dacOutput, int stream, int channel);
    void setDACthreshold(int dacOutput, float threshold);
    void setDefaultNamingScheme(int scheme);

	String getChannelName(channelType t, int str, int ch);

private:
    void setDefaultChannelNamesAndType();
    bool channelModified(channelType t, int str, int k, String &oldName, float &oldGain, int &index);

    ScopedPointer<Rhd2000EvalBoard> evalBoard;
    Rhd2000Registers chipRegisters;
    Rhd2000DataBlock* dataBlock;

    std::vector<std::vector<std::vector<double>>> amplifierPreFilter;
    Array<int> numChannelsPerDataStream;
    void factorOutParallelCapacitance(double &impedanceMagnitude, double &impedancePhase,
                                              double frequency, double parasiticCapacitance);
    void empiricalResistanceCorrection(double &impedanceMagnitude, double &impedancePhase,
                                               double boardSampleRate);
    int numChannels;
    bool deviceFound;

    float thisSample[256];
    float auxBuffer[256]; // aux inputs are only sampled every 4th sample, so use this to buffer the samples so they can be handles just like the regular neural channels later

    int blockSize;

    bool isTransmitting;

    bool dacOutputShouldChange;
    bool acquireAdcChannels;
    bool acquireAuxChannels;

    bool fastSettleEnabled;
    bool fastTTLSettleEnabled;
    bool fastSettleTTLChannel;
    bool ttlMode;
    bool desiredDAChpfState;
    double desiredDAChpf;

    bool dspEnabled;
    double actualDspCutoffFreq, desiredDspCutoffFreq;
    double actualUpperBandwidth, desiredUpperBandwidth;
    double actualLowerBandwidth, desiredLowerBandwidth;
    int actualNoiseSlicerLevel, desiredNoiseSlicerLevel;
    double boardSampleRate;
    int savedSampleRateIndex;

    String libraryFilePath;

    void timerCallback();

    bool startAcquisition();
    bool stopAcquisition();

    bool openBoard(String pathToLibrary);
    bool uploadBitfile(String pathToBitfile);
    void initializeBoard();

    void updateRegisters();

    int deviceId(Rhd2000DataBlock* dataBlock, int stream, int &register59Value);

    bool updateBuffer();

    double cableLengthPortA, cableLengthPortB, cableLengthPortC, cableLengthPortD;

    int audioOutputL, audioOutputR;
    int *dacChannels, *dacStream;
    float *dacThresholds;
    bool *dacChannelsToUpdate;
    Array<int> chipId;

    // used for data stream names...
    int numberingScheme ;
    StringArray Names, oldNames;
    Array<channelType> type, oldType;
    Array<float> gains, oldGains;
    Array<int> stream, oldStream;
    Array<bool> modifiedName, oldModifiedName;
    Array<int> originalChannelNumber, oldChannelNumber;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RHD2000Thread);
};

#endif  // __RHD2000THREAD_H_2C4CBD67__
