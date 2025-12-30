//==============================================================================
// PluginEditor.cpp — AURIC Z-CLIP (Darkmode 3D) FULL FILE
//==============================================================================

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AuricLookAndFeel.h"

#include <cmath>

//==============================================================================
namespace
{
    static constexpr int kBaseW = 2048;
    static constexpr int kBaseH = 1365;

    inline juce::Rectangle<float> rectN (juce::Rectangle<float> b, float x, float y, float w, float h)
    {
        return { b.getX() + x * b.getWidth(),
                 b.getY() + y * b.getHeight(),
                 w * b.getWidth(),
                 h * b.getHeight() };
    }

    inline juce::Rectangle<float> squareCN (juce::Rectangle<float> b, float cx, float cy, float sizeN)
    {
        const float m  = (float) juce::jmin (b.getWidth(), b.getHeight());
        const float s  = sizeN * m;
        const float px = b.getX() + cx * b.getWidth();
        const float py = b.getY() + cy * b.getHeight();
        return { px - s * 0.5f, py - s * 0.5f, s, s };
    }

    inline juce::Rectangle<float> getPanel (juce::Rectangle<int> localBounds)
    {
        auto b = localBounds.toFloat();
        const float sx = b.getWidth()  / (float) kBaseW;
        const float sy = b.getHeight() / (float) kBaseH;
        const float s  = juce::jmin (sx, sy);

        const float w = (float) kBaseW * s;
        const float h = (float) kBaseH * s;
        return { b.getCentreX() - w * 0.5f, b.getCentreY() - h * 0.5f, w, h };
    }

    struct InvisibleToggleLF : juce::LookAndFeel_V4
    {
        void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override {}
    };

    inline InvisibleToggleLF& getInvisibleToggleLF()
    {
        static InvisibleToggleLF instance;
        return instance;
    }

    enum class UIClass { S, M, L };

    inline UIClass getUIClassForPanel (juce::Rectangle<float> panel)
    {
        const float w = panel.getWidth();
        if (w < 820.0f)   return UIClass::S;
        if (w < 1220.0f)  return UIClass::M;
        return UIClass::L;
    }

    inline juce::Font makeFont (float px, bool bold, float kerning)
    {
        px = juce::jlimit (10.0f, 260.0f, px);
        juce::Font f (px);
        f.setBold (bold);
        f.setExtraKerningFactor (kerning);
        f.setTypefaceName ("Montserrat");
        return f;
    }

    //==============================================================
    // Brushed metal panel
    //==============================================================
    inline void drawBrushedMetal (juce::Graphics& g, juce::Rectangle<float> r)
    {
        using namespace juce;

        g.setColour (AuricLookAndFeel::bgDark());
        g.fillRect (r);

        {
            ColourGradient grad (AuricLookAndFeel::bgMetalHi().darker (0.10f), r.getX(), r.getY(),
                                 AuricLookAndFeel::bgMetalLo().darker (0.05f), r.getX(), r.getBottom(),
                                 false);
            g.setGradientFill (grad);
            g.fillRect (r);
        }

        {
            Random rng (0xA11C0FFEu);
            const int y0 = (int) r.getY();
            const int y1 = (int) r.getBottom();

            for (int y = y0; y < y1; ++y)
            {
                const float a = 0.006f + rng.nextFloat() * 0.014f;
                g.setColour (Colours::white.withAlpha (a));
                g.drawLine (r.getX(), (float) y, r.getRight(), (float) y, 1.0f);
            }
        }

        {
            ColourGradient sheen (Colours::white.withAlpha (0.060f),
                                  r.getX(), r.getY() + r.getHeight() * 0.10f,
                                  Colours::transparentWhite,
                                  r.getX(), r.getY() + r.getHeight() * 0.52f,
                                  false);
            g.setGradientFill (sheen);
            g.fillRect (r);
        }

        {
            const float cx = r.getCentreX();
            const float cy = r.getY() + r.getHeight() * 0.34f;
            ColourGradient spot (Colours::white.withAlpha (0.055f), cx, cy,
                                 Colours::transparentWhite,      cx, r.getBottom(), true);
            g.setGradientFill (spot);
            g.fillRect (r);
        }

        {
            ColourGradient vig (Colours::transparentBlack,
                                r.getCentreX(), r.getCentreY(),
                                Colours::black.withAlpha (0.28f),
                                r.getX(), r.getY(), true);
            g.setGradientFill (vig);
            g.fillRect (r);
        }
    }

