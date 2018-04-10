/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2015 Open Ephys

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

#ifndef __RIPPLEDETECTOR_H__
#define __RIPPLEDETECTOR_H__

#ifdef _WIN32
#include <Windows.h>
#endif

#include <ProcessorHeaders.h>

#define NUM_INTERVALS 5

class RippleDetector : public GenericProcessor

{
public:

    RippleDetector();
    ~RippleDetector();

    void process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void setParameter(int parameterIndex, float newValue);
    
    AudioProcessorEditor* createEditor();

    bool hasEditor() const
    {
        return false;
    }

    bool enable();

    void updateSettings();

    void addModule();
    void setActiveModule(int);
    
    bool hasEditor2() const { return true; }


private:

    struct DetectorModule
    {

        int inputChan;
        int gateChan;
        int outputChan;
        bool isActive;
        int samplesSinceTrigger;
        bool wasTriggered;
        int cntLimits;
        int SR;        
        float MED;
        float STD;
        int AvgCount;
        int flag;
        double tReft;
		int count;
		float lastSample;

//		int BLThreshold[];
    };
        
    Array<DetectorModule> modules;

    int activeModule;
    
    void handleEvent(int eventType, MidiMessage& event, int sampleNum);

    void setFilterParameters (double, double, int);

    Array<double> times;
    Array<double> amplitudes;
    double TimeT;
    double amplitude;

    double defaultTime = 20.0f;
    double defaultAmplitude = 2.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RippleDetector);

};

#endif  // __RippleDETECTOR_H__

