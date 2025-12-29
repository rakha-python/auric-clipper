#pragma once
#include <JuceHeader.h>
#include <cmath>

class AuricLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // ============================================================
    // Palette - AURIC Z-CLIP Darkmode 3D (Magenta Luxe)
    // ============================================================
    static juce::Colour bgDark()      { return juce::Colour (0xFF0A0A09); }
    static juce::Colour bgMetal()     { return juce::Colour (0xFF141312); }
    static juce::Colour bgMetalHi()   { return juce::Colour (0xFF1F1D1C); }
    static juce::Colour bgMetalLo()   { return juce::Colour (0xFF070707); }

    static juce::Colour knobDark()    { return juce::Colour (0xFF0E0E0D); }
    static juce::Colour knobMid()     { return juce::Colour (0xFF1B1A19); }
    static juce::Colour knobRing()    { return juce::Colour (0xFF2A2928); }
    static juce::Colour crystalEdge() { return juce::Colour (0xFF3A3836); }

    // Magenta Luxe
    static juce::Colour accentPink()  { return juce::Colour (0xFF9E3F6E); }
    static juce::Colour neonCore()    { return juce::Colour (0xFFD14B9C); }
    static juce::Colour neonGlow()    { return juce::Colour (0xFFFF8ACD); }
    static juce::Colour accentGlow()  { return juce::Colour (0xFFEE74B8); }

    static juce::Colour textLight()   { return juce::Colour (0xFF5E5C5A); }
    static juce::Colour textBright()  { return juce::Colour (0xFFB8B8B8); }

    static juce::Colour screwMetal()  { return juce::Colour (0xFF3A3A3A); }

    AuricLookAndFeel()
    {
        setColour (juce::Slider::rotarySliderFillColourId,     neonCore());
        setColour (juce::Slider::rotarySliderOutlineColourId,  knobRing());
        setColour (juce::Slider::thumbColourId,                neonCore());
        setColour (juce::Label::textColourId,                  textLight());
        setColour (juce::TextButton::textColourOffId,          textBright());
        setColour (juce::TextButton::textColourOnId,           neonGlow());
        setColour (juce::TextButton::buttonOnColourId,         neonCore().withAlpha (0.35f));
    }

    // Optional: call from Editor::paint()
    static void paintAuricBrushedPanel (juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        using namespace juce;
        const auto r = bounds.toFloat();

        ColourGradient grad (bgMetalHi(), r.getCentreX(), r.getY(),
                             bgMetalLo(), r.getCentreX(), r.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRect (r);

        // subtle diagonal brushing
        g.setColour (Colours::white.withAlpha (0.016f));
        const float step = jmax (6.0f, r.getWidth() / 150.0f);
        for (float x = r.getX() - r.getHeight(); x < r.getRight() + r.getHeight(); x += step)
            g.drawLine (x, r.getBottom(), x + r.getHeight(), r.getY(), 1.0f);

        // vignette edge
        g.setColour (Colours::black.withAlpha (0.35f));
        g.drawRect (r, 2.0f);
    }

private:
    static float clamp01 (float v) { return juce::jlimit (0.0f, 1.0f, v); }

    // Big knob detection: name + size
    static bool isBigKnob (juce::Slider& s, int w, int h)
    {
        const auto nm = s.getName().toLowerCase();
        if (nm.contains ("drive") || nm.contains ("input") || nm.contains ("output") || nm.contains ("main"))
            return true;

        const float d = (float) juce::jmin (w, h);
        return d >= 150.0f;
    }

    static void drawKnobShadow (juce::Graphics& g, juce::Rectangle<float> r, float radius, bool big)
    {
        using namespace juce;

        const float ox = radius * 0.04f;
        const float oy = radius * 0.09f;

        for (int i = 6; i >= 0; --i)
        {
            const float spread = radius * (0.08f + 0.022f * (float) i);
            const float alpha  = big ? (0.12f - 0.014f * (float) i)
                                     : (0.10f - 0.012f * (float) i);

            g.setColour (Colours::black.withAlpha (jmax (0.0f, alpha)));
            g.fillEllipse (r.expanded (spread).translated (ox * (1.0f + i * 0.25f),
                                                          oy * (1.0f + i * 0.25f)));
        }

        g.setColour (Colours::black.withAlpha (big ? 0.55f : 0.45f));
        g.fillEllipse (r.expanded (radius * 0.02f).translated (ox * 0.45f, oy * 0.45f));
    }

    static void drawRadialBrushed (juce::Graphics& g, juce::Point<float> c,
                                   float innerR, float outerR, float alpha, int lines)
    {
        using namespace juce;
        if (alpha <= 0.0f || lines <= 0) return;

        g.setColour (Colours::white.withAlpha (alpha));
        Random rng (0xB12345u);

        for (int i = 0; i < lines; ++i)
        {
            const float a  = rng.nextFloat() * MathConstants<float>::twoPi;
            const float r1 = innerR + rng.nextFloat() * (outerR - innerR) * 0.35f;
            const float r2 = outerR - rng.nextFloat() * (outerR - innerR) * 0.15f;
            const float th = 0.35f + rng.nextFloat() * 0.55f;

            g.drawLine (c.x + std::cos (a) * r1, c.y + std::sin (a) * r1,
                        c.x + std::cos (a) * r2, c.y + std::sin (a) * r2, th);
        }
    }

    // ====== NEW: knurled/grip teeth ring for SMALL knobs (match ref) ======
    static void drawKnurledRing (juce::Graphics& g, juce::Point<float> c,
                                 float rMid, float toothLen, int teeth,
                                 float alpha)
    {
        using namespace juce;
        g.setColour (Colours::white.withAlpha (alpha));

        const float a0 = degreesToRadians (-135.0f);
        const float a1 = degreesToRadians ( 135.0f);
        for (int i = 0; i < teeth; ++i)
        {
            const float t = (float) i / (float) (teeth - 1);
            const float a = a0 + t * (a1 - a0);

            const float x1 = c.x + std::cos (a) * (rMid - toothLen);
            const float y1 = c.y + std::sin (a) * (rMid - toothLen);
            const float x2 = c.x + std::cos (a) * (rMid + toothLen);
            const float y2 = c.y + std::sin (a) * (rMid + toothLen);

            g.drawLine (x1, y1, x2, y2, 1.0f);
        }
    }

    // ====== NEW: tick dots around SMALL knobs (match ref) ======
    static void drawTickDots (juce::Graphics& g, juce::Point<float> c, float r, float dotR)
    {
        using namespace juce;
        const float a0 = degreesToRadians (-135.0f);
        const float a1 = degreesToRadians ( 135.0f);

        g.setColour (textLight().withAlpha (0.30f));
        for (int i = 0; i <= 12; ++i)
        {
            const float t = (float) i / 12.0f;
            const float a = a0 + t * (a1 - a0);
            const float px = c.x + std::cos (a) * r;
            const float py = c.y + std::sin (a) * r;
            g.fillEllipse (px - dotR, py - dotR, dotR * 2.0f, dotR * 2.0f);
        }
    }

    // ====== neon ring (dual) ======
    static void drawNeonRingZ (juce::Graphics& g, juce::Rectangle<float> ring, float thickness, bool big)
    {
        using namespace juce;

        const float t = thickness;

        // Luxe: big glow stronger, small glow subtle
        g.setColour (neonGlow().withAlpha (big ? 0.050f : 0.028f));
        g.drawEllipse (ring.expanded (t * 2.2f), t * 2.2f);

        g.setColour (neonGlow().withAlpha (big ? 0.085f : 0.045f));
        g.drawEllipse (ring.expanded (t * 1.3f), t * 1.6f);

        g.setColour (neonCore().withAlpha (0.92f));
        g.drawEllipse (ring, t);

        g.setColour (accentGlow().withAlpha (big ? 0.30f : 0.18f));
        g.drawEllipse (ring.reduced (t * 0.55f), juce::jmax (1.0f, t * 0.38f));

        g.setColour (Colours::white.withAlpha (big ? 0.14f : 0.08f));
        g.drawEllipse (ring, juce::jmax (0.8f, t * 0.22f));
    }

    static void drawCrystalEdge (juce::Graphics& g, juce::Rectangle<float> r, float stroke)
    {
        using namespace juce;
        g.setColour (crystalEdge().withAlpha (0.85f));
        g.drawEllipse (r, stroke);

        auto hi = r.reduced (stroke * 0.8f).translated (-r.getWidth() * 0.015f, -r.getHeight() * 0.02f);
        g.setColour (Colours::white.withAlpha (0.05f));
        g.drawEllipse (hi, stroke * 0.6f);
    }

public:
    // ============================================================
    // ROTARY: BIG = Knob Z, SMALL = knurled mini (ref match)
    // ============================================================
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        using namespace juce;
        g.setImageResamplingQuality (Graphics::highResamplingQuality);

        auto bounds = Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
        const float pad = jmax (2.0f, jmin (bounds.getWidth(), bounds.getHeight()) * 0.03f);
        bounds = bounds.reduced (pad);

        const float diam = jmin (bounds.getWidth(), bounds.getHeight());
        if (diam < 18.0f) return;

        const float radius = diam * 0.5f;
        const auto  c      = bounds.getCentre();

        const float pos   = clamp01 (sliderPos);
        const float angle = rotaryStartAngle + pos * (rotaryEndAngle - rotaryStartAngle);
        const bool  big   = isBigKnob (slider, width, height);

        drawKnobShadow (g, bounds, radius, big);

        if (big)
        {
            // ======================
            // BIG KNOB — KNOB Z (hero)
            // ======================

            // Outer bezel (thicker)
            {
                ColourGradient bezelGrad (bgMetalHi().brighter (0.12f), c.x, bounds.getY(),
                                          bgMetalLo().darker  (0.28f), c.x, bounds.getBottom(), false);
                g.setGradientFill (bezelGrad);
                g.fillEllipse (bounds);

                drawCrystalEdge (g, bounds.reduced (0.7f), 2.2f);

                g.setColour (Colours::black.withAlpha (0.60f));
                g.drawEllipse (bounds.reduced (radius * 0.10f), 2.4f);
            }

            // Cavity (deeper)
            auto cavity = bounds.reduced (radius * 0.12f);
            {
                ColourGradient cavGrad (knobDark().brighter (0.03f), c.x, cavity.getY(),
                                        Colours::black,             c.x, cavity.getBottom(), false);
                g.setGradientFill (cavGrad);
                g.fillEllipse (cavity);

                g.setColour (Colours::black.withAlpha (0.70f));
                g.drawEllipse (cavity, 1.7f);
            }

            // Neon ring (hero)
            auto neonRing = bounds.reduced (radius * 0.215f);
            drawNeonRingZ (g, neonRing, jmax (3.0f, radius * 0.070f), true);

            // Face (matte + brushed + gloss)
            auto face = bounds.reduced (radius * 0.40f);
            {
                ColourGradient faceGrad (knobMid().brighter (0.14f), c.x, face.getY(),
                                         knobDark().darker (0.68f),   c.x, face.getBottom(), false);
                g.setGradientFill (faceGrad);
                g.fillEllipse (face);

                drawRadialBrushed (g, c, radius * 0.09f, radius * 0.62f, 0.013f, 170);

                // inner dish (subtle)
                auto dish = face.reduced (radius * 0.06f);
                g.setColour (Colours::black.withAlpha (0.15f));
                g.drawEllipse (dish, 1.0f);

                // gloss highlight
                auto gloss = face.reduced (face.getWidth() * 0.32f)
                                .translated (-face.getWidth() * 0.08f, -face.getHeight() * 0.12f);
                ColourGradient gl (Colours::white.withAlpha (0.075f), gloss.getCentreX(), gloss.getY(),
                                  Colours::transparentWhite,         gloss.getCentreX(), gloss.getBottom(), false);
                g.setGradientFill (gl);
                g.fillEllipse (gloss);

                g.setColour (Colours::black.withAlpha (0.35f));
                g.drawEllipse (face, 1.1f);
            }

            // Pointer: small magenta insert (match ref vibe: subtle, not huge)
            {
                const float ptrLen   = radius * 0.22f;
                const float ptrW     = radius * 0.055f;
                const float ptrStart = radius * 0.12f;

                Path ptr;
                ptr.addRoundedRectangle (-ptrW * 0.5f, -(ptrStart + ptrLen),
                                         ptrW, ptrLen, ptrW * 0.45f);

                auto t = AffineTransform::rotation (angle).translated (c.x, c.y);

                g.setColour (neonGlow().withAlpha (0.12f));
                g.fillPath (ptr, t);

                g.setColour (neonCore().withAlpha (0.90f));
                g.fillPath (ptr, t);

                g.setColour (Colours::white.withAlpha (0.18f));
                g.strokePath (ptr, PathStrokeType (1.0f), t);
            }
        }
        else
        {
            // ======================
            // SMALL KNOBS — knurled + dot ticks (REF)
            // ======================

            // Outer bezel
            {
                ColourGradient bezelGrad (bgMetalHi().brighter (0.07f), c.x, bounds.getY(),
                                          bgMetalLo().darker  (0.32f), c.x, bounds.getBottom(), false);
                g.setGradientFill (bezelGrad);
                g.fillEllipse (bounds);

                g.setColour (Colours::black.withAlpha (0.60f));
                g.drawEllipse (bounds, 1.25f);
            }

            // Knurled ring (teeth)
            {
                const float rMid     = radius * 0.86f;
                const float toothLen = radius * 0.05f;
                drawKnurledRing (g, c, rMid, toothLen, 44, 0.075f);

                // darken under-knurl
                g.setColour (Colours::black.withAlpha (0.18f));
                g.drawEllipse (bounds.reduced (radius * 0.05f), 2.0f);
            }

            // Cavity
            auto cavity = bounds.reduced (radius * 0.16f);
            {
                ColourGradient cavGrad (knobDark().brighter (0.02f), c.x, cavity.getY(),
                                        Colours::black.brighter (0.02f), c.x, cavity.getBottom(), false);
                g.setGradientFill (cavGrad);
                g.fillEllipse (cavity);

                g.setColour (Colours::black.withAlpha (0.62f));
                g.drawEllipse (cavity, 1.15f);
            }

            // Dot ticks (ref style)
            drawTickDots (g, c, radius * 0.96f, jmax (1.2f, radius * 0.028f));

            // Subtle neon ring (thin!)
            auto ring = bounds.reduced (radius * 0.26f);
            drawNeonRingZ (g, ring, jmax (1.2f, radius * 0.028f), false);

            // Center cap
            auto cap = bounds.reduced (radius * 0.48f);
            {
                ColourGradient capGrad (knobMid().brighter (0.06f), c.x, cap.getY(),
                                        knobDark().darker (0.55f),  c.x, cap.getBottom(), false);
                g.setGradientFill (capGrad);
                g.fillEllipse (cap);

                auto capHi = cap.reduced (cap.getWidth() * 0.40f)
                                .translated (-cap.getWidth() * 0.12f, -cap.getHeight() * 0.16f);
                g.setColour (Colours::white.withAlpha (0.06f));
                g.fillEllipse (capHi);

                g.setColour (Colours::black.withAlpha (0.45f));
                g.drawEllipse (cap, 0.95f);
            }

            // Pointer line (white, thin)
            {
                const float ptrLen   = radius * 0.52f;
                const float ptrWidth = radius * 0.040f;
                const float ptrStart = radius * 0.10f;

                Path ptr;
                ptr.addRoundedRectangle (-ptrWidth * 0.5f, -(ptrStart + ptrLen),
                                         ptrWidth, ptrLen, ptrWidth * 0.40f);

                g.setColour (textBright().withAlpha (0.85f));
                g.fillPath (ptr, AffineTransform::rotation (angle).translated (c.x, c.y));
            }
        }
    }

    // ============================================================
    // Linear Slider — Output Ceiling style
    // ============================================================
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float, float,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        using namespace juce;

        if (style != Slider::LinearHorizontal)
        {
            LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, 0, 0, style, slider);
            return;
        }

        g.setImageResamplingQuality (Graphics::highResamplingQuality);

        auto r = Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
        const float slotH = jmax (10.0f, r.getHeight() * 0.70f);
        auto slot = Rectangle<float> (r.getX(), r.getCentreY() - slotH * 0.5f, r.getWidth(), slotH);

        // slot shadow
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillRoundedRectangle (slot.translated (1.0f, 1.6f), slotH * 0.42f);

        // slot base
        ColourGradient slotGrad (knobDark().brighter (0.02f), slot.getX(), slot.getY(),
                                 Colours::black,             slot.getX(), slot.getBottom(), false);
        g.setGradientFill (slotGrad);
        g.fillRoundedRectangle (slot, slotH * 0.42f);

        g.setColour (Colours::black.withAlpha (0.65f));
        g.drawRoundedRectangle (slot, slotH * 0.42f, 1.2f);

        // thumb
        const float px  = jlimit (r.getX() + slotH * 0.5f, r.getRight() - slotH * 0.5f, sliderPos);
        const float thW = jmax (12.0f, r.getHeight() * 0.46f);
        const float thH = jmax (16.0f, r.getHeight() * 0.90f);
        auto th = Rectangle<float> (px - thW * 0.5f, r.getCentreY() - thH * 0.5f, thW, thH);

        g.setColour (Colours::black.withAlpha (0.50f));
        g.fillRoundedRectangle (th.translated (1.0f, 1.5f), 4.0f);

        ColourGradient thGrad (bgMetalHi().brighter (0.16f), th.getX(), th.getY(),
                               bgMetalLo().darker (0.10f),   th.getRight(), th.getBottom(), false);
        g.setGradientFill (thGrad);
        g.fillRoundedRectangle (th, 4.0f);

        g.setColour (knobRing().withAlpha (0.90f));
        g.drawRoundedRectangle (th, 4.0f, 1.1f);

        // grip line
        g.setColour (Colours::black.withAlpha (0.40f));
        g.drawLine (th.getCentreX(), th.getY() + 4.0f, th.getCentreX(), th.getBottom() - 4.0f, 1.2f);

        // subtle magenta cue
        if (slider.isEnabled())
        {
            g.setColour (neonCore().withAlpha (0.10f));
            g.drawRoundedRectangle (slot.reduced (1.0f), slotH * 0.40f, 1.0f);
        }
    }

    // ============================================================
    // ToggleButton — LED orb style (Power)
    // ============================================================
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool, bool) override
    {
        using namespace juce;
        g.setImageResamplingQuality (Graphics::highResamplingQuality);

        auto b = button.getLocalBounds().toFloat();
        const float d = jmin (b.getWidth(), b.getHeight()) * 0.72f;
        auto led = Rectangle<float> (0, 0, d, d).withCentre (b.getCentre());
        const bool on = button.getToggleState();

        auto hole = led.expanded (d * 0.28f);

        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillEllipse (hole.translated (1.0f, 1.6f));

        ColourGradient holeGrad (knobDark().darker (0.35f), hole.getCentreX(), hole.getY(),
                                 Colours::black,           hole.getCentreX(), hole.getBottom(), false);
        g.setGradientFill (holeGrad);
        g.fillEllipse (hole);

        g.setColour (Colours::black.withAlpha (0.65f));
        g.drawEllipse (hole, 1.1f);

        if (on)
        {
            g.setColour (neonGlow().withAlpha (0.10f));
            g.fillEllipse (led.expanded (d * 0.45f));

            g.setColour (neonGlow().withAlpha (0.18f));
            g.fillEllipse (led.expanded (d * 0.22f));

            ColourGradient ledGrad (neonGlow(),   led.getCentreX(), led.getY(),
                                    accentPink(), led.getCentreX(), led.getBottom(), false);
            g.setGradientFill (ledGrad);
            g.fillEllipse (led);

            auto hotspot = led.reduced (d * 0.36f).translated (-d * 0.10f, -d * 0.14f);
            g.setColour (Colours::white.withAlpha (0.55f));
            g.fillEllipse (hotspot);

            g.setColour (neonCore().withAlpha (0.70f));
            g.drawEllipse (led, 1.1f);
        }
        else
        {
            ColourGradient ledGrad (knobDark().brighter (0.06f), led.getCentreX(), led.getY(),
                                    knobDark().darker (0.45f),  led.getCentreX(), led.getBottom(), false);
            g.setGradientFill (ledGrad);
            g.fillEllipse (led);

            g.setColour (knobRing().darker (0.25f).withAlpha (0.85f));
            g.drawEllipse (led, 1.1f);
        }
    }

    // ============================================================
    // TextButton — OS "2X" plate
    // ============================================================
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour&, bool over, bool down) override
    {
        using namespace juce;

        auto r = button.getLocalBounds().toFloat().reduced (1.0f);
        const float cr = 6.0f;

        g.setColour (Colours::black.withAlpha (0.45f));
        g.fillRoundedRectangle (r.translated (1.0f, 1.4f), cr);

        auto top = bgMetalHi().brighter (button.getToggleState() ? 0.10f : 0.06f);
        auto bot = bgMetalLo().darker   (button.getToggleState() ? 0.00f : 0.10f);

        if (down) { top = top.brighter (0.06f); bot = bot.brighter (0.02f); }

        ColourGradient grad (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (r, cr);

        g.setColour (knobRing().withAlpha (0.90f));
        g.drawRoundedRectangle (r, cr, 1.2f);

        if (button.getToggleState())
        {
            g.setColour (neonCore().withAlpha (0.18f));
            g.drawRoundedRectangle (r.reduced (1.3f), cr - 1.0f, 1.0f);

            if (over)
            {
                g.setColour (neonGlow().withAlpha (0.08f));
                g.fillRoundedRectangle (r.expanded (2.0f), cr + 1.0f);
            }
        }
        else if (over)
        {
            g.setColour (neonCore().withAlpha (0.08f));
            g.drawRoundedRectangle (r.reduced (1.2f), cr - 1.0f, 1.0f);
        }
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool, bool) override
    {
        using namespace juce;
        auto textCol = button.getToggleState() ? neonGlow() : textBright();

        if (! button.isEnabled())
            textCol = textCol.withAlpha (0.35f);

        g.setColour (textCol.withAlpha (button.getToggleState() ? 0.95f : 0.75f));
        g.setFont (getTextButtonFont (button, button.getHeight()));
        g.drawFittedText (button.getButtonText(), button.getLocalBounds(),
                          Justification::centred, 1);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override
    {
        const float sz = juce::jlimit (11.0f, 16.0f, buttonHeight * 0.42f);
       #if JUCE_MAJOR_VERSION >= 7
        return juce::Font (juce::FontOptions (sz)).withExtraKerningFactor (0.10f);
       #else
        return juce::Font (sz).withExtraKerningFactor (0.10f);
       #endif
    }

    juce::Font getLabelFont (juce::Label&) override
    {
       #if JUCE_MAJOR_VERSION >= 7
        return juce::Font (juce::FontOptions (12.0f)).withExtraKerningFactor (0.10f);
       #else
        return juce::Font (12.0f).withExtraKerningFactor (0.10f);
       #endif
    }
};