    //==============================================================
    // Bezel + inner rim
    //==============================================================
    inline void drawBezel (juce::Graphics& g, juce::Rectangle<float> r)
    {
        using namespace juce;

        const float s = jmin (r.getWidth() / (float) kBaseW, r.getHeight() / (float) kBaseH);
        const float rad = 18.0f * s;

        {
            Path p;
            p.addRoundedRectangle (r.reduced (2.0f * s), rad);

            g.setColour (Colours::black.withAlpha (0.70f));
            g.strokePath (p, PathStrokeType (2.2f * s));

            g.setColour (Colours::white.withAlpha (0.05f));
            g.strokePath (p, PathStrokeType (1.0f * s));
        }

        {
            Path p;
            p.addRoundedRectangle (r.reduced (10.0f * s), rad - 2.0f * s);

            g.setColour (Colours::black.withAlpha (0.62f));
            g.strokePath (p, PathStrokeType (2.0f * s));

            g.setColour (Colours::white.withAlpha (0.035f));
            g.strokePath (p, PathStrokeType (1.0f * s));
        }
    }

    //==============================================================
    // Screws (phillips + recessed)
    //==============================================================
    inline void drawScrews (juce::Graphics& g, juce::Rectangle<float> r)
    {
        using namespace juce;

        const float s = jmin (r.getWidth() / (float) kBaseW, r.getHeight() / (float) kBaseH);

        auto one = [&](float cx, float cy, float rotDeg)
        {
            const float dWell  = 90.0f * s;
            const float dHead  = 64.0f * s;
            const float dHi    = 22.0f * s;
            const float rrW    = dWell * 0.5f;
            const float rrH    = dHead * 0.5f;

            g.setColour (Colours::black.withAlpha (0.58f));
            g.fillEllipse (cx - rrW + 1.5f * s, cy - rrW + 2.5f * s, dWell, dWell);

            {
                ColourGradient cav (Colours::black.withAlpha (0.62f), cx - rrW, cy - rrW,
                                    Colours::black.withAlpha (0.16f), cx + rrW, cy + rrW,
                                    false);
                g.setGradientFill (cav);
                g.fillEllipse (cx - rrW, cy - rrW, dWell, dWell);

                g.setColour (Colours::white.withAlpha (0.05f));
                g.drawEllipse (cx - rrW, cy - rrW, dWell, dWell, 1.0f * s);
            }

            {
                auto metal = AuricLookAndFeel::screwMetal();
                ColourGradient head (metal.brighter (0.18f), cx - rrH, cy - rrH,
                                     metal.darker   (0.65f), cx + rrH, cy + rrH,
                                     false);
                g.setGradientFill (head);
                g.fillEllipse (cx - rrH, cy - rrH, dHead, dHead);

                g.setColour (Colours::black.withAlpha (0.55f));
                g.drawEllipse (cx - rrH, cy - rrH, dHead, dHead, 1.2f * s);

                g.setColour (Colours::white.withAlpha (0.07f));
                g.drawEllipse (cx - rrH + 0.9f * s, cy - rrH + 0.6f * s,
                               dHead - 1.8f * s, dHead - 1.8f * s, 0.9f * s);
            }

            {
                const float slotW = 30.0f * s;
                const float slotH = 6.6f * s;

                auto hSlot = Rectangle<float> (0, 0, slotW, slotH).withCentre ({ cx, cy });
                auto vSlot = Rectangle<float> (0, 0, slotH, slotW).withCentre ({ cx, cy });

                Path p;
                p.addRoundedRectangle (hSlot, slotH * 0.45f);
                p.addRoundedRectangle (vSlot, slotH * 0.45f);

                p.applyTransform (AffineTransform::rotation (degreesToRadians (rotDeg), cx, cy));

                Path sh = p;
                sh.applyTransform (AffineTransform::translation (0.9f * s, 1.2f * s));
                g.setColour (Colours::black.withAlpha (0.68f));
                g.fillPath (sh);

                ColourGradient slotG (Colours::black.withAlpha (0.78f), cx, cy - 10.0f * s,
                                      Colours::black.withAlpha (0.28f), cx, cy + 10.0f * s,
                                      false);
                g.setGradientFill (slotG);
                g.fillPath (p);

                g.setColour (Colours::white.withAlpha (0.10f));
                g.strokePath (p, PathStrokeType (0.85f * s));
            }

            g.setColour (Colours::white.withAlpha (0.10f));
            g.fillEllipse (cx - rrH * 0.55f, cy - rrH * 0.55f, dHi, dHi);
        };

        const float xL = r.getX() + r.getWidth()  * 0.040f;
        const float xR = r.getX() + r.getWidth()  * 0.960f;
        const float yT = r.getY() + r.getHeight() * 0.060f;
        const float yB = r.getY() + r.getHeight() * 0.925f;

        one (xL, yT,  10.0f);
        one (xR, yT, -12.0f);
        one (xL, yB, -8.0f);
        one (xR, yB,  12.0f);
    }

