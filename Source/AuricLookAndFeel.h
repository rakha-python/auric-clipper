#pragma once
#include <JuceHeader.h>
#include <cmath>

//==============================================================================
// AuricLookAndFeel.h — AURIC Z-CLIP Darkmode 3D (Magenta Luxe) — JUCE 8
// HEADER-ONLY (drop-in).
//==============================================================================

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

    // Screws
    static juce::Colour screwMetal()  { return juce::Colour (0xFF3A3A3A); }
    static juce::Colour screwDark()   { return juce::Colour (0xFF141414); }
    static juce::Colour screwHi()     { return juce::Colour (0xFF6A6A6A); }

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

    // ============================================================
    // ROTARY SLIDER
    // ============================================================
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        using namespace juce;

        g.setImageResamplingQuality (Graphics::highResamplingQuality);

        auto bounds = Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
        const float diam0 = jmin (bounds.getWidth(), bounds.getHeight());
        if (diam0 < 18.0f) return;

        const bool big = isBigKnob (slider, width, height);

        bounds = bounds.reduced (diam0 * (big ? 0.085f : 0.070f));
        const float markerMargin = big ? 0.0f : (jmin (bounds.getWidth(), bounds.getHeight()) * 0.21f);
        auto body = bounds.reduced (big ? (diam0 * 0.02f) : markerMargin);

        const float diam = jmin (body.getWidth(), body.getHeight());
        const float R    = diam * 0.5f;
        const auto  c    = body.getCentre();

        const float pos   = clamp01 (sliderPos);
        const float angle = rotaryStartAngle + pos * (rotaryEndAngle - rotaryStartAngle);

        drawKnobShadow (g, body, R, big);

        if (big)
        {
            drawBigKnob_ZClip (g, body, R, c, angle);
        }
        else
        {
            const float markerR = (jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f) * 0.965f;
            const float mSize   = jmax (2.1f, markerR * 0.070f);
            drawDiamondMarkers (g, bounds.getCentre(), markerR, mSize, 12);
            drawSmallKnobCap (g, body, R);
            drawMachinedPointerBar (g, c, R, angle);
            drawSmallCenterHub (g, c, R);
        }
    }

    // ============================================================
    // LINEAR SLIDER — Z-CLIP recessed slot + black fader thumb
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

        auto r = Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (1.0f);
        const float H = r.getHeight();
        const float W = r.getWidth();
        if (W < 40.0f || H < 16.0f) return;

        const float insetX = jlimit (12.0f, 40.0f, W * 0.11f);
        const float slotH  = jlimit (12.0f, 18.0f, H * 0.42f);
        const float slotR  = slotH * 0.55f;

        auto slot = Rectangle<float> (r.getX() + insetX,
                                      r.getCentreY() - slotH * 0.5f,
                                      r.getWidth() - insetX * 2.0f,
                                      slotH);

        auto outer = slot;
        const float pad   = jlimit (1.9f, 3.8f, slotH * 0.22f);
        auto inner        = slot.reduced (pad);
        const float innerR = jmax (0.0f, slotR - pad);

        drawZClipRecessedSlot (g, outer, inner, slotR, innerR, slider.isEnabled());

        const float px = jlimit (inner.getX(), inner.getRight(), sliderPos);
        const float thumbH = jlimit (22.0f, 42.0f, H * 0.98f);
        const float thumbW = jlimit (14.0f, 26.0f, thumbH * 0.58f);
        const float thumbR = thumbW * 0.50f;

        auto thumb = Rectangle<float> (0, 0, thumbW, thumbH)
                        .withCentre ({ px, outer.getCentreY() - slotH * 0.06f });

        thumb = thumb.withY (jlimit (r.getY() + 1.0f,
                                     r.getBottom() - thumbH - 1.0f,
                                     thumb.getY()));

        drawZClipFaderThumb (g, thumb, inner, thumbR);
    }

    // ============================================================
    // ToggleButton — recessed LED orb
    // ============================================================
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, bool, bool) override
    {
        using namespace juce;

        g.setImageResamplingQuality (Graphics::highResamplingQuality);

        auto b = button.getLocalBounds().toFloat();
        const float m = jmin (b.getWidth(), b.getHeight());
        if (m < 8.0f) return;

        const bool on = button.getToggleState();
        auto safe = b.reduced (jmax (1.0f, m * 0.06f));

        const float d = jlimit (10.0f, m, m * 0.60f);
        auto led = Rectangle<float> (0, 0, d, d).withCentre (safe.getCentre());

        auto clampExpand = [&] (Rectangle<float> base, float want) -> Rectangle<float>
        {
            const float maxL = base.getX()      - safe.getX();
            const float maxR = safe.getRight()  - base.getRight();
            const float maxT = base.getY()      - safe.getY();
            const float maxB = safe.getBottom() - base.getBottom();
            const float maxE = jmax (0.0f, jmin (jmin (maxL, maxR), jmin (maxT, maxB)));
            const float e    = jmin (want, maxE);
            return base.expanded (e);
        };

        auto hole  = clampExpand (led, d * 0.55f);
        auto bezel = clampExpand (led, d * 0.16f);

        for (int i = 5; i >= 0; --i)
        {
            const float spread = d * (0.05f + 0.026f * (float) i);
            const float a      = 0.17f - 0.024f * (float) i;
            auto sh = clampExpand (hole, spread).translated (d * 0.03f, d * 0.07f);
            g.setColour (Colours::black.withAlpha (jmax (0.0f, a)));
            g.fillEllipse (sh);
        }

        {
            ColourGradient cav (Colours::black.withAlpha (0.92f),
                                hole.getCentreX(), hole.getY(),
                                knobDark().brighter (0.07f).withAlpha (0.97f),
                                hole.getCentreX(), hole.getBottom(), false);
            g.setGradientFill (cav);
            g.fillEllipse (hole);
            g.setColour (Colours::black.withAlpha (0.80f));
            g.drawEllipse (hole, 1.25f);
            auto lip = hole.reduced (d * 0.07f).translated (-d * 0.015f, -d * 0.025f);
            g.setColour (Colours::white.withAlpha (0.050f));
            g.drawEllipse (lip, 1.15f);
        }

        {
            ColourGradient rg (bgMetalHi().brighter (0.30f).withAlpha (0.75f),
                               bezel.getX(), bezel.getY(),
                               bgMetalLo().darker (0.34f).withAlpha (0.98f),
                               bezel.getRight(), bezel.getBottom(), false);
            g.setGradientFill (rg);
            g.fillEllipse (bezel);
            g.setColour (Colours::black.withAlpha (0.62f));
            g.drawEllipse (bezel, 1.15f);
            g.setColour (Colours::white.withAlpha (0.055f));
            g.drawEllipse (bezel.reduced (d * 0.035f).translated (-d * 0.01f, -d * 0.01f), 1.0f);
        }

        if (on)
        {
            auto bloomBig   = clampExpand (led, d * 0.44f);
            auto bloomMid   = clampExpand (led, d * 0.24f);
            auto bloomSmall = clampExpand (led, d * 0.12f);

            g.setColour (neonGlow().withAlpha (0.09f));
            g.fillEllipse (bloomBig);
            g.setColour (neonGlow().withAlpha (0.16f));
            g.fillEllipse (bloomMid);
            g.setColour (neonGlow().withAlpha (0.18f));
            g.fillEllipse (bloomSmall);

            ColourGradient bodyG (neonGlow().withAlpha (0.96f),
                                  led.getCentreX(), led.getY(),
                                  accentPink().darker (0.30f).withAlpha (0.94f),
                                  led.getCentreX(), led.getBottom(), false);
            g.setGradientFill (bodyG);
            g.fillEllipse (led);

            auto core = led.reduced (d * 0.34f);
            ColourGradient coreG (Colours::white.withAlpha (0.26f),
                                  core.getCentreX(), core.getY(),
                                  neonCore().withAlpha (0.92f),
                                  core.getCentreX(), core.getBottom(), false);
            g.setGradientFill (coreG);
            g.fillEllipse (core);

            auto hotspot = led.reduced (d * 0.52f).translated (-d * 0.12f, -d * 0.16f);
            g.setColour (Colours::white.withAlpha (0.62f));
            g.fillEllipse (hotspot);

            g.setColour (neonCore().withAlpha (0.78f));
            g.drawEllipse (led, 1.15f);
        }
        else
        {
            ColourGradient offG (knobDark().brighter (0.08f).withAlpha (0.99f),
                                 led.getCentreX(), led.getY(),
                                 knobDark().darker (0.76f).withAlpha (0.99f),
                                 led.getCentreX(), led.getBottom(), false);
            g.setGradientFill (offG);
            g.fillEllipse (led);

            auto hi = led.reduced (d * 0.64f).translated (-d * 0.10f, -d * 0.12f);
            g.setColour (Colours::white.withAlpha (0.028f));
            g.fillEllipse (hi);

            g.setColour (knobRing().darker (0.35f).withAlpha (0.92f));
            g.drawEllipse (led, 1.05f);
        }
    }

    // ============================================================
    // TextButton
    // ============================================================
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour&, bool over, bool down) override
    {
        using namespace juce;

        auto r  = button.getLocalBounds().toFloat().reduced (1.0f);
        const float cr = jlimit (4.0f, 9.0f, r.getHeight() * 0.22f);
        const bool on  = button.getToggleState();

        g.setColour (Colours::black.withAlpha (0.62f));
        g.fillRoundedRectangle (r.translated (1.0f, 1.5f), cr);

        {
            auto top = bgMetalHi().brighter (on ? 0.10f : 0.05f);
            auto bot = bgMetalLo().darker   (on ? 0.00f : 0.12f);
            if (down) { top = top.brighter (0.05f); bot = bot.brighter (0.02f); }

            ColourGradient grad (top.withAlpha (0.92f), r.getX(), r.getY(),
                                 bot.withAlpha (0.98f), r.getX(), r.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (r, cr);

            g.setColour (Colours::black.withAlpha (0.62f));
            g.drawRoundedRectangle (r, cr, 1.15f);

            g.setColour (Colours::white.withAlpha (0.05f));
            g.drawRoundedRectangle (r.reduced (0.9f).translated (-0.3f, -0.4f),
                                    jmax (0.0f, cr - 0.9f), 1.0f);
        }

        {
            auto inner = r.reduced (jmax (1.2f, r.getHeight() * 0.10f));
            const float pad = jmax (1.2f, r.getHeight() * 0.10f);
            const float icr = jmax (0.0f, cr - pad);

            g.setColour (Colours::black.withAlpha (0.35f));
            g.fillRoundedRectangle (inner.translated (0.6f, 0.9f), icr);

            ColourGradient ig (knobDark().brighter (0.05f).withAlpha (0.85f),
                               inner.getX(), inner.getY(),
                               Colours::black.withAlpha (0.88f),
                               inner.getX(), inner.getBottom(), false);
            g.setGradientFill (ig);
            g.fillRoundedRectangle (inner, icr);

            g.setColour (Colours::white.withAlpha (0.04f));
            g.drawRoundedRectangle (inner.reduced (0.8f).translated (-0.2f, -0.3f),
                                    jmax (0.0f, icr - 0.8f), 1.0f);
        }

        if (on)
        {
            g.setColour (neonCore().withAlpha (0.14f));
            g.drawRoundedRectangle (r.reduced (1.4f), jmax (0.0f, cr - 1.4f), 1.0f);
            if (over)
            {
                g.setColour (neonGlow().withAlpha (0.06f));
                g.fillRoundedRectangle (r.expanded (2.0f), cr + 1.0f);
            }
        }
        else if (over)
        {
            g.setColour (neonCore().withAlpha (0.06f));
            g.drawRoundedRectangle (r.reduced (1.3f), jmax (0.0f, cr - 1.3f), 1.0f);
        }
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        using namespace juce;
        auto textCol = button.getToggleState() ? neonGlow() : textBright();
        if (! button.isEnabled()) textCol = textCol.withAlpha (0.35f);
        g.setColour (textCol.withAlpha (button.getToggleState() ? 0.95f : 0.75f));
        g.setFont (getTextButtonFont (button, button.getHeight()));
        g.drawFittedText (button.getButtonText(), button.getLocalBounds(), Justification::centred, 1);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override
    {
        const float sz = juce::jlimit (11.0f, 16.0f, (float) buttonHeight * 0.42f);
        return juce::Font (sz).withExtraKerningFactor (0.10f);
    }

    juce::Font getLabelFont (juce::Label&) override
    {
        return juce::Font (12.0f).withExtraKerningFactor (0.10f);
    }

private:
    static float clamp01 (float v) { return juce::jlimit (0.0f, 1.0f, v); }

    static bool isBigKnob (juce::Slider& s, int w, int h)
    {
        const auto nm = s.getName().toLowerCase();
        if (nm.contains ("drive") || nm.contains ("input") || nm.contains ("output") || nm.contains ("main"))
            return true;
        const float d = (float) juce::jmin (w, h);
        return d >= 150.0f;
    }

    // ============================================================
    // SLOT helper
    // ============================================================
    static void drawZClipRecessedSlot (juce::Graphics& g,
                                       juce::Rectangle<float> outer,
                                       juce::Rectangle<float> inner,
                                       float outerR, float innerR,
                                       bool enabled)
    {
        using namespace juce;

        const float h = outer.getHeight();
        const float w = outer.getWidth();
        const float ox = h * 0.05f;
        const float oy = h * 0.14f;

        for (int i = 6; i >= 0; --i)
        {
            const float k      = (float) i / 6.0f;
            const float spread = h * (0.08f + 0.22f * k);
            const float a      = 0.14f * (0.30f + 0.70f * k);
            g.setColour (Colours::black.withAlpha (a));
            g.fillRoundedRectangle (outer.expanded (spread)
                                         .translated (ox * (0.25f + 0.55f * k),
                                                      oy * (0.45f + 0.55f * k)),
                                    outerR + spread);
        }

        {
            ColourGradient shell (bgMetalHi().withAlpha (0.16f),
                                  outer.getX(), outer.getY(),
                                  bgMetalLo().withAlpha (0.96f),
                                  outer.getX(), outer.getBottom(), false);
            g.setGradientFill (shell);
            g.fillRoundedRectangle (outer, outerR);
            g.setColour (Colours::black.withAlpha (0.84f));
            g.drawRoundedRectangle (outer, outerR, 1.10f);
            g.setColour (Colours::white.withAlpha (0.022f));
            g.drawRoundedRectangle (outer.reduced (0.9f).translated (-0.15f, -0.55f),
                                    jmax (0.0f, outerR - 0.9f), 1.0f);
            g.setColour (Colours::black.withAlpha (0.42f));
            g.drawRoundedRectangle (outer.reduced (0.9f).translated (0.0f, 0.55f),
                                    jmax (0.0f, outerR - 0.9f), 1.0f);
        }

        {
            g.setColour (Colours::black.withAlpha (0.76f));
            g.fillRoundedRectangle (inner.translated (0.60f, 1.15f), innerR);

            ColourGradient cav (Colours::black.withAlpha (0.985f),
                                inner.getX(), inner.getY(),
                                knobDark().brighter (0.04f).withAlpha (0.99f),
                                inner.getX(), inner.getBottom(), false);
            g.setGradientFill (cav);
            g.fillRoundedRectangle (inner, innerR);

            const float fadeW = jlimit (10.0f, 34.0f, w * 0.18f);

            ColourGradient leftFade (Colours::black.withAlpha (0.58f),
                                     inner.getX(), inner.getCentreY(),
                                     Colours::transparentBlack,
                                     inner.getX() + fadeW, inner.getCentreY(), false);
            g.setGradientFill (leftFade);
            g.fillRoundedRectangle (inner, innerR);

            ColourGradient rightFade (Colours::transparentBlack,
                                      inner.getRight() - fadeW, inner.getCentreY(),
                                      Colours::black.withAlpha (0.58f),
                                      inner.getRight(), inner.getCentreY(), false);
            g.setGradientFill (rightFade);
            g.fillRoundedRectangle (inner, innerR);

            g.setColour (Colours::black.withAlpha (0.72f));
            g.drawLine (inner.getX() + 3.0f, inner.getY() + 1.10f,
                        inner.getRight() - 3.0f, inner.getY() + 1.10f, 1.25f);

            {
                const float lineH = jlimit (1.2f, 1.9f, inner.getHeight() * 0.18f);
                const float yy    = inner.getBottom() - lineH - jlimit (0.7f, 1.7f, inner.getHeight() * 0.10f);
                auto line = Rectangle<float> (inner.getX() + 4.5f, yy, inner.getWidth() - 9.0f, lineH);
                Path p;
                p.addRoundedRectangle (line, jmax (0.0f, innerR - 1.10f));
                g.setColour (Colours::white.withAlpha (0.16f));
                g.fillPath (p);
                g.setColour (Colours::white.withAlpha (0.030f));
                g.strokePath (p, PathStrokeType (1.6f, PathStrokeType::curved, PathStrokeType::rounded));
            }

            g.setColour (Colours::black.withAlpha (0.74f));
            g.drawRoundedRectangle (inner, innerR, 1.0f);
            g.setColour (Colours::white.withAlpha (0.018f));
            g.drawRoundedRectangle (inner.reduced (0.9f).translated (-0.15f, -0.30f),
                                    jmax (0.0f, innerR - 0.9f), 1.0f);
        }

        if (enabled)
        {
            g.setColour (neonCore().withAlpha (0.010f));
            g.drawRoundedRectangle (inner.reduced (1.2f), jmax (0.0f, innerR - 1.2f), 1.0f);
        }
    }

    // ============================================================
    // THUMB helper
    // ============================================================
    static void drawZClipFaderThumb (juce::Graphics& g,
                                     juce::Rectangle<float> thumb,
                                     juce::Rectangle<float> innerSlot,
                                     float r)
    {
        using namespace juce;

        const float tw = thumb.getWidth();
        const float th = thumb.getHeight();
        const float rr = jmax (r, tw * 0.48f);

        {
            auto contact = Rectangle<float> (thumb.getX() - 1.2f,
                                             innerSlot.getY() - 0.6f,
                                             thumb.getWidth() + 2.4f,
                                             innerSlot.getHeight() + 1.2f);
            g.setColour (Colours::black.withAlpha (0.26f));
            g.fillRoundedRectangle (contact, jmax (0.0f, innerSlot.getHeight() * 0.48f));
        }

        g.setColour (Colours::black.withAlpha (0.82f));
        g.fillRoundedRectangle (thumb.translated (1.05f, 1.85f), rr);
        g.setColour (Colours::black.withAlpha (0.30f));
        g.fillRoundedRectangle (thumb.translated (0.60f, 1.10f), rr);

        {
            ColourGradient body (knobMid().brighter (0.045f).withAlpha (0.99f),
                                 thumb.getX(), thumb.getY(),
                                 Colours::black.withAlpha (0.99f),
                                 thumb.getX(), thumb.getBottom(), false);
            g.setGradientFill (body);
            g.fillRoundedRectangle (thumb, rr);

            g.setColour (Colours::black.withAlpha (0.74f));
            g.drawRoundedRectangle (thumb, rr, 1.10f);

            g.setColour (Colours::white.withAlpha (0.020f));
            g.drawRoundedRectangle (thumb.reduced (0.9f).translated (-0.20f, -0.45f),
                                    jmax (0.0f, rr - 0.9f), 1.0f);

            auto spec = thumb.reduced (tw * 0.52f, th * 0.78f)
                             .translated (-tw * 0.06f, -th * 0.22f);
            g.setColour (Colours::white.withAlpha (0.065f));
            g.fillRoundedRectangle (spec, rr * 0.85f);
        }

        {
            const float y1 = thumb.getY() + th * 0.22f;
            const float y2 = thumb.getBottom() - th * 0.18f;
            const float spacing = jlimit (2.2f, 4.6f, tw * 0.22f);
            const float stroke  = jlimit (1.0f, 1.45f, tw * 0.12f);

            for (int i = -1; i <= 1; ++i)
            {
                const float gx = thumb.getCentreX() + (float) i * spacing;
                g.setColour (Colours::black.withAlpha (0.52f));
                g.drawLine (gx, y1, gx, y2, stroke);
                g.setColour (Colours::white.withAlpha (0.028f));
                g.drawLine (gx - 0.65f, y1 + 0.9f, gx - 0.65f, y2 - 0.9f, 1.0f);
            }
        }
    }

    // ============================================================
    // Shadow
    // ============================================================
    static void drawKnobShadow (juce::Graphics& g, juce::Rectangle<float> body, float R, bool big)
    {
        using namespace juce;

        const float ox = R * 0.020f;
        const float oy = R * 0.070f;
        const int   passes = 4;
        const float baseA  = big ? 0.095f : 0.10f;

        for (int i = passes; i >= 0; --i)
        {
            const float k = (float) i / (float) passes;
            const float spread = R * (0.030f + 0.020f * k);
            const float alpha  = jmax (0.0f, baseA * (0.55f - 0.42f * k));
            g.setColour (Colours::black.withAlpha (alpha));
            g.fillEllipse (body.expanded (spread).translated (ox * (0.55f + 0.55f * k),
                                                             oy * (0.55f + 0.55f * k)));
        }

        g.setColour (Colours::black.withAlpha (big ? 0.40f : 0.40f));
        g.fillEllipse (body.expanded (R * 0.008f).translated (ox * 0.22f, oy * 0.22f));
    }

    static void drawCrystalEdge (juce::Graphics& g, juce::Rectangle<float> r, float stroke)
    {
        using namespace juce;
        g.setColour (crystalEdge().withAlpha (0.72f));
        g.drawEllipse (r, stroke);
        auto hi = r.reduced (stroke * 0.9f).translated (-r.getWidth() * 0.010f, -r.getHeight() * 0.018f);
        g.setColour (Colours::white.withAlpha (0.040f));
        g.drawEllipse (hi, stroke * 0.55f);
    }

    // ============================================================
    // BIG KNOB (Z-CLIP)
    // ============================================================
    static void drawBigKnob_ZClip (juce::Graphics& g,
                                   juce::Rectangle<float> body,
                                   float R,
                                   juce::Point<float> c,
                                   float angleRad)
    {
        using namespace juce;

        {
            ColourGradient bezelGrad (bgMetalHi().brighter (0.06f), c.x, body.getY(),
                                      bgMetalLo().darker  (0.34f), c.x, body.getBottom(), false);
            g.setGradientFill (bezelGrad);
            g.fillEllipse (body);

            g.setColour (Colours::black.withAlpha (0.62f));
            g.drawEllipse (body, jmax (1.2f, R * 0.018f));

            g.setColour (Colours::white.withAlpha (0.030f));
            g.drawEllipse (body.reduced (R * 0.030f).translated (-R * 0.010f, -R * 0.020f),
                           jmax (1.0f, R * 0.012f));

            drawCrystalEdge (g, body.reduced (0.8f), jmax (1.6f, R * 0.020f));
        }

        auto channelOuter = body.reduced (R * 0.095f);
        auto channelInner = channelOuter.reduced (R * 0.060f);

        {
            ColourGradient cav (knobDark().brighter (0.02f), c.x, channelOuter.getY(),
                                Colours::black.withAlpha (0.96f), c.x, channelOuter.getBottom(), false);
            g.setGradientFill (cav);
            g.fillEllipse (channelOuter);

            g.setColour (Colours::black.withAlpha (0.78f));
            g.drawEllipse (channelOuter, jmax (1.1f, R * 0.014f));

            g.setColour (Colours::white.withAlpha (0.028f));
            g.drawEllipse (channelInner, jmax (1.0f, R * 0.010f));
        }

        auto ringRect = channelOuter.reduced (R * 0.145f);
        const float t = jmax (2.6f, R * 0.048f);
        drawNeonRingZClip (g, ringRect, t);

        auto innerRim = ringRect.reduced (t * 1.25f);
        {
            g.setColour (Colours::black.withAlpha (0.56f));
            g.drawEllipse (innerRim, jmax (1.0f, R * 0.010f));
            g.setColour (Colours::white.withAlpha (0.018f));
            g.drawEllipse (innerRim.reduced (1.2f), 1.0f);
        }

        {
            auto face = innerRim.reduced (R * 0.02f);
            drawConicalBrushedFace (g, face, angleRad, true);
            g.setColour (Colours::black.withAlpha (0.28f));
            g.drawEllipse (face, 1.1f);
        }

        drawMicroNotchReflection (g, c, R, angleRad);
        drawNeonSpillArcs (g, c, R);
    }

    // ============================================================
    // BIG FACE: rotating brushed sheen
    // ============================================================
    static void drawConicalBrushedFace (juce::Graphics& g,
                                        juce::Rectangle<float> face,
                                        float angleRad,
                                        bool big)
    {
        using namespace juce;

        const auto c = face.getCentre();
        const float R = face.getWidth() * 0.5f;

        Path clip; clip.addEllipse (face);
        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (clip);

        {
            ColourGradient base (knobMid().brighter (0.08f).withAlpha (0.96f),
                                 c.x - R * 0.10f, c.y - R * 0.18f,
                                 knobDark().darker (0.86f).withAlpha (0.98f),
                                 c.x + R * 0.10f, c.y + R * 0.20f, true);
            g.setGradientFill (base);
            g.fillEllipse (face);
        }

        const int lines = big ? 420 : 220;
        const float r1 = R * 0.02f;
        const float r2 = R * 0.985f;

        Random rng (0xC11F9A9u);

        for (int i = 0; i < lines; ++i)
        {
            const float tt = (float) i / (float) lines;
            const float baseA = tt * MathConstants<float>::twoPi;
            const float th = baseA + angleRad;

            float b = std::cos (baseA - MathConstants<float>::halfPi);
            b = (b + 1.0f) * 0.5f;
            b = std::pow (b, 0.75f);

            const float wob = (rng.nextFloat() - 0.5f) * 0.0045f;
            const float alpha = (big ? 0.010f : 0.014f) + b * (big ? 0.050f : 0.060f) + wob;

            const float x1 = c.x + std::cos (th) * r1;
            const float y1 = c.y + std::sin (th) * r1;
            const float x2 = c.x + std::cos (th) * r2;
            const float y2 = c.y + std::sin (th) * r2;

            g.setColour (Colours::white.withAlpha (jlimit (0.0f, big ? 0.090f : 0.105f, alpha)));
            g.drawLine (x1, y1, x2, y2, big ? 0.95f : 0.80f);
        }

        for (int i = 0; i < lines / 2; ++i)
        {
            const float tt = (float) i / (float) (lines / 2);
            const float baseA = tt * MathConstants<float>::twoPi;
            const float th = baseA + angleRad + MathConstants<float>::pi;

            float d = std::cos (baseA + MathConstants<float>::halfPi);
            d = jmax (0.0f, d);
            d = std::pow (d, 1.45f);

            const float x1 = c.x + std::cos (th) * (R * 0.03f);
            const float y1 = c.y + std::sin (th) * (R * 0.03f);
            const float x2 = c.x + std::cos (th) * (R * 0.985f);
            const float y2 = c.y + std::sin (th) * (R * 0.985f);

            g.setColour (Colours::black.withAlpha (d * (big ? 0.050f : 0.060f)));
            g.drawLine (x1, y1, x2, y2, big ? 0.80f : 0.60f);
        }

        {
            const float wedgeR1 = R * 0.18f;
            const float wedgeR2 = R * 1.05f;

            const float start = angleRad - MathConstants<float>::pi * 0.22f;
            const float end   = angleRad + MathConstants<float>::pi * 0.10f;

            Path wedge;
            wedge.addCentredArc (c.x, c.y, wedgeR2, wedgeR2, 0.0f, start, end, true);
            wedge.lineTo (c.x + std::cos (end) * wedgeR1, c.y + std::sin (end) * wedgeR1);
            wedge.addCentredArc (c.x, c.y, wedgeR1, wedgeR1, 0.0f, end, start, false);
            wedge.closeSubPath();

            ColourGradient wg (Colours::white.withAlpha (big ? 0.075f : 0.085f),
                               c.x - R * 0.30f, c.y - R * 0.35f,
                               Colours::transparentWhite,
                               c.x + R * 0.25f, c.y + R * 0.40f, false);
            g.setGradientFill (wg);
            g.fillPath (wedge);
        }
    }

    static void drawMicroNotchReflection (juce::Graphics& g, juce::Point<float> c, float R, float angleRad)
    {
        using namespace juce;

        const float len = R * 0.10f;
        const float w   = jmax (1.0f, R * 0.012f);

        Path p;
        p.addRoundedRectangle (-w * 0.5f, -(R * 0.64f), w, len, w);

        auto t = AffineTransform::rotation (angleRad).translated (c.x, c.y);

        g.setColour (Colours::black.withAlpha (0.35f));
        g.fillPath (p, t.translated (R * 0.012f, R * 0.018f));

        g.setColour (Colours::white.withAlpha (0.055f));
        g.fillPath (p, t);
    }

    // ============================================================
    // Neon ring
    // ============================================================
    static void drawNeonRingZClip (juce::Graphics& g, juce::Rectangle<float> ringRect, float t)
    {
        using namespace juce;

        g.setColour (neonGlow().withAlpha (0.055f));
        g.drawEllipse (ringRect.expanded (t * 1.8f), t * 2.0f);

        g.setColour (neonGlow().withAlpha (0.085f));
        g.drawEllipse (ringRect.expanded (t * 1.05f), t * 1.45f);

        g.setColour (neonCore().withAlpha (0.92f));
        g.drawEllipse (ringRect, t);

        g.setColour (accentGlow().withAlpha (0.22f));
        g.drawEllipse (ringRect.reduced (t * 0.62f), jmax (1.0f, t * 0.42f));

        {
            Path arc;
            const float cx = ringRect.getCentreX();
            const float cy = ringRect.getCentreY();
            const float rr = ringRect.getWidth() * 0.5f;

            arc.addCentredArc (cx, cy, rr, rr, 0.0f,
                               degreesToRadians (185.0f),
                               degreesToRadians (355.0f), true);

            g.setColour (accentGlow().withAlpha (0.18f));
            g.strokePath (arc, PathStrokeType (t * 0.70f, PathStrokeType::curved, PathStrokeType::rounded));

            g.setColour (Colours::white.withAlpha (0.10f));
            g.strokePath (arc, PathStrokeType (t * 0.28f, PathStrokeType::curved, PathStrokeType::rounded));
        }

        g.setColour (Colours::white.withAlpha (0.10f));
        g.drawEllipse (ringRect, jmax (0.9f, t * 0.20f));
    }

    static void drawNeonSpillArcs (juce::Graphics& g, juce::Point<float> c, float R)
    {
        using namespace juce;

        auto strokeArc = [&] (float startDeg, float endDeg, float radiusMul, float thickness, Colour col, float a)
        {
            Path p;
            const float rr = R * radiusMul;
            p.addCentredArc (c.x, c.y, rr, rr, 0.0f,
                             degreesToRadians (startDeg),
                             degreesToRadians (endDeg), true);
            g.setColour (col.withAlpha (a));
            g.strokePath (p, PathStrokeType (thickness, PathStrokeType::curved, PathStrokeType::rounded));
        };

        const float thick = jmax (2.0f, R * 0.022f);

        strokeArc (206.0f, 246.0f, 1.060f, thick * 3.2f, neonGlow(), 0.030f);
        strokeArc (294.0f, 334.0f, 1.060f, thick * 3.2f, neonGlow(), 0.030f);

        strokeArc (208.0f, 244.0f, 1.054f, thick * 1.8f, neonGlow(), 0.045f);
        strokeArc (296.0f, 332.0f, 1.054f, thick * 1.8f, neonGlow(), 0.045f);

        strokeArc (210.0f, 242.0f, 1.048f, thick * 0.85f, neonCore(), 0.12f);
        strokeArc (298.0f, 330.0f, 1.048f, thick * 0.85f, neonCore(), 0.12f);
    }

    // ============================================================
    // SMALL KNOB: diamond markers
    // ============================================================
    static void drawDiamondMarkers (juce::Graphics& g, juce::Point<float> c,
                                    float r, float size, int count)
    {
        using namespace juce;

        for (int i = 0; i < count; ++i)
        {
            const float a = (float) i / (float) count * MathConstants<float>::twoPi;

            const float x = c.x + std::cos (a) * r;
            const float y = c.y + std::sin (a) * r;

            float fade = 1.0f;
            const float s = std::sin (a);
            if (s > 0.15f) fade = jlimit (0.0f, 1.0f, 1.0f - (s - 0.15f) / 0.85f);
            fade = std::pow (fade, 1.25f);

            Path d;
            d.addRectangle (-size * 0.5f, -size * 0.5f, size, size);

            auto T = AffineTransform::rotation (MathConstants<float>::pi * 0.25f).translated (x, y);

            g.setColour (Colours::black.withAlpha (0.42f * fade));
            g.fillPath (d, T.translated (size * 0.10f, size * 0.16f));

            ColourGradient mg (textLight().withAlpha (0.20f * fade),
                               x - size * 0.45f, y - size * 0.45f,
                               Colours::black.withAlpha (0.70f * fade),
                               x + size * 0.45f, y + size * 0.45f, false);
            g.setGradientFill (mg);
            g.fillPath (d, T);

            g.setColour (Colours::white.withAlpha (0.030f * fade));
            g.strokePath (d, PathStrokeType (0.8f), T.translated (-size * 0.07f, -size * 0.10f));
        }
    }

    // ============================================================
    // SMALL KNOB: cap + glossy face
    // ============================================================
    static void drawSmallKnobCap (juce::Graphics& g, juce::Rectangle<float> body, float R)
    {
        using namespace juce;
        const auto c = body.getCentre();

        {
            for (int i = 3; i >= 0; --i)
            {
                const float spread = R * (0.018f + 0.014f * (float) i);
                const float alpha  = 0.10f - 0.02f * (float) i;
                g.setColour (Colours::black.withAlpha (jmax (0.0f, alpha)));
                g.fillEllipse (body.expanded (spread).translated (R * 0.018f, R * 0.035f));
            }

            ColourGradient bezel (bgMetalHi().brighter (0.06f).withAlpha (0.92f),
                                  c.x - R * 0.25f, body.getY(),
                                  bgMetalLo().darker (0.62f).withAlpha (0.99f),
                                  c.x + R * 0.25f, body.getBottom(), false);
            g.setGradientFill (bezel);
            g.fillEllipse (body);

            g.setColour (Colours::black.withAlpha (0.82f));
            g.drawEllipse (body, jmax (1.3f, R * 0.032f));
        }

        auto channel = body.reduced (R * 0.12f);
        {
            ColourGradient channelGrad (Colours::black.withAlpha (0.94f),
                                        c.x, channel.getY(),
                                        knobDark().brighter (0.04f).withAlpha (0.96f),
                                        c.x, channel.getBottom(), false);
            g.setGradientFill (channelGrad);
            g.fillEllipse (channel);

            g.setColour (Colours::black.withAlpha (0.74f));
            g.drawEllipse (channel, jmax (1.0f, R * 0.020f));

            g.setColour (Colours::white.withAlpha (0.020f));
            g.drawEllipse (channel.reduced (R * 0.05f).translated (-R * 0.01f, -R * 0.02f), 1.0f);
        }

        auto innerCap = body.reduced (R * 0.22f);
        {
            g.setColour (Colours::black.withAlpha (0.48f));
            g.fillEllipse (innerCap.translated (R * 0.015f, R * 0.022f));

            ColourGradient capGrad (bgMetalHi().brighter (0.10f).withAlpha (0.94f),
                                    c.x - R * 0.22f, innerCap.getY() - R * 0.08f,
                                    bgMetalLo().darker (0.52f).withAlpha (0.99f),
                                    c.x + R * 0.16f, innerCap.getBottom() + R * 0.06f, false);
            g.setGradientFill (capGrad);
            g.fillEllipse (innerCap);

            g.setColour (Colours::black.withAlpha (0.70f));
            g.drawEllipse (innerCap, jmax (1.0f, R * 0.020f));
        }

        auto face = body.reduced (R * 0.34f);
        {
            Path clip; clip.addEllipse (face);
            Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (clip);

            ColourGradient conc (knobMid().brighter (0.05f).withAlpha (0.92f),
                                 c.x - R * 0.18f, c.y - R * 0.22f,
                                 Colours::black.withAlpha (0.99f),
                                 c.x + R * 0.12f, c.y + R * 0.30f, true);
            g.setGradientFill (conc);
            g.fillEllipse (face);

            ColourGradient vig (Colours::transparentBlack,
                                c.x, c.y,
                                Colours::black.withAlpha (0.42f),
                                c.x, c.y + face.getHeight() * 0.55f, true);
            g.setGradientFill (vig);
            g.fillEllipse (face);

            auto gloss = face.reduced (R * 0.18f, R * 0.52f)
                             .translated (-R * 0.14f, -R * 0.22f);
            ColourGradient glossGrad (Colours::white.withAlpha (0.11f),
                                      gloss.getCentreX(), gloss.getY(),
                                      Colours::transparentWhite,
                                      gloss.getCentreX(), gloss.getBottom(), false);
            g.setGradientFill (glossGrad);
            g.fillEllipse (gloss);

            auto gloss2 = face.reduced (R * 0.55f, R * 0.70f)
                              .translated (-R * 0.08f, -R * 0.12f);
            g.setColour (Colours::white.withAlpha (0.06f));
            g.fillEllipse (gloss2);
        }

        g.setColour (Colours::black.withAlpha (0.62f));
        g.drawEllipse (face, jmax (1.0f, R * 0.018f));
    }

    // ============================================================
    // SMALL KNOB: machined pointer BAR
    // ============================================================
    static void drawMachinedPointerBar (juce::Graphics& g, juce::Point<float> c, float R, float angleRad)
    {
        using namespace juce;

        const float barLen = R * 0.64f;
        const float barW   = jmax (1.8f, R * 0.105f);
        const float barR   = barW * 0.55f;
        const float yStart = R * 0.14f;

        Path bar;
        bar.addRoundedRectangle (-barW * 0.5f, -(yStart + barLen), barW, barLen, barR);

        auto T = AffineTransform::rotation (angleRad).translated (c.x, c.y);

        g.setColour (Colours::black.withAlpha (0.62f));
        g.fillPath (bar, T.translated (R * 0.016f, R * 0.024f));
        g.setColour (Colours::black.withAlpha (0.30f));
        g.fillPath (bar, T.translated (R * 0.010f, R * 0.014f));

        Path barT = bar;
        barT.applyTransform (T);
        auto b = barT.getBounds();

        {
            ColourGradient mg (bgMetalLo().darker (0.10f).withAlpha (0.99f),
                               b.getX() - b.getWidth() * 0.2f, b.getCentreY(),
                               bgMetalHi().brighter (0.45f).withAlpha (0.99f),
                               b.getRight() + b.getWidth() * 0.2f, b.getCentreY(), false);

            Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (barT);
            g.setGradientFill (mg);
            g.fillRect (b.expanded (6.0f));
        }

        g.setColour (Colours::black.withAlpha (0.58f));
        g.strokePath (bar, PathStrokeType (jmax (1.0f, R * 0.012f),
                                           PathStrokeType::curved, PathStrokeType::rounded), T);

        g.setColour (Colours::white.withAlpha (0.18f));
        g.strokePath (bar, PathStrokeType (jmax (0.9f, R * 0.010f),
                                           PathStrokeType::curved, PathStrokeType::rounded),
                      T.translated (-R * 0.006f, -R * 0.004f));

        Path groove;
        groove.startNewSubPath (0.0f, -(yStart + barLen * 0.98f));
        groove.lineTo (0.0f, -(yStart + barLen * 0.10f));

        g.setColour (Colours::black.withAlpha (0.44f));
        g.strokePath (groove, PathStrokeType (jmax (1.0f, R * 0.014f),
                                              PathStrokeType::curved, PathStrokeType::rounded), T);

        g.setColour (Colours::white.withAlpha (0.09f));
        g.strokePath (groove, PathStrokeType (jmax (0.6f, R * 0.006f),
                                              PathStrokeType::curved, PathStrokeType::rounded),
                      T.translated (-R * 0.007f, -R * 0.004f));
    }

    static void drawSmallCenterHub (juce::Graphics& g, juce::Point<float> c, float R)
    {
        using namespace juce;

        const float hubR = R * 0.13f;
        auto hub = Rectangle<float> (0, 0, hubR * 2.0f, hubR * 2.0f).withCentre (c);

        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillEllipse (hub.translated (R * 0.010f, R * 0.014f));

        ColourGradient hg (bgMetalHi().brighter (0.18f).withAlpha (0.92f),
                           hub.getX(), hub.getY(),
                           bgMetalLo().darker (0.28f).withAlpha (0.99f),
                           hub.getRight(), hub.getBottom(), false);
        g.setGradientFill (hg);
        g.fillEllipse (hub);

        g.setColour (Colours::black.withAlpha (0.62f));
        g.drawEllipse (hub, jmax (1.0f, R * 0.010f));

        g.setColour (Colours::white.withAlpha (0.08f));
        g.drawEllipse (hub.reduced (hubR * 0.30f).translated (-hubR * 0.10f, -hubR * 0.12f), 1.0f);
    }
};
