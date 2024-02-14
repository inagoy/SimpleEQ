/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


void LookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);

    auto enabled = slider.isEnabled();

    g.setColour(enabled? Colour(133u, 115u, 161u) : Colours::grey);
    g.fillEllipse(bounds);

    g.setColour(enabled ? Colour(105u, 88u, 136u) : Colours::darkgrey);
    g.drawEllipse(bounds, 2.f);
    

    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        Path p;

        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY()- rswl->getTextBoxHeight() * 1.1);

        p.addRoundedRectangle(r, 2.f);

        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        g.setColour(enabled? Colours::black : Colours::grey);
        g.fillRect(r);

        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);

    }
}

void LookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    using namespace juce;

    if (auto* pb = dynamic_cast<PowerButton*>(&toggleButton))
    { 
        Path powerButton;

        auto bounds = toggleButton.getLocalBounds();
        auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();

        float ang = 30.f;
        size -= 6;

        powerButton.addCentredArc(r.getCentreX(), r.getCentreY(),
            size * 0.5, size * 0.5, 0.f,
            degreesToRadians(ang), degreesToRadians(360.f - ang), true);

        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());

        PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);
        auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colours::green;
    
        g.setColour(color);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 1);
    }
    else if (auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton))
    {
        auto color = ! toggleButton.getToggleState() ? Colours::dimgrey : Colours::green;
        g.setColour(color);

        auto bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);


        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
    }


}
//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();

    //g.setColour(Colours::red);
    //g.drawRect(getLocalBounds());

    //g.setColour(Colours::yellow);
    //g.drawRect(sliderBounds);

    getLookAndFeel().drawRotarySlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
        startAng,
        endAng,
        *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(Colours::grey);
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i=0; i<numChoices; i++)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);

        auto c= center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }

    
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= getTextHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}


juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();

    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param) )
    {
        float val = getValue();

        if (val>999.f)
        {
            val /= 1000.f;
            addK = true;
        }

        str = juce::String(val, (addK ? 2: 0));

    }
    else
    {
        jassertfalse; 
    }

    if (suffix.isNotEmpty())
    {
        str << " ";
        if (addK)
            str << "k";

        str << suffix;
    }

    return str;
}
//==============================================================================
ResponseCurveComponent::ResponseCurveComponent(SimpleEQAudioProcessor& p) : 
audioProcessor(p),
leftPathProducer(audioProcessor.leftChannelFifo),
rightPathProducer(audioProcessor.rightChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    updateChain();
    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
    
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);

}

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;

    while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            auto size = tempIncomingBuffer.getNumSamples();

            juce::FloatVectorOperations::copy(
                monoBuffer.getWritePointer(0, 0),
                monoBuffer.getReadPointer(0, size),
                monoBuffer.getNumSamples() - size
            );

            juce::FloatVectorOperations::copy(
                monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                tempIncomingBuffer.getReadPointer(0, 0),
                size
            );

            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);

        }
    }
    /*
  if there are FFT data buffers to pull
     if we can pull a buffer
         generate a path
  */
    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate / (double) fftSize;

    DBG("RES=" << sampleRate << " " << fftSize << " " << binWidth);

    while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if (leftChannelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }

    }

    /*
    while there are paths that can be pull
       display the most recent path
    */

    while (pathProducer.getNumPathsAvailable())
    {
        pathProducer.getPath(leftChannelFFTPath);
    }

}


void ResponseCurveComponent::timerCallback()
{
    if (shouldShowFFTAnalysis)
    {
        auto fftBounds = getAnalysisArea().toFloat();
        auto sampleRate = audioProcessor.getSampleRate();

        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }

    if (parametersChanged.compareAndSetBool(false, true))
    {
        updateChain();
    };

    repaint();
}