    //==============================================================
    // Bottom accent strip
    //==============================================================
    inline void drawBottomAccent (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour accent)
    {
        using namespace juce;

        const float s = jmin (r.getWidth() / (float) kBaseW, r.getHeight() / (float) kBaseH);
        const float margin = 54.0f * s;
        const float y = r.getBottom() - 30.0f * s;

        g.setColour (accent.withAlpha (0.12f));
        g.fillRect (r.getX() + margin, y - 4.8f * s, r.getWidth() - margin * 2.0f, 10.5f * s);

        g.setColour (accent.withAlpha (0.78f));
        g.fillRect (r.getX() + margin + 2.0f * s, y,
                    r.getWidth() - (margin + 2.0f * s) * 2.0f, 2.2f * s);
    }

    //==============================================================
    // Small hole decor
    //==============================================================
    inline void drawSmallHole (juce::Graphics& g, juce::Rectangle<float> r)
    {
        using namespace juce;

        const float d = jmin (r.getWidth(), r.getHeight());
        auto hole = Rectangle<float> (0, 0, d, d).withCentre (r.getCentre());

        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillEllipse (hole.translated (0.8f, 1.0f));

        ColourGradient gg (AuricLookAndFeel::knobDark().brighter (0.05f), hole.getX(), hole.getY(),
                           AuricLookAndFeel::knobDark().darker   (0.60f), hole.getRight(), hole.getBottom(),
                           false);
        g.setGradientFill (gg);
        g.fillEllipse (hole);

        g.setColour (AuricLookAndFeel::knobRing().darker (0.45f).withAlpha (0.90f));
        g.drawEllipse (hole, 1.0f);
    }

    inline void snapEditorSize (juce::AudioProcessorEditor& ed, UIClass c)
    {
        auto makeH = [](int w) { return (int) std::round (w * (double) kBaseH / (double) kBaseW); };

        if (c == UIClass::S) ed.setSize (760,  makeH (760));
        if (c == UIClass::M) ed.setSize (1024, makeH (1024));
        if (c == UIClass::L) ed.setSize (1400, makeH (1400));
    }
}

//==============================================================================
// Editor
//==============================================================================

