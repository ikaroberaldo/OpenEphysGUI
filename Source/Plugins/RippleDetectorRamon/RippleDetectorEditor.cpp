/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "RippleDetectorEditor.h"
#include "RippleDetector.h"
//Libraries of OpenEphys

#include <stdio.h>
#include <cmath>
//C++ libraries

using namespace std;
//Used to print messages

RippleDetectorEditor::RippleDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors), previousChannelCount(-1)
// 2- What are these parameters?
{
//	cout << "-----------------------------Editor Constructor------------------------------" << endl;
    desiredWidth = 300;
//Width of the module's box

    RippleDetector* sd = (RippleDetector*) getProcessor();
//Initializes what it need from RippleDetector.cpp

    int detectorNumber = interfaces.size()+1;

    RippleInterface* di = new RippleInterface(sd, backgroundColours[detectorNumber%5], detectorNumber-1);
    di->setBounds(90,40,190,80);
//Call RippleInterface

    addAndMakeVisible(di);

    interfaces.add(di);

    backgroundColours.add(Colours::green);
    backgroundColours.add(Colours::red);
    backgroundColours.add(Colours::orange);
    backgroundColours.add(Colours::magenta);
    backgroundColours.add(Colours::blue);
// 2- What does the backgroundcolour means?

    lastTimeString = " ";
    lastAmplitudeString = " ";
//Initialization of the strings

    amplitudeLabel = new Label("Amplitude label", "Amplitude (sd):");
    amplitudeLabel->setBounds(10,75,80,20);
    amplitudeLabel->setFont(Font("Small Text", 10, Font::plain));
    amplitudeLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(amplitudeLabel);
//Create Amplitude's label

    timeLabel = new Label("Time label", "Time (ms):");
    timeLabel->setBounds(10,35,80,20);
    timeLabel->setFont(Font("Small Text", 10, Font::plain));
    timeLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(timeLabel);
//Create Time's label

    amplitudeValue = new Label("Amplitude label", lastAmplitudeString);
    amplitudeValue->setBounds(15,92,60,18);
    amplitudeValue->setFont(Font("Default", 15, Font::plain));
    amplitudeValue->setColour(Label::textColourId, Colours::white);
    amplitudeValue->setColour(Label::backgroundColourId, Colours::grey);
    amplitudeValue->setEditable(true);
    amplitudeValue->addListener(this);
    amplitudeValue->setTooltip("Set the amplitude for the selected channels");
    addAndMakeVisible(amplitudeValue);
//Create Amplitude's message

    timeValue = new Label("Time value", lastTimeString);
    timeValue->setBounds(15,52,60,18);
    timeValue->setFont(Font("Default", 15, Font::plain));
    timeValue->setColour(Label::textColourId, Colours::white);
    timeValue->setColour(Label::backgroundColourId, Colours::grey);
    timeValue->setEditable(true);
    timeValue->addListener(this);
    timeValue->setTooltip("Set the time for the selected channels");
    addAndMakeVisible(timeValue);
//Create Time's message

}
// 1- Editor Constructor
// 2- The questions are inside the function

RippleDetectorEditor::~RippleDetectorEditor()
{
//	cout << "-----------------------------Editor Destructor-------------------------------" << endl;
}
// 1- Editor Destructor

void RippleDetectorEditor::updateSettings()
{
//	cout << "-----------------------------Update Settings2--------------------------------" << endl;
    if (getProcessor()->getNumInputs() != previousChannelCount)
    {

        for (int i = 0; i < interfaces.size(); i++)
        {
            interfaces[i]->updateChannels(getProcessor()->getNumInputs());
        }
    }

    previousChannelCount = getProcessor()->getNumInputs();

}
//Update the nº of channels. (When the signal chain is changed and when a signal is loaded) _ called after _

void RippleDetectorEditor::saveCustomParameters(XmlElement* xml)
{
	cout << "-----------------------------Save Parameters---------------------------------" << endl;
    xml->setAttribute("Type", "RippleDetectorEditor");

    for (int i = 0; i < interfaces.size(); i++)
    {
        XmlElement* d = xml->createNewChildElement("DETECTOR");
        d->setAttribute("INPUT",interfaces[i]->getInputChan());
        d->setAttribute("GATE",interfaces[i]->getGateChan());
        d->setAttribute("OUTPUT",interfaces[i]->getOutputChan());
    }
    
    lastAmplitudeString = amplitudeValue->getText();
    lastTimeString = timeValue->getText();

    XmlElement* textLabelValues = xml->createNewChildElement("VALUES");
    textLabelValues->setAttribute("Amplitude",lastAmplitudeString);
    textLabelValues->setAttribute("Time",lastTimeString);
}
//Called whe the "record" button is pressed