void ResponseCurveComponent::updateChain() 
{
    auto chainSettings = getChainSettings(audioProcessor.apvts);

    monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypass);
    monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypass);
    monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypass);

    auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

    auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
    auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
    updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutShape);
    updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutShape);

}
void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::whitesmoke);

    g.drawImage(background, getLocalBounds().toFloat());

    auto responseArea = getAnalysisArea();

    auto w = responseArea.getWidth();

    auto& lowCut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highCut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();


    std::vector<double> mags;
    mags.resize(w);

    for (int i = 0; i < w; ++i)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);
        if (!monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!monoChain.isBypassed<ChainPositions::LowCut>()){
            if (!lowCut.isBypassed<0>())
                mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowCut.isBypassed<1>())
                mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowCut.isBypassed<2>())
                mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowCut.isBypassed<3>())
                mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if (!monoChain.isBypassed<ChainPositions::HighCut>()) {
            if (!highCut.isBypassed<0>())
                mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highCut.isBypassed<1>())
                mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highCut.isBypassed<2>())
                mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highCut.isBypassed<3>())
                mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path responseCurve;

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
        {
            return jmap(input, -24.0, 24.0, outputMin, outputMax);
        };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    for (size_t i = 1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }
    
    if (shouldShowFFTAnalysis)
    {
        auto leftChannelFFTPath = leftPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

        g.setColour(Colours::blue);
        g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));
    
        auto rightChannelFFTPath = rightPathProducer.getPath();
        rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

        g.setColour(Colours::orange);
        g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
    }

    g.setColour(Colours::darkgrey);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 2.f);

    g.setColour(Colours::black);
    g.strokePath(responseCurve, PathStrokeType(2.f));


}


void ResponseCurveComponent::resized()
{
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);

    Graphics g(background);
    g.setColour(Colours::whitesmoke);
    g.fillAll();
    Array<float> freqs
    {
        20, 50, 100,
        200, 500, 1000,
        2000, 5000, 10000,
        20000
    };

    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    Array<float> xs;
    for (auto f : freqs)
    {
        auto normX = mapFromLog10(f, 20.f, 20000.f);
        xs.add(left + width * normX);
    }

    g.setColour(Colours::grey);
    for ( auto x: xs)
    {
       g.drawVerticalLine(x, top, bottom);
    }

    Array<float> gain
    {
        -24, -12, 0, 12, 24
    };

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        g.setColour(gDb == 0.f ? Colour(105u, 88u, 136u) : Colours::grey);
        g.drawHorizontalLine(y, left, right);
    }

    g.setColour(Colour(105u, 88u, 136u));
    const int fontHeight = 10;
    g.setFont(fontHeight);

    for (int i=0; i<freqs.size(); i++)
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;

        if (f > 999.f)
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if (addK)
            str << "k";
        str << "Hz";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);

        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

        String str;
        if (gDb > 0)
            str << "+";
        str << gDb;

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(getWidth() - textWidth);
        r.setCentre(r.getCentreX(), y);

        g.setColour(gDb == 0.f ? Colour(122u, 105u, 151u) : Colours::grey);

        g.drawFittedText(str, r, juce::Justification::centred, 1);

        str.clear();
        str << (gDb - 24.f);

        //Db Labels Left
        r.setX(1);
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);
        g.setColour(Colours::dodgerblue);
        g.drawFittedText(str, r, juce::Justification::centred, 1);


    }

}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();

    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);

    return bounds;
}

juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
}
//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
    peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
    peakQSlider(*audioProcessor.apvts.getParameter("Peak Q"), ""),
    lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    lowCutShapeSlider(*audioProcessor.apvts.getParameter("LowCut Shape"), "dB/Oct"),
    highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    highCutShapeSlider(*audioProcessor.apvts.getParameter("HighCut Shape"), "dB/Oct"),

    responseCurveComponent(audioProcessor),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQSliderAttachment(audioProcessor.apvts, "Peak Q", peakQSlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    lowCutShapeSliderAttachment(audioProcessor.apvts, "LowCut Shape", lowCutShapeSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    highCutShapeSliderAttachment(audioProcessor.apvts, "HighCut Shape", highCutShapeSlider),

    lowCutBypassAttachment(audioProcessor.apvts, "LowCut Bypass", lowCutBypassButton),
    highCutBypassAttachment(audioProcessor.apvts, "HighCut Bypass", highCutBypassButton),
    peakBypassAttachment(audioProcessor.apvts, "Peak Bypass", peakBypassButton),
    analyzerEnableAttachment(audioProcessor.apvts, "Analyzer Enable", analyzerEnableButton)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    peakFreqSlider.labels.add({ 0.f, "20Hz" });
    peakFreqSlider.labels.add({ 1.f, "20kHz" });
    lowCutFreqSlider.labels.add({ 0.f, "20Hz" });
    lowCutFreqSlider.labels.add({ 1.f, "20kHz" });
    highCutFreqSlider.labels.add({ 0.f, "20Hz" });
    highCutFreqSlider.labels.add({ 1.f, "20kHz" });

    peakQSlider.labels.add({ 0.f, "0.1" });
    peakQSlider.labels.add({ 1.f, "10" });
    peakGainSlider.labels.add({ 0.f, "-24dB" });
    peakGainSlider.labels.add({ 1.f, "+24dB" });

    lowCutShapeSlider.labels.add({ 0.f, "12" });
    lowCutShapeSlider.labels.add({ 1.f, "48" });
    highCutShapeSlider.labels.add({ 0.f, "12" });
    highCutShapeSlider.labels.add({ 1.f, "48" });


    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    lowCutBypassButton.setLookAndFeel(&lnf);
    highCutBypassButton.setLookAndFeel(&lnf);
    peakBypassButton.setLookAndFeel(&lnf);
    analyzerEnableButton.setLookAndFeel(&lnf);

    auto safePtr = juce::Component::SafePointer<SimpleEQAudioProcessorEditor>(this);
    peakBypassButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            auto bypassed = comp->peakBypassButton.getToggleState();
            comp->peakFreqSlider.setEnabled(!bypassed);
            comp->peakGainSlider.setEnabled(!bypassed);
            comp->peakQSlider.setEnabled(!bypassed);
        }
    };

    lowCutBypassButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            auto bypassed = comp->lowCutBypassButton.getToggleState();
            comp->lowCutFreqSlider.setEnabled(!bypassed);
            comp->lowCutShapeSlider.setEnabled(!bypassed);
        }
    };
    highCutBypassButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            auto bypassed = comp->highCutBypassButton.getToggleState();
            comp->highCutFreqSlider.setEnabled(!bypassed);
            comp->highCutShapeSlider.setEnabled(!bypassed);
        }
    };

    analyzerEnableButton.onClick = [safePtr]()
    {
        if (auto* comp = safePtr.getComponent())
        {
            auto enabled = comp->analyzerEnableButton.getToggleState();
            comp->responseCurveComponent.toggleAnalysisEnablement(enabled);
        }
    };

    setSize(600, 500);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
    lowCutBypassButton.setLookAndFeel(nullptr);
    highCutBypassButton.setLookAndFeel(nullptr);
    peakBypassButton.setLookAndFeel(nullptr);
    analyzerEnableButton.setLookAndFeel(nullptr);
}

//==============================================================================

void SimpleEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    using namespace juce;
    g.fillAll(Colours::whitesmoke);
}

void SimpleEQAudioProcessorEditor::resized()
{

    auto bounds = getLocalBounds();

    auto analyzerEnableArea = bounds.removeFromTop(25);
    analyzerEnableArea.setWidth(100);
    analyzerEnableArea.setX(5);
    analyzerEnableArea.removeFromTop(2);

    analyzerEnableButton.setBounds(analyzerEnableArea);

    bounds.removeFromTop(5);

    float hRatio = 30.f / 100.f;
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);

    responseCurveComponent.setBounds(responseArea);

    bounds.removeFromTop(10);
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);


    lowCutBypassButton.setBounds(lowCutArea.removeFromTop(25));
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutShapeSlider.setBounds(lowCutArea);

    highCutBypassButton.setBounds(highCutArea.removeFromTop(25));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutShapeSlider.setBounds(highCutArea);

    peakBypassButton.setBounds(bounds.removeFromTop(25));
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQSlider.setBounds(bounds);
}
 
std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQSlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutShapeSlider,
        &highCutShapeSlider,
        &responseCurveComponent,

        &lowCutBypassButton, 
        &highCutBypassButton, 
        &peakBypassButton, 
        &analyzerEnableButton
    };
}