AuricClipperAudioProcessorEditor::AuricClipperAudioProcessorEditor (AuricClipperAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&auricLookAndFeel);
    setWantsKeyboardFocus (true);
    setOpaque (true);

    setupKnob (driveKnob,   driveLabel,   "DRIVE");
    setupKnob (preKnob,     preLabel,     "PRE");
    setupKnob (satClipKnob, satClipLabel, "SAT/CLIP");
    setupKnob (trimKnob,    trimLabel,    "TRIM");
    setupKnob (mixKnob,     mixLabel,     "MIX");

    driveLabel.setVisible (false);
    preLabel.setVisible (false);
    satClipLabel.setVisible (false);
    trimLabel.setVisible (false);
    mixLabel.setVisible (false);

    ceilingSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    ceilingSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    ceilingSlider.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    ceilingSlider.setWantsKeyboardFocus (false);
    addAndMakeVisible (ceilingSlider);

    oversampleButton.setClickingTogglesState (true);
    addAndMakeVisible (oversampleButton);

    oversampleButton.setButtonText ("");
    oversampleButton.setLookAndFeel (&getInvisibleToggleLF());
    oversampleButton.setColour (juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    oversampleButton.setColour (juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    oversampleButton.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    oversampleButton.setWantsKeyboardFocus (false);

    driveAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "drive",     driveKnob);
    preAttachment        = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "pre",       preKnob);
    satClipAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "satclip",   satClipKnob);
    trimAttachment       = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "trim",      trimKnob);
    mixAttachment        = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "mix",       mixKnob);
    ceilingAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "ceiling",   ceilingSlider);
    oversampleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (audioProcessor.apvts, "oversample", oversampleButton);

    snapEditorSize (*this, UIClass::M);
    setResizable (true, true);
    setResizeLimits (620, 414, 2200, 1467);

    referenceOverlay = loadReferenceOverlay();
    startTimerHz (30);
}

AuricClipperAudioProcessorEditor::~AuricClipperAudioProcessorEditor()
{
    stopTimer();
    oversampleButton.setLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

//==============================================================================

void AuricClipperAudioProcessorEditor::setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& text)
{
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    knob.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    knob.setWantsKeyboardFocus (false);
    knob.setName (text);
    addAndMakeVisible (knob);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);
}

//==============================================================================
// LED besar kiri bawah (indikator)
void AuricClipperAudioProcessorEditor::drawStatusLed (juce::Graphics& g, juce::Rectangle<float> ledBounds, bool isOn)
{
    using namespace juce;

    const float d = jmin (ledBounds.getWidth(), ledBounds.getHeight());
    auto led = Rectangle<float> (0, 0, d, d).withCentre (ledBounds.getCentre());

    auto hole = led.expanded (d * 0.28f);

    g.setColour (Colours::black.withAlpha (0.55f));
    g.fillEllipse (hole.translated (0.9f, 1.2f));

    g.setColour (AuricLookAndFeel::knobDark().darker (0.35f));
    g.fillEllipse (hole);

    g.setColour (AuricLookAndFeel::knobRing().darker (0.45f).withAlpha (0.90f));
    g.drawEllipse (hole, 1.0f);

    if (isOn)
    {
        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.12f));
        g.fillEllipse (led.expanded (d * 0.42f));

        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.18f));
        g.fillEllipse (led.expanded (d * 0.24f));

        ColourGradient lg (AuricLookAndFeel::accentGlow().withAlpha (0.92f),
                           led.getCentreX(), led.getY(),
                           AuricLookAndFeel::accentPink().darker (0.25f),
                           led.getCentreX(), led.getBottom(), false);
        g.setGradientFill (lg);
        g.fillEllipse (led);

        g.setColour (Colours::white.withAlpha (0.58f));
        g.fillEllipse (led.reduced (d * 0.36f).translated (-d * 0.10f, -d * 0.12f));

        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.74f));
        g.drawEllipse (led, 1.0f);
    }
    else
    {
        g.setColour (AuricLookAndFeel::knobDark().darker (0.60f));
        g.fillEllipse (led);
        g.setColour (AuricLookAndFeel::knobRing().darker (0.55f).withAlpha (0.85f));
        g.drawEllipse (led, 1.0f);
    }
}

//==============================================================================

juce::Image AuricClipperAudioProcessorEditor::loadReferenceOverlay() const
{
    using namespace juce;

    auto loadFromDir = [] (const File& dir) -> Image
    {
        if (! dir.exists()) return {};
        auto f = dir.getChildFile ("image.png");
        if (f.existsAsFile())
            return ImageFileFormat::loadFrom (f);
        return {};
    };

    if (auto img = loadFromDir (File::getCurrentWorkingDirectory()); img.isValid())
        return img;

    auto exe = File::getSpecialLocation (File::currentExecutableFile);
    auto d = exe.getParentDirectory();
    for (int i = 0; i < 6; ++i)
    {
        if (auto img = loadFromDir (d); img.isValid())
            return img;
        d = d.getParentDirectory();
    }
    return {};
}

