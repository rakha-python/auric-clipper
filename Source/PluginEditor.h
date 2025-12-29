#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "AuricLookAndFeel.h"

class AuricClipperAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AuricClipperAudioProcessorEditor(AuricClipperAudioProcessor&);
    ~AuricClipperAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;

private:
    AuricClipperAudioProcessor& audioProcessor;
    AuricLookAndFeel auricLookAndFeel;

    // Main drive knob
    juce::Slider driveKnob;
    juce::Label driveLabel;

    // Small knobs
    juce::Slider preKnob, satClipKnob, trimKnob, mixKnob;
    juce::Label preLabel, satClipLabel, trimLabel, mixLabel;

    // Output ceiling slider
    juce::Slider ceilingSlider;
    juce::Label ceilingLabel;

    // 2X toggle
    juce::ToggleButton oversampleButton;
    juce::Label oversampleLabel;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> satClipAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trimAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ceilingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> oversampleAttachment;

    void setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& text);
    void drawBrushedMetal(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawCornerScrews(juce::Graphics& g);
    void drawStatusLed (juce::Graphics& g, juce::Rectangle<float> ledBounds, bool isOn);
    juce::Image loadReferenceOverlay() const;

    bool showReferenceOverlay = false;
    juce::Image referenceOverlay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuricClipperAudioProcessorEditor)
};
