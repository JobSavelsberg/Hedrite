/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Hedrite.h"

//==============================================================================
/**
*/
class HedriteAudioProcessorEditor  : public juce::AudioProcessorEditor
{
    Hedrite hedrite;
public:
    HedriteAudioProcessorEditor (HedriteAudioProcessor&);
    ~HedriteAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    HedriteAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HedriteAudioProcessorEditor)
};
