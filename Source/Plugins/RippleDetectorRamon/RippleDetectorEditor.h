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

#ifndef __RippleDETECTOREDITOR_H__
#define __RippleDETECTOREDITOR_H__

#include <EditorHeaders.h>

class RippleInterface;
class RippleDetector;

class RippleDetectorEditor : public GenericEditor,
    public Label::Listener
{
public:
    RippleDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~RippleDetectorEditor();

    void updateSettings();

    void saveCustomParameters(XmlElement* xml);
    void loadCustomParameters(XmlElement* xml);
    
    void labelTextChanged(Label* label);

    void setDefaults(double time, double amplitude);

private:

    OwnedArray<RippleInterface> interfaces;

    int previousChannelCount;

    Array<Colour> backgroundColours;
    
    String lastAmplitudeString;
    String lastTimeString;

    ScopedPointer<Label> amplitudeLabel;
    ScopedPointer<Label> timeLabel;

    ScopedPointer<Label> amplitudeValue;
    ScopedPointer<Label> timeValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RippleDetectorEditor);

};

class RippleInterface : public Component,
    public ComboBox::Listener
{
public:
    RippleInterface(RippleDetector*, Colour, int);
    ~RippleInterface();

    void paint(Graphics& g);

    void comboBoxChanged(ComboBox*);

    void updateChannels(int);

    void setInputChan(int);
    void setOutputChan(int);
    void setGateChan(int);

    int getInputChan();
    int getOutputChan();
    int getGateChan();

private:

    Colour backgroundColour;

    Path sineWave;
    Font font;

    int idNum;

    RippleDetector* processor;

    ScopedPointer<ComboBox> inputSelector;
    ScopedPointer<ComboBox> gateSelector;
    ScopedPointer<ComboBox> outputSelector;

};

#endif

