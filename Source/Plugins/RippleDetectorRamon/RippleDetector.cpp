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
 
//easy way to do a envelope, do a findpeaks for the entire buff (like the phase detector) and then create a polynomial regression in the points. 
//a good idea would be to plot this data, maybe create a new channel and put the envelope in it with write pointer.

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <algorithm>
#include "RippleDetector.h"
#include "RippleDetectorEditor.h"
//Libraries

using namespace std;
//Used to print messages

RippleDetector::RippleDetector()
    : GenericProcessor("Ripple Detector"), activeModule(-1)
{
//	cout << "-----------------------------Ripple Constructor------------------------------" << endl;
}
// 1- Constructor

RippleDetector::~RippleDetector()
{
//	cout << "-----------------------------Rippĺe Destructor-------------------------------" << endl;
}
// 1- Destructor

AudioProcessorEditor* RippleDetector::createEditor()
{
//	cout << "-----------------------------Create Editor-----------------------------------" << endl;
    editor = new RippleDetectorEditor(this, true);

    cout << "Creating Editor" << endl;

    RippleDetectorEditor* ed = (RippleDetectorEditor*) getEditor();
    ed->setDefaults (defaultTime, defaultAmplitude);

    return editor;
}
// 1- Call Editor (and the function setDefault to set initial Time and Amplitude)
// 2- Como retirar o SetDefault dessa função?

void RippleDetector::addModule()
{
//	cout << "-----------------------------Add Module--------------------------------------" << endl;
    DetectorModule m = DetectorModule();
    m.inputChan = -1;
    m.outputChan = -1;
    m.gateChan = -1;
    m.isActive = true;
    m.lastSample = 0.0;
    m.samplesSinceTrigger = 5000;
    m.wasTriggered = false;
    m.MED = 0.00;
    m.STD = 0.00;
    m.AvgCount = 0;
    m.flag = 0;
    m.tReft = 0.0;
    m.count = 0;
//---New---    

//	int arrLen = 60000/4;
//	m.BLThreshold[arrLen];

//---
    modules.add(m);

}
// 1- Initialize variables
// 2- Could I do it inside the other functions?

void RippleDetector::setActiveModule(int i)
{
	cout << "-----------------------------Set Active Module-------------------------------" << endl;
    activeModule = i;
}                                                                                                             
//It is called, always, in "comboBoxChanged" function at Editor

void RippleDetector::setParameter(int parameterIndex, float newValue)
{
    DetectorModule& module = modules.getReference(activeModule);
/*
    if (parameterIndex == 1) // module type
    {

        int val = (int) newValue;

    }
    else */if (parameterIndex == 2)   // inputChan
    {
        module.inputChan = (int) newValue;
    }
    else if (parameterIndex == 3)   // outputChan
    {
        module.outputChan = (int) newValue;
    }
    else if (parameterIndex == 4)   // gateChan
    {
        module.gateChan = (int) newValue;
        if (module.gateChan < 0)
        {
            module.isActive = true; 
        }
        else
        {
            module.isActive = false;
        }
    }

    if (parameterIndex < 2) // change filter settings
    {
        if (newValue <= 0.01 || newValue >= 10000.0f)
            return;

        if (parameterIndex == 0)
        {
            times.set (currentChannel,newValue);
        }
        else if (parameterIndex == 1)
        {
            amplitudes.set (currentChannel,newValue);
        }

        setFilterParameters (times[currentChannel],
                             amplitudes[currentChannel],
                             currentChannel);

//        editor->updateParameterButtons (parameterIndex);
    }

}
//It is called, always, in "comboBoxChanged" function at Editor

void RippleDetector::updateSettings()
{
//	cout << "-----------------------------Update Settings---------------------------------" << endl;
    int numInputs = getNumInputs();

    if (numInputs < 1024)
    {
        Array<double> oldtimes;
        Array<double> oldamplitudes;
        oldtimes = times;
        oldamplitudes = amplitudes;

        times.clear();
        amplitudes.clear();

        for (int n = 0; n < getNumInputs(); ++n)
        {
            float newTime  = 0.f;
            float newAmplitude = 0.f;

            if (oldtimes.size() > n)
            {
                newTime  = oldtimes[n];
                newAmplitude = oldamplitudes[n];
            }
            else
            {
                newTime  = defaultTime;
                newAmplitude = defaultAmplitude;
            }

            times.add  (newTime);
            amplitudes.add (newAmplitude);

            setFilterParameters (newTime, newAmplitude, n);
        }
    }
}
//Update the nº of channels. (When the signal chain is changed and when a signal is loaded) _ called before _

bool RippleDetector::enable()
{
	cout << "-----------------------------Enable------------------------------------------" << endl;
    return true;
}
//Called when the "play" button is pressed

void RippleDetector::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
	cout << "-----------------------------Handle Event------------------------------------" << endl;
    if (eventType == TTL)
    {
        const uint8* dataptr = event.getRawData();

        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);

        for (int i = 0; i < modules.size(); i++)
        {
            DetectorModule& module = modules.getReference(i);

            if (module.gateChan == eventChannel)
            {
                if (eventId)
                    module.isActive = true;
                else
                    module.isActive = false;
            }
        }

    }

}
//Called when time is running after the "play/record" button is pressed

