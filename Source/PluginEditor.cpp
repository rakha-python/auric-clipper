//==============================================================================
// PluginEditor.cpp — AURIC Z-CLIP (Darkmode 3D) FULL FILE
// NOTE:
// - Requires AuricLookAndFeel.h (the one we refined: BIG knob ≠ SMALL knob)
// - This file keeps your 1:1 normalized layout + screws + brushed metal + bottom neon strip
//==============================================================================

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AuricLookAndFeel.h"

#include <cmath>

//==============================================================================
// Layout base mengikuti artwork reference (AURIC Z-CLIP)
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

    //==============================================================
    // Invisible toggle LF (oversampleButton jadi click-area saja)
    //==============================================================
    struct InvisibleToggleLF : juce::LookAndFeel_V4
    {
        void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override {}
    };

    inline InvisibleToggleLF& getInvisibleToggleLF()
    {
        static InvisibleToggleLF instance;
        return instance;
    }

    //==============================================================
    // Responsive size class (S/M/L)
    //==============================================================
    enum class UIClass { S, M, L };

    inline UIClass getUIClassForPanel (juce::Rectangle<float> panel)
    {
        const float w = panel.getWidth();
        if (w < 820.0f)   return UIClass::S;
        if (w < 1220.0f)  return UIClass::M;
        return UIClass::L;
    }

    //==============================================================
    // Font helper (SAFE - no FontOptions)
    //==============================================================
    inline juce::Font makeFont (float px, bool bold, float kerning)
    {
        px = juce::jlimit (10.0f, 260.0f, px);

        juce::Font f (px);
        f.setBold (bold);
        f.setExtraKerningFactor (kerning);
        f.setTypefaceName ("Montserrat"); // OK if missing: JUCE will fallback
        return f;
    }

    //==============================================================
    // Brushed metal panel (horizontal brush + sheen + vignette)
    //==============================================================
    inline void drawBrushedMetal (juce::Graphics& g, juce::Rectangle<float> r)
    {
        using namespace juce;

        g.setColour (AuricLookAndFeel::bgDark());
        g.fillRect (r);

        {
            ColourGradient grad (AuricLookAndFeel::bgMetal().brighter (0.08f), r.getX(), r.getY(),
                                 AuricLookAndFeel::bgDark().darker   (0.35f), r.getX(), r.getBottom(),
                                 false);
            g.setGradientFill (grad);
            g.fillRect (r);
        }

        // Horizontal brushing noise
        {
            Random rng (0xA11C0FFEu);
            const int y0 = (int) r.getY();
            const int y1 = (int) r.getBottom();

            for (int y = y0; y < y1; ++y)
            {
                const float a = 0.0035f + rng.nextFloat() * 0.0075f;
                g.setColour (Colours::white.withAlpha (a));
                g.drawLine (r.getX(), (float) y, r.getRight(), (float) y, 1.0f);
            }
        }

        // Top sheen
        {
            ColourGradient sheen (Colours::white.withAlpha (0.035f),
                                  r.getX(), r.getY() + r.getHeight() * 0.12f,
                                  Colours::transparentWhite,
                                  r.getX(), r.getY() + r.getHeight() * 0.62f,
                                  false);
            g.setGradientFill (sheen);
            g.fillRect (r);
        }

        // Vignette
        {
            ColourGradient vig (Colours::transparentBlack,
                                r.getCentreX(), r.getCentreY(),
                                Colours::black.withAlpha (0.48f),
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
    // Screws (hardware feel: recessed + head + slot depth)
    //==============================================================
    inline void drawScrews (juce::Graphics& g, juce::Rectangle<float> r)
    {
        using namespace juce;

        const float s = jmin (r.getWidth() / (float) kBaseW, r.getHeight() / (float) kBaseH);

        auto one = [&](float cx, float cy, float rotDeg)
        {
            const float dWell  = 88.0f * s;   // recess diameter
            const float dHead  = 62.0f * s;   // screw head diameter
            const float dHi    = 22.0f * s;   // tiny spec
            const float rrW    = dWell * 0.5f;
            const float rrH    = dHead * 0.5f;

            // 1) recess shadow
            g.setColour (Colours::black.withAlpha (0.55f));
            g.fillEllipse (cx - rrW + 1.4f * s, cy - rrW + 2.4f * s, dWell, dWell);

            // 2) recess cavity gradient
            {
                ColourGradient cav (Colours::black.withAlpha (0.55f), cx - rrW, cy - rrW,
                                    Colours::black.withAlpha (0.15f), cx + rrW, cy + rrW,
                                    false);
                g.setGradientFill (cav);
                g.fillEllipse (cx - rrW, cy - rrW, dWell, dWell);

                g.setColour (Colours::white.withAlpha (0.06f));
                g.drawEllipse (cx - rrW, cy - rrW, dWell, dWell, 1.1f * s);
            }

            // 3) screw head metal
            {
                ColourGradient head (AuricLookAndFeel::screwMetal().brighter (0.28f), cx - rrH, cy - rrH,
                                     AuricLookAndFeel::screwMetal().darker   (0.55f), cx + rrH, cy + rrH,
                                     false);
                g.setGradientFill (head);
                g.fillEllipse (cx - rrH, cy - rrH, dHead, dHead);

                g.setColour (Colours::black.withAlpha (0.55f));
                g.drawEllipse (cx - rrH, cy - rrH, dHead, dHead, 1.2f * s);

                g.setColour (Colours::white.withAlpha (0.08f));
                g.drawEllipse (cx - rrH + 0.8f * s, cy - rrH + 0.6f * s,
                               dHead - 1.6f * s, dHead - 1.6f * s, 0.9f * s);
            }

            // 4) slot (depth + highlight) with rotation
            {
                const float slotW = 30.0f * s;
                const float slotH = 6.8f * s;
                auto slot = Rectangle<float> (0, 0, slotW, slotH).withCentre ({ cx, cy });

                Path p;
                p.addRoundedRectangle (slot, slotH * 0.45f);

                AffineTransform t = AffineTransform::rotation (degreesToRadians (rotDeg), cx, cy);
                p.applyTransform (t);

                g.setColour (Colours::black.withAlpha (0.65f));
                Path pShadow = p;
                pShadow.applyTransform (AffineTransform::translation (0.9f * s, 1.2f * s));
                g.fillPath (pShadow);

                ColourGradient slotG (Colours::black.withAlpha (0.75f), cx, cy - 8.0f * s,
                                      Colours::black.withAlpha (0.25f), cx, cy + 8.0f * s,
                                      false);
                g.setGradientFill (slotG);
                g.fillPath (p);

                g.setColour (Colours::white.withAlpha (0.10f));
                g.strokePath (p, PathStrokeType (0.9f * s));
            }

            // 5) tiny specular highlight
            g.setColour (Colours::white.withAlpha (0.11f));
            g.fillEllipse (cx - rrH * 0.55f, cy - rrH * 0.55f, dHi, dHi);
        };

        const float xL = r.getX() + r.getWidth()  * 0.040f;
        const float xR = r.getX() + r.getWidth()  * 0.960f;
        const float yT = r.getY() + r.getHeight() * 0.060f;
        const float yB = r.getY() + r.getHeight() * 0.925f;

        one (xL, yT, -10.0f);
        one (xR, yT,  12.0f);
        one (xL, yB,   9.0f);
        one (xR, yB, -12.0f);
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

        g.setColour (accent.withAlpha (0.10f));
        g.fillRect (r.getX() + margin, y - 4.2f * s, r.getWidth() - margin * 2.0f, 9.0f * s);

        g.setColour (accent.withAlpha (0.72f));
        g.fillRect (r.getX() + margin + 2.0f * s, y,
                    r.getWidth() - (margin + 2.0f * s) * 2.0f, 2.0f * s);
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

    //==============================================================
    // Snap window sizes (S/M/L)
    //==============================================================
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

    // We paint labels manually in paint() (match ref)
    driveLabel.setVisible (false);
    preLabel.setVisible (false);
    satClipLabel.setVisible (false);
    trimLabel.setVisible (false);
    mixLabel.setVisible (false);

    ceilingSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    ceilingSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (ceilingSlider);

    oversampleButton.setClickingTogglesState (true);
    addAndMakeVisible (oversampleButton);

    // Invisible click-area (LED drawn in paint)
    oversampleButton.setButtonText ("");
    oversampleButton.setLookAndFeel (&getInvisibleToggleLF());
    oversampleButton.setColour (juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
    oversampleButton.setColour (juce::ToggleButton::tickColourId, juce::Colours::transparentBlack);
    oversampleButton.setMouseCursor (juce::MouseCursor::PointingHandCursor);

    // Attachments
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
}

AuricClipperAudioProcessorEditor::~AuricClipperAudioProcessorEditor()
{
    oversampleButton.setLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

//==============================================================================

void AuricClipperAudioProcessorEditor::setupKnob (juce::Slider& knob, juce::Label& label, const juce::String& text)
{
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
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
        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.10f));
        g.fillEllipse (led.expanded (d * 0.42f));

        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.16f));
        g.fillEllipse (led.expanded (d * 0.24f));

        ColourGradient lg (AuricLookAndFeel::accentGlow().withAlpha (0.90f),
                           led.getCentreX(), led.getY(),
                           AuricLookAndFeel::accentPink().darker (0.25f),
                           led.getCentreX(), led.getBottom(), false);
        g.setGradientFill (lg);
        g.fillEllipse (led);

        g.setColour (Colours::white.withAlpha (0.55f));
        g.fillEllipse (led.reduced (d * 0.36f).translated (-d * 0.10f, -d * 0.12f));

        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.70f));
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

    auto panel = getPanel (getLocalBounds());
    const auto ui = getUIClassForPanel (panel);

    g.fillAll (AuricLookAndFeel::bgDark().darker (0.70f));

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

        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.62f));
        g.drawText ("AURIC", r, Justification::left);
    }
    {
        auto r = rectN (panel, 0.640f, 0.070f, 0.300f, 0.100f);
        g.setFont (makeFont (86.0f * s, true, 0.06f));
        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.18f));
        g.drawText ("Z-CLIP", r.translated (-1.0f * s, 1.0f * s), Justification::right);

        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.74f));
        g.drawText ("Z-CLIP", r, Justification::right);
    }

    // ===== Small labels (di atas knob masing-masing) =====
    {
        float labelPx = 42.0f * s;
        if (ui == UIClass::S) labelPx *= 0.92f;

        g.setFont (makeFont (labelPx, false, 0.16f));
        g.setColour (AuricLookAndFeel::textLight().withAlpha (0.55f));

        g.drawText ("PRE",      rectN (panel, 0.065f, 0.175f, 0.100f, 0.06f), Justification::centred);
        g.drawText ("TRIM",     rectN (panel, 0.115f, 0.415f, 0.100f, 0.06f), Justification::centred);
        g.drawText ("SAT/CLIP", rectN (panel, 0.815f, 0.175f, 0.140f, 0.06f), Justification::centred);
        g.drawText ("MIX",      rectN (panel, 0.785f, 0.415f, 0.100f, 0.06f), Justification::centred);
    }

    // ===== DRIVE label (di bawah knob besar) =====
    {
        float yTop = 0.540f;
        float hN   = 0.100f;
        float px   = 90.0f * s;

        if (ui == UIClass::S) { yTop = 0.545f; hN = 0.095f; px *= 0.88f; }
        if (ui == UIClass::L) { yTop = 0.535f; hN = 0.105f; }

        auto r = rectN (panel, 0.380f, yTop, 0.240f, hN);
        g.setFont (makeFont (px, true, 0.10f));
        g.setColour (AuricLookAndFeel::accentPink().withAlpha (0.55f));
        g.drawText ("DRIVE", r, Justification::centred);
    }

    // ===== 2X label (di samping kanan LED) =====
    {
        float px = 46.0f * s;
        if (ui == UIClass::S) px *= 0.92f;

        g.setFont (makeFont (px, false, 0.18f));
        g.setColour (AuricLookAndFeel::textLight().withAlpha (0.60f));
        g.drawText ("2X", rectN (panel, 0.160f, 0.765f, 0.08f, 0.06f), Justification::centredLeft);
    }

    // ===== small hole decor =====
    {
        auto hole = squareCN (panel, 0.340f, 0.790f, 0.016f);
        ::drawSmallHole (g, hole);
    }

    // ===== OUTPUT CEILING label =====
    {
        float px = 42.0f * s;
        if (ui == UIClass::S) px *= 0.90f;

        g.setFont (makeFont (px, false, 0.16f));
        g.setColour (AuricLookAndFeel::textLight().withAlpha (0.52f));

        auto r = rectN (panel, 0.580f, 0.765f, 0.280f, 0.060f);
        g.drawText ("OUTPUT CEILING", r, Justification::centredLeft, false);
    }

    // ===== LED (draw) =====
    {
        auto led = squareCN (panel, 0.115f, 0.790f, (ui == UIClass::S ? 0.058f : 0.055f));
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

//==============================================================================

void AuricClipperAudioProcessorEditor::resized()
{
    auto panel = getPanel (getLocalBounds());
    const auto ui = getUIClassForPanel (panel);

    // ===== Big knob DRIVE (tengah) =====
    float bigSizeN   = 0.38f;
    float bigCenterX = 0.500f;
    float bigCenterY = 0.340f;

    // ===== Small knobs =====
    float smallN   = 0.135f;

    // X positions
    float leftX1   = 0.115f;   // PRE
    float leftX2   = 0.165f;   // TRIM
    float rightX1  = 0.885f;   // SAT/CLIP
    float rightX2  = 0.835f;   // MIX

    // Y positions
    float smallY1  = 0.280f;
    float smallY2  = 0.520f;

    // LED 2X
    float ledCX    = 0.115f;
    float ledCY    = 0.790f;
    float ledSizeN = 0.055f;

    // OUTPUT CEILING slider
    float sliderX  = 0.380f;
    float sliderY  = 0.765f;
    float sliderW  = 0.200f;
    float sliderH  = 0.055f;

    if (ui == UIClass::S)
    {
        bigSizeN   = 0.36f;
        bigCenterY = 0.335f;

        smallN   = 0.130f;

        leftX1   = 0.120f;
        leftX2   = 0.170f;
        rightX1  = 0.880f;
        rightX2  = 0.830f;

        ledCX    = 0.120f;
        ledSizeN = 0.058f;

        sliderW  = 0.195f;
        sliderY  = 0.768f;
        sliderH  = 0.060f;
    }
    else if (ui == UIClass::L)
    {
        bigSizeN   = 0.40f;
        bigCenterY = 0.345f;

        smallN   = 0.140f;

        leftX1   = 0.110f;
        leftX2   = 0.160f;
        rightX1  = 0.890f;
        rightX2  = 0.840f;

        ledCX    = 0.110f;
        ledSizeN = 0.052f;

        sliderW  = 0.190f;
        sliderY  = 0.760f;
    }

    driveKnob.setBounds (squareCN (panel, bigCenterX, bigCenterY, bigSizeN).toNearestInt());

    preKnob.setBounds     (squareCN (panel, leftX1,  smallY1, smallN).toNearestInt());
    satClipKnob.setBounds (squareCN (panel, rightX1, smallY1, smallN).toNearestInt());
    trimKnob.setBounds    (squareCN (panel, leftX2,  smallY2, smallN).toNearestInt());
    mixKnob.setBounds     (squareCN (panel, rightX2, smallY2, smallN).toNearestInt());

    oversampleButton.setBounds (squareCN (panel, ledCX, ledCY, ledSizeN).toNearestInt());
    ceilingSlider.setBounds (rectN (panel, sliderX, sliderY, sliderW, sliderH).toNearestInt());
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