//==============================================================================

void AuricClipperAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;

    g.setImageResamplingQuality (Graphics::highResamplingQuality);

    auto panel = getPanel (getLocalBounds().reduced (2));
    const auto ui = getUIClassForPanel (panel);

    g.fillAll (AuricLookAndFeel::bgDark());

    ::drawBrushedMetal (g, panel);
    ::drawBezel (g, panel);
    ::drawScrews (g, panel);

    const float s = jmin (panel.getWidth()  / (float) kBaseW,
                          panel.getHeight() / (float) kBaseH);

    // ===== Titles =====
    {
        auto r = rectN (panel, 0.090f, 0.070f, 0.280f, 0.100f);
        g.setFont (makeFont (86.0f * s, true, 0.06f));
        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.16f));
        g.drawText ("AURIC", r.translated (-1.0f * s, 1.0f * s), Justification::left);

        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.68f));
        g.drawText ("AURIC", r, Justification::left);
    }
    {
        auto r = rectN (panel, 0.640f, 0.070f, 0.300f, 0.100f);
        g.setFont (makeFont (86.0f * s, true, 0.06f));
        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.18f));
        g.drawText ("Z-CLIP", r.translated (-1.0f * s, 1.0f * s), Justification::right);

        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.80f));
        g.drawText ("Z-CLIP", r, Justification::right);
    }

    // ===== Small labels =====
    {
        float labelPx = 42.0f * s;
        if (ui == UIClass::S) labelPx *= 0.92f;

        g.setFont (makeFont (labelPx, false, 0.16f));
        g.setColour (AuricLookAndFeel::textLight().withAlpha (0.70f));

        g.drawText ("PRE",      rectN (panel, 0.150f, 0.175f, 0.120f, 0.06f), Justification::centred);
        g.drawText ("TRIM",     rectN (panel, 0.200f, 0.415f, 0.120f, 0.06f), Justification::centred);
        g.drawText ("SAT/CLIP", rectN (panel, 0.730f, 0.175f, 0.180f, 0.06f), Justification::centred);
        g.drawText ("MIX",      rectN (panel, 0.700f, 0.415f, 0.120f, 0.06f), Justification::centred);
    }

    // ===== DRIVE label =====
    {
        float yTop = 0.545f;
        float hN   = 0.100f;
        float px   = 90.0f * s;

        if (ui == UIClass::S) { yTop = 0.548f; hN = 0.095f; px *= 0.88f; }
        if (ui == UIClass::L) { yTop = 0.542f; hN = 0.105f; }

        auto r = rectN (panel, 0.380f, yTop, 0.240f, hN);
        g.setFont (makeFont (px, true, 0.10f));
        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.62f));
        g.drawText ("DRIVE", r, Justification::centred);
    }

    // ===== 2X label =====
    {
        float px = 46.0f * s;
        if (ui == UIClass::S) px *= 0.92f;

        g.setFont (makeFont (px, false, 0.18f));
        g.setColour (AuricLookAndFeel::textLight().withAlpha (0.70f));
        g.drawText ("2X", rectN (panel, 0.220f, 0.765f, 0.08f, 0.06f), Justification::centredLeft);
    }

    // ===== small hole decor =====
    {
        auto hole = squareCN (panel, 0.355f, 0.790f, 0.016f);
        ::drawSmallHole (g, hole);
    }

    // ===== OUTPUT CEILING label =====
    {
        float px = 42.0f * s;
        if (ui == UIClass::S) px *= 0.90f;

        g.setFont (makeFont (px, false, 0.16f));
        g.setColour (AuricLookAndFeel::textLight().withAlpha (0.66f));

        auto r = rectN (panel, 0.585f, 0.765f, 0.320f, 0.060f);
        g.drawText ("OUTPUT CEILING", r, Justification::centredLeft);
    }

    // ===== LED =====
    {
        auto led = squareCN (panel, 0.175f, 0.790f, (ui == UIClass::S ? 0.060f : 0.058f));
        drawStatusLed (g, led, oversampleButton.getToggleState());
    }

    ::drawBottomAccent (g, panel, AuricLookAndFeel::accentPink());

    // Reference overlay (toggle with O)
    if (showReferenceOverlay)
    {
        if (! referenceOverlay.isValid())
            referenceOverlay = loadReferenceOverlay();

        if (referenceOverlay.isValid())
        {
            g.setOpacity (0.45f);
            g.drawImageWithin (referenceOverlay,
                               (int) panel.getX(), (int) panel.getY(),
                               (int) panel.getWidth(), (int) panel.getHeight(),
                               RectanglePlacement::stretchToFit);
            g.setOpacity (1.0f);
        }
    }
}

