#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "AuricLookAndFeel.h"

//==============================================================================
class AuricClipperAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    explicit AuricClipperAudioProcessorEditor (AuricClipperAudioProcessor&);
    ~AuricClipperAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;

private:
    //==============================================================================
    void timerCallback() override;

    void setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& text);

    // LED besar kiri bawah (indikator oversample)
    void drawStatusLed (juce::Graphics& g, juce::Rectangle<float> ledBounds, bool isOn);

    // Reference overlay loader (image.png)
    juce::Image loadReferenceOverlay() const;

    //==============================================================================
    AuricClipperAudioProcessor& audioProcessor;

    AuricLookAndFeel auricLookAndFeel;

    // Knobs
    juce::Slider driveKnob;
    juce::Slider preKnob;
    juce::Slider satClipKnob;
    juce::Slider trimKnob;
    juce::Slider mixKnob;

    // Labels (dibuat tapi di-hide karena label digambar manual di paint())
    juce::Label driveLabel;
    juce::Label preLabel;
    juce::Label satClipLabel;
    juce::Label trimLabel;
    juce::Label mixLabel;

    // Output ceiling slider
    juce::Slider ceilingSlider;

    // Oversample toggle (click-area saja; LED digambar manual di paint())
    juce::ToggleButton oversampleButton;

    //==============================================================================
    // APVTS Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> satClipAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trimAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ceilingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> oversampleAttachment;

    //==============================================================================
    // Overlay
    bool showReferenceOverlay { false };
    juce::Image referenceOverlay;

    // LED state cache
    bool lastLedState { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AuricClipperAudioProcessorEditor)
};
