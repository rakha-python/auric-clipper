#include "PluginProcessor.h"
#include "PluginEditor.h"

AuricClipperAudioProcessor::AuricClipperAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

AuricClipperAudioProcessor::~AuricClipperAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout AuricClipperAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // DRIVE - main knob (0-100%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"drive", 1}, "Drive", 0.0f, 100.0f, 50.0f));

    // PRE gain (-24dB to +24dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"pre", 1}, "Pre", -24.0f, 24.0f, 0.0f));

    // SAT/CLIP blend (0-100%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"satclip", 1}, "Sat/Clip", 0.0f, 100.0f, 50.0f));

    // TRIM output (-24dB to +24dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"trim", 1}, "Trim", -24.0f, 24.0f, 0.0f));

    // MIX dry/wet (0-100%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mix", 1}, "Mix", 0.0f, 100.0f, 100.0f));

    // OUTPUT CEILING (-12dB to 0dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"ceiling", 1}, "Output Ceiling", -12.0f, 0.0f, 0.0f));

    // 2X Oversampling toggle
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"oversample", 1}, "2X", false));

    return { params.begin(), params.end() };
}

const juce::String AuricClipperAudioProcessor::getName() const { return JucePlugin_Name; }
bool AuricClipperAudioProcessor::acceptsMidi() const { return false; }
bool AuricClipperAudioProcessor::producesMidi() const { return false; }
bool AuricClipperAudioProcessor::isMidiEffect() const { return false; }
double AuricClipperAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int AuricClipperAudioProcessor::getNumPrograms() { return 1; }
int AuricClipperAudioProcessor::getCurrentProgram() { return 0; }
void AuricClipperAudioProcessor::setCurrentProgram(int) {}
const juce::String AuricClipperAudioProcessor::getProgramName(int) { return {}; }
void AuricClipperAudioProcessor::changeProgramName(int, const juce::String&) {}

void AuricClipperAudioProcessor::prepareToPlay(double, int) {}
void AuricClipperAudioProcessor::releaseResources() {}

bool AuricClipperAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void AuricClipperAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    // TODO: Implement clipping/saturation DSP
}

bool AuricClipperAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* AuricClipperAudioProcessor::createEditor()
{
    return new AuricClipperAudioProcessorEditor(*this);
}

void AuricClipperAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AuricClipperAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AuricClipperAudioProcessor();
}