void RippleDetectorEditor::loadCustomParameters(XmlElement* xml)
{
	cout << "-----------------------------Load Parameters---------------------------------" << endl;
    int i = 0;

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("DETECTOR"))
        {
            
            interfaces[i]->setInputChan(xmlNode->getIntAttribute("INPUT"));
            interfaces[i]->setGateChan(xmlNode->getIntAttribute("GATE"));
            interfaces[i]->setOutputChan(xmlNode->getIntAttribute("OUTPUT"));

            i++;
        }

        else if (xmlNode->hasTagName("VALUES"))
        {
            amplitudeValue->setText(xmlNode->getStringAttribute("Amplitude"),dontSendNotification);
            timeValue->setText(xmlNode->getStringAttribute("Time"),dontSendNotification);
        }
    }
}
//Called when the value of time or amplitude is changed

// ===================================================================

RippleInterface::RippleInterface(RippleDetector* sd, Colour c, int id) :
    backgroundColour(c), idNum(id), processor(sd)
//What are these parameters?
{
//	cout << "-----------------------------Interface Constructor---------------------------" << endl;
    font = Font("Small Text", 10, Font::plain);

    const double PI = 3.14159265;

    sineWave.startNewSubPath(5,35);

    cout << "Creating sine wave" << endl;

    for (double i = 0; i < 2*PI; i += PI/80)
    {
        
        if (i > 1.5*PI/3 || i < 1.7*PI/3)
        {
            sineWave.lineTo(i*12+5.0f, -sin(i*9)*sin(i*0.5)*30 + 35);
        }
        else
        {
            sineWave.lineTo(i*12 + 5.0f, -sin(i*9)*sin(i*0.5)*10 + 35);
        }
    }
//Creates the sinewave
    cout << "Creating combo boxes" << endl;


    inputSelector = new ComboBox();
    inputSelector->setBounds(140,5,50,20);
    inputSelector->addItem("-",1);
    inputSelector->setSelectedId(1);
    inputSelector->addListener(this);
    addAndMakeVisible(inputSelector);

    gateSelector = new ComboBox();
    gateSelector->setBounds(140,30,50,20);
    gateSelector->addItem("-",1);
    gateSelector->addListener(this);

    cout << "Populating combo boxes" << endl;


    for (int i = 1; i < 9; i++)
    {
        gateSelector->addItem(String(i),i+1);
    }

    gateSelector->setSelectedId(1);
    addAndMakeVisible(gateSelector);

    outputSelector = new ComboBox();
    outputSelector->setBounds(140,55,50,20);
    outputSelector->addItem("-",1);
    outputSelector->addListener(this);

    for (int i = 1; i < 9; i++)
    {
        outputSelector->addItem(String(i),i+1);
    }
    outputSelector->setSelectedId(1);
    addAndMakeVisible(outputSelector);
//Creates the input/output comboBoxes

    cout << "Updating channels" << endl;

    updateChannels(processor->getNumInputs());

    cout << "Updating processor" << endl;

    processor->addModule();

}
// 1- Build the visual interface of the sine wave and input/gate/output boxes

RippleInterface::~RippleInterface()
{
//	cout << "-----------------------------Interface Destructor----------------------------" << endl;
}
// 1- Destructor of the visual interface of the sine wave and input/gate/output boxes

void RippleInterface::comboBoxChanged(ComboBox* c)
{
	cout << "-----------------------------Combo Box Changed-------------------------------" << endl;
    processor->setActiveModule(idNum);

    int parameterIndex;

    if (c == inputSelector)
    {
        parameterIndex = 2;
    }
    else if (c == outputSelector)
    {
        parameterIndex = 3;
    }
    else if (c == gateSelector)
    {
        parameterIndex = 4;
    }


    processor->setParameter(parameterIndex, (float) c->getSelectedId() - 2);

}
//Adjust the numbers in the input/gate/output boxes