void RippleDetector::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events) //This is the core of the code, is the script that will run when every buffer comes
{
	cout << "-----------------------------Process-----------------------------------------" << endl;
    Time time; //I'm using a library to count time
    checkForEvents(events);
    // loop through the modules
    for (int i = 0; i < modules.size(); i++)
    {
      DetectorModule& module = modules.getReference(i);

        double t;    
        double t3;
        double RefratTime;
        double ThresholdAmplitude = 2.00;
        double ThresholdTime = 0.020;/*<- THIS IS THE TIME THRESHOLD*/ //Divide it for 1000 when it comes from the user input, for now

        ThresholdTime = TimeT/1000;
        ThresholdAmplitude = amplitude;
        
            t = double(time.getHighResolutionTicks()) / double(time.getHighResolutionTicksPerSecond());//Starting to count time for the script here
            double arrSized = round(getNumSamples(module.inputChan)/4);//the following 3 lines are to create an array of specific size for saving the RMS from buffer

            int arrSize = (int) arrSized;
            float RMS[arrSize];
            for (int index = 0; index < arrSize; index++) //here the RMS is calculated
            {
                RMS[index] = sqrt( (
                   pow(buffer.getSample(module.inputChan,(index*4)),2) +
                   pow(buffer.getSample(module.inputChan,(index*4)+1),2) +
                   pow(buffer.getSample(module.inputChan,(index*4)+2),2) +
                   pow(buffer.getSample(module.inputChan,(index*4)+3),2)
                )/4 );
            }
//---New---            

//			int size = 60000/4;
//		    if (module.AvgCount >= size)
//		    {
//		    	for (int i = arrSize; i < size; i++)
//		       	{
//		        	module.BLThreshold[i-arrSize] = module.BLThreshold[i];
//           	}
//		    }//rotate the baseline threshold to input more data, as a circular buffer

//---
            for (int pac = 0; pac < arrSize; pac++)
            {
                if (module.AvgCount < 60000/4/*size*/) //Using the RMS value in the first 2 s as baseline to build a threshold of detection
                {
//---New---
//					int idx = module.AvgCount;
//					module.BLThreshold[idx] = RMS[pac];
//
//---
                    module.AvgCount++; // all the values that must be saved from one buffer to other has to be save in another function outside the process (this function), when I need to save values to reuse in the next buffer I use the structure module from addModule function
///*
                    float var = RMS[pac];
                    float delta = var - module.MED;
                    module.MED = module.MED + (delta/module.AvgCount); //calculates average for threshold
                    module.STD = module.STD + delta*(var-module.MED); // calculates standard deviation for threhsold
//If New is on, put it like a comment
//*/
                }
                else
                {
					break;
//                    module.BLThreshold[pac+(module.AvgCount-arrSize)] = RMS[pac];
                }
            }
//---New---
//			float sum;
//		    for (int i = 0; i< (size);i++)
//		    {
//				sum += module.BLThreshold[i];
//            }
//		    module.MED = ((float)sum)/size;

//		    float sum2;
//		    for (int i = 0; i< (size);i++)
//		    {
//				sum2 += pow(module.BLThreshold[i]-module.MED,2);
//            }
//		    module.STD = sqrt(sum2/(size-1));

//		    double threshold = module.MED + ThresholdAmplitude*(module.STD); //building the threshold from average + n*standard deviation                
//---
            for (int i = 0; i < arrSize; i++)
            {
            
                const float sample = RMS[i];
/*If New is on, put it like a comment */                double threshold = module.MED + ThresholdAmplitude*sqrt(module.STD/(module.AvgCount*4)); //building the threshold from average + n*standard deviation                
                
                if ( sample >=  threshold & RefratTime > .0 ) //counting how many points are above the threshold and if has been 2 s after the last event (refractory period)
                {                
                  module.count++;
                }
		              else if(sample < threshold & i == 0)//protect from acumulation
		            {
		                 module.count = 0;
 	             	}               
                if (module.flag == 1) //if it had a detector activation, starts to recalculate refrat time
                {
                    t3 = ( double(time.getHighResolutionTicks()) / double(time.getHighResolutionTicksPerSecond()) )- module.tReft;//calculating refractory time
                    RefratTime = t3;
                }
                else //if module.flag == 0, then the script is just starting and refractory time is adjusted for a a value that works
                {
                    RefratTime = 1;
                }

				if(module.isActive && module.outputChan >= 0 && module.inputChan >= 0 && buffer.getNumChannels())
				{
                if (module.count >= round(ThresholdTime*30000/4) & RefratTime > .0 ) //this is the time threshold, buffer RMS amplitude must be higher than threshold for a certain period of time, the second term is the Refractory period for the detection, so it hasn't a burst of activation after reaching both thresholds
                {
			//below from here starts the activation for sending the TTL to the actuator
			
                        module.flag = 1;
			            module.count = 0;
                        module.tReft = double(time.getHighResolutionTicks()) / double(time.getHighResolutionTicksPerSecond());
    
                        addEvent(events, TTL, i, 1, module.outputChan);
                        module.samplesSinceTrigger = 0;

                        module.wasTriggered = true;
                        
                }
                module.lastSample = sample;

                if (module.wasTriggered)
                {
                    if (module.samplesSinceTrigger > 1000)
                    {
                        addEvent(events, TTL, i, 0, module.outputChan);
                        module.wasTriggered = false;
                    }
                    else
                    {
                        module.samplesSinceTrigger++;
                    }
                }
				}
        }

    }

    for (int n = 0; n < getNumOutputs(); ++n)
    {
            float* ptr = buffer.getWritePointer (n);
    }

}
//Called when time is running after the "play/record" button is pressed

void RippleDetector::setFilterParameters (double a, double b, int chan)
{
//	cout << "-----------------------------Set Filter Parameters---------------------------" << endl;
    if (channels.size() - 1 < chan)
        return;

    TimeT = a;
    amplitude = b;
}
//Update settings when signal chain is changed and when time/amplitude changes _ called each time a channel is added/subtracted _