void AuricClipperAudioProcessorEditor::resized()
{
    auto panel = getPanel (getLocalBounds().reduced (2));
    const auto ui = getUIClassForPanel (panel);

    // ===== Big knob DRIVE (hero) =====
    float bigSizeN   = 0.49f;
    float bigCenterX = 0.500f;
    float bigCenterY = 0.325f;

    // ===== Small knobs =====
    float smallN = 0.218f;

    float leftX1  = 0.190f; // PRE
    float leftX2  = 0.240f; // TRIM
    float rightX1 = 0.810f; // SAT/CLIP
    float rightX2 = 0.760f; // MIX

    float smallY1 = 0.305f;
    float smallY2 = 0.540f;

    // LED
    float ledCX    = 0.175f;
    float ledCY    = 0.790f;
    float ledSizeN = 0.058f;

    // OUTPUT CEILING slider
    float sliderX  = 0.340f;
    float sliderY  = 0.755f;
    float sliderW  = 0.240f;
    float sliderH  = 0.078f;

    if (ui == UIClass::S)
    {
        bigSizeN   = 0.47f;
        bigCenterY = 0.320f;

        smallN     = 0.205f;

        leftX1     = 0.195f;
        leftX2     = 0.245f;
        rightX1    = 0.805f;
        rightX2    = 0.755f;

        ledCX      = 0.180f;
        ledSizeN   = 0.060f;

        sliderW    = 0.230f;
        sliderY    = 0.768f;
        sliderH    = 0.060f;
    }
    else if (ui == UIClass::L)
    {
        bigSizeN   = 0.51f;
        bigCenterY = 0.330f;

        smallN     = 0.228f;

        leftX1     = 0.185f;
        leftX2     = 0.235f;
        rightX1    = 0.815f;
        rightX2    = 0.765f;

        ledCX      = 0.170f;
        ledSizeN   = 0.056f;

        sliderW    = 0.240f;
        sliderY    = 0.760f;
    }

    // ===== Apply bounds =====
    driveKnob.setBounds (squareCN (panel, bigCenterX, bigCenterY, bigSizeN).toNearestInt());

    preKnob.setBounds     (squareCN (panel, leftX1,  smallY1, smallN).toNearestInt());
    satClipKnob.setBounds (squareCN (panel, rightX1, smallY1, smallN).toNearestInt());
    trimKnob.setBounds    (squareCN (panel, leftX2,  smallY2, smallN).toNearestInt());
    mixKnob.setBounds     (squareCN (panel, rightX2, smallY2, smallN).toNearestInt());

    oversampleButton.setBounds (squareCN (panel, ledCX, ledCY, ledSizeN).toNearestInt());
    ceilingSlider.setBounds    (rectN (panel, sliderX, sliderY, sliderW, sliderH).toNearestInt());
}

//==============================================================================

bool AuricClipperAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    const auto ch = key.getTextCharacter();

    if (ch == 'o' || ch == 'O')
    {
        showReferenceOverlay = ! showReferenceOverlay;
        repaint();
        return true;
    }

    if (ch == '1') { snapEditorSize (*this, UIClass::S); return true; }
    if (ch == '2') { snapEditorSize (*this, UIClass::M); return true; }
    if (ch == '3') { snapEditorSize (*this, UIClass::L); return true; }

    return false;
}

//==============================================================================

void AuricClipperAudioProcessorEditor::timerCallback()
{
    const bool currentState = oversampleButton.getToggleState();
    if (currentState != lastLedState)
    {
        lastLedState = currentState;
        repaint();
    }
}