void RippleInterface::updateChannels(int numChannels)
{
//	cout << "-----------------------------Update Channels---------------------------------" << endl;
    inputSelector->clear();

    inputSelector->addItem("-", 1);

    if (numChannels > 2048) // channel count hasn't been updated yet
    {
        return;
    }

    for (int i = 0; i < numChannels; i++)
    {
        inputSelector->addItem(String(i+1), i+2);

    }

    inputSelector->setSelectedId(1, dontSendNotification);

}
// 1- Updates the number of channels the plugin receives
// 2- Which are the differences from the updateSettings?
//    Y 2048?

void RippleInterface::paint(Graphics& g)
{
//	cout << "-----------------------------Paint-------------------------------------------" << endl;
    g.setColour(Colours::blue);
    g.strokePath(sineWave, PathStrokeType(2.0f));

    g.setColour(Colours::darkgrey);
    g.setFont(font);
    g.drawText("INPUT",50,10,85,10,Justification::right, true);
    g.drawText("GATE",50,35,85,10,Justification::right, true);
    g.drawText("OUTPUT",50,60,85,10,Justification::right, true);

}
// 1- Graphic part of the input/gate/output boxes
// 2- Y is it called everytime something changes in the plugin?

void RippleInterface::setInputChan(int chan)
{
	cout << "-----------------------------Set Input Channel-------------------------------" << endl;
    inputSelector->setSelectedId(chan+2);

    processor->setParameter(2, (float) chan);
}
//Called when the value of time or amplitude is changed

void RippleInterface::setOutputChan(int chan)
{
	cout << "-----------------------------Set Output Channel------------------------------" << endl;
    outputSelector->setSelectedId(chan+2);

    processor->setParameter(3, (float) chan);
}
//Called when the value of time or amplitude is changed

void RippleInterface::setGateChan(int chan)
{
    gateSelector->setSelectedId(chan+2);

    processor->setParameter(4, (float) chan);
}
//Called when the value of time or amplitude is changed

int RippleInterface::getInputChan()
{
	cout << "-----------------------------Get Input Channels------------------------------" << endl;
    return inputSelector->getSelectedId()-2;
}
//Called whe the "record" button is pressed

int RippleInterface::getOutputChan()
{
	cout << "-----------------------------Get Output Channels-----------------------------" << endl;
    return outputSelector->getSelectedId()-2;
}
//Called whe the "record" button is pressed

int RippleInterface::getGateChan()
{
    return gateSelector->getSelectedId()-2;
} 
//Called whe the "record" button is pressed

void RippleDetectorEditor::setDefaults(double time, double amplitude)
{
//	cout << "-----------------------------Set Defaults------------------------------------" << endl;

	lastAmplitudeString = String(roundFloatToInt(amplitude));
    lastTimeString = String(roundFloatToInt(time));

    amplitudeValue->setText(lastAmplitudeString, dontSendNotification);
    timeValue->setText(lastTimeString, dontSendNotification);
}
// 1- Set default values of time and amplitude

void RippleDetectorEditor::labelTextChanged(Label* label)
{
//	cout << "-----------------------------Label Text Changed------------------------------" << endl;
    RippleDetector* sd = (RippleDetector*) getProcessor();

    Value val = label->getTextValue();
    double requestedValue = double(val.getValue());

    if (requestedValue < 0.01 || requestedValue > 10000)
    {
        CoreServices::sendStatusMessage("Value out of range.");

        if (label == amplitudeValue)
        {
            label->setText(lastAmplitudeString, dontSendNotification);
            lastAmplitudeString = label->getText();
        }
        else
        {
            label->setText(lastTimeString, dontSendNotification);
            lastTimeString = label->getText();
        }

        return;
    }

    Array<int> chans = getActiveChannels();


    for (int n = 0; n < chans.size(); n++)
    {

        if (label == amplitudeValue)
        {

                sd->setCurrentChannel(chans[n]);

                sd->setParameter(1, requestedValue);


            lastAmplitudeString = label->getText();

        }
        else
        {

                sd->setCurrentChannel(chans[n]);
                sd->setParameter(0, requestedValue);

            lastTimeString = label->getText();

        }

    }

}
// 1- Change Time/Amplitude's values
// 2- Ask Cleiton what will be the limits for the time/amplitude values

