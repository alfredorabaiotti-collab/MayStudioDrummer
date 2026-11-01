#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "DrumTypes.h"
#include <algorithm>
#include <cmath>

namespace may
{
// Kit di batteria sintetico interno, per ascoltare l'anteprima senza altri strumenti.
class DrumSynth
{
public:
    void prepare (double sr)
    {
        sampleRate = sr;
        for (auto& v : voices) v.active = false;
        pending.clear();
        pending.reserve (512);
    }

    // Da chiamare durante processBlock, prima di render()
    void trigger (int piece, int vel, int sampleOffset)
    {
        if (pending.size() < pending.capacity()) pending.push_back ({ piece, vel, sampleOffset });
    }

    void render (juce::AudioBuffer<float>& buffer, float gain)
    {
        const int n = buffer.getNumSamples();
        std::sort (pending.begin(), pending.end(), [] (const Trig& a, const Trig& b) { return a.offset < b.offset; });
        size_t next = 0;
        auto* L = buffer.getWritePointer (0);
        auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

        for (int i = 0; i < n; ++i)
        {
            while (next < pending.size() && pending[next].offset <= i)
            {
                start (pending[next].piece, pending[next].vel);
                ++next;
            }
            float l = 0.0f, r = 0.0f;
            for (auto& v : voices)
            {
                if (! v.active) continue;
                const float s = v.tick (sampleRate, rng);
                l += s * v.panL;
                r += s * v.panR;
            }
            l = std::tanh (l * gain);
            r = std::tanh (r * gain);
            L[i] += l;
            if (R != nullptr) R[i] += r; else L[i] += r;
        }
        pending.clear();
    }

private:
    struct Trig { int piece, vel, offset; };

    struct Voice
    {
        bool active = false;
        int piece = 0;
        double age = 0;
        // tono
        int shape = 0;            // 0 sine, 1 triangolo, 2 quadra
        double phase = 0, phase2 = 0;
        float f0 = 0, f1 = 0, sweep = 0.1f, f2 = 0;
        float toneAmp = 0, toneAmp2 = 0, toneMul = 1, toneMul2 = 1;
        // rumore
        float noiseAmp = 0, noiseMul = 1, attack = 0;
        struct Biquad
        {
            float c[5] { 1, 0, 0, 0, 0 };
            float z1 = 0, z2 = 0;
            void set (const juce::IIRCoefficients& k) { for (int i = 0; i < 5; ++i) c[i] = k.coefficients[i]; z1 = z2 = 0; }
            float process (float x)
            {
                const float y = c[0] * x + z1;
                z1 = c[1] * x - c[3] * y + z2;
                z2 = c[2] * x - c[4] * y;
                return y;
            }
        } filt;
        bool clap = false;
        float panL = 0.7f, panR = 0.7f;
        float killMul = 1.0f;

        static float wave (int shape, double ph)
        {
            const double x = ph - std::floor (ph);
            switch (shape)
            {
                case 1:  return (float) (4.0 * std::abs (x - 0.5) - 1.0);
                case 2:  return x < 0.5 ? 1.0f : -1.0f;
                default: return (float) std::sin (x * juce::MathConstants<double>::twoPi);
            }
        }

        float tick (double sr, juce::Random& rng)
        {
            const double t = age / sr;
            float out = 0.0f;
            if (toneAmp > 1e-5f)
            {
                const double f = (f1 > 0 && t < sweep) ? f0 * std::pow ((double) f1 / f0, t / sweep) : (f1 > 0 ? f1 : f0);
                phase += f / sr;
                out += wave (shape, phase) * toneAmp;
                toneAmp *= toneMul;
                if (f2 > 0)
                {
                    phase2 += f2 / sr;
                    out += wave (shape, phase2) * toneAmp2;
                    toneAmp2 *= toneMul2;
                }
            }
            if (noiseAmp > 1e-5f)
            {
                float nz = filt.process (rng.nextFloat() * 2.0f - 1.0f);
                float env = noiseAmp;
                if (attack > 0 && t < attack) env *= (float) (t / attack);
                if (clap && t < 0.035) env *= (std::fmod (t, 0.011) < 0.004) ? 1.0f : 0.25f;
                out += nz * env;
                noiseAmp *= noiseMul;
            }
            toneAmp *= killMul; toneAmp2 *= killMul; noiseAmp *= killMul;
            age += 1.0;
            if (toneAmp < 1e-5f && noiseAmp < 1e-5f && toneAmp2 < 1e-5f) active = false;
            return out;
        }
    };

    float decayMul (float seconds) const
    {
        return (float) std::exp (std::log (0.0008) / (juce::jmax (0.005, (double) seconds) * sampleRate));
    }

    void start (int piece, int vel)
    {
        // l'hi-hat chiuso o a pedale ferma quello aperto
        if (piece == HHClosed || piece == HHPedal)
            for (auto& v : voices) if (v.active && v.piece == HHOpen) v.killMul = decayMul (0.03f);

        Voice* v = nullptr;
        for (auto& x : voices) if (! x.active) { v = &x; break; }
        if (v == nullptr)
        {
            v = &voices[0];
            for (auto& x : voices) if (x.age > v->age) v = &x;
        }

        *v = Voice();
        v->active = true;
        v->piece = piece;
        const float a = std::pow (juce::jlimit (1, 127, vel) / 127.0f, 1.6f);
        const double sr = sampleRate;

        auto tone = [&] (int shape, float f0, float f1, float amp, float dec, float sweep)
        { v->shape = shape; v->f0 = f0; v->f1 = f1; v->toneAmp = amp * a; v->toneMul = decayMul (dec); v->sweep = sweep; };
        auto tone2 = [&] (float f, float amp, float dec) { v->f2 = f; v->toneAmp2 = amp * a; v->toneMul2 = decayMul (dec); };
        auto noise = [&] (int type, float freq, float q, float amp, float dec)
        {
            v->noiseAmp = amp * a; v->noiseMul = decayMul (dec);
            freq = juce::jmin (freq, (float) (sr * 0.45));
            v->filt.set (type == 0 ? juce::IIRCoefficients::makeHighPass (sr, freq, q)
                                               : juce::IIRCoefficients::makeBandPass (sr, freq, q));
        };
        auto pan = [&] (float p) { v->panL = std::cos ((p + 1.0f) * juce::MathConstants<float>::pi * 0.25f); v->panR = std::sin ((p + 1.0f) * juce::MathConstants<float>::pi * 0.25f); };
        pan (0.0f);

        switch (piece)
        {
            case Kick:      tone (0, 160, 42, 1.1f, 0.38f, 0.11f); noise (1, 3000, 0.8f, 0.10f, 0.012f); break;
            case Snare:     noise (1, 2200, 0.7f, 0.75f, 0.2f); tone (1, 210, 160, 0.35f, 0.09f, 0.05f); break;
            case SideStick: noise (1, 2600, 4.0f, 0.8f, 0.04f); tone (2, 1700, 900, 0.07f, 0.025f, 0.02f); pan (-0.1f); break;
            case HHClosed:  noise (0, 7600, 0.8f, 0.34f, 0.05f); pan (0.35f); break;
            case HHOpen:    noise (0, 7000, 0.8f, 0.30f, 0.38f); pan (0.35f); break;
            case HHPedal:   noise (0, 6500, 0.8f, 0.22f, 0.035f); pan (0.35f); break;
            case Ride:      noise (1, 5800, 1.4f, 0.17f, 0.55f); tone (0, 3100, 0, 0.04f, 0.5f, 0.1f); pan (-0.4f); break;
            case RideBell:  tone (0, 2350, 0, 0.18f, 0.7f, 0.1f); tone2 (3520, 0.08f, 0.5f); noise (0, 6000, 1.0f, 0.06f, 0.2f); pan (-0.4f); break;
            case Crash:     noise (0, 4200, 0.6f, 0.42f, 1.3f); v->attack = 0.003f; pan (0.45f); break;
            case China:     noise (1, 3200, 1.2f, 0.45f, 1.0f); v->attack = 0.003f; pan (-0.5f); break;
            case TomHi:     tone (0, 260, 175, 0.75f, 0.35f, 0.2f); noise (1, 1800, 1.0f, 0.08f, 0.03f); pan (0.25f); break;
            case TomMid:    tone (0, 200, 130, 0.75f, 0.40f, 0.2f); noise (1, 1500, 1.0f, 0.08f, 0.03f); pan (-0.05f); break;
            case TomFloor:  tone (0, 140, 85, 0.80f, 0.50f, 0.22f); noise (1, 1200, 1.0f, 0.08f, 0.03f); pan (-0.35f); break;
            case Clap:      noise (1, 1300, 1.5f, 0.55f, 0.16f); v->clap = true; break;
            case Tambourine:noise (0, 8500, 1.0f, 0.28f, 0.12f); pan (0.55f); break;
            case Cowbell:   tone (2, 545, 0, 0.11f, 0.25f, 0.1f); tone2 (815, 0.09f, 0.2f); pan (0.3f); break;
            case Shaker:    noise (0, 6800, 1.0f, 0.18f, 0.05f); v->attack = 0.02f; pan (-0.55f); break;
            default:        v->active = false; break;
        }
    }

    double sampleRate = 44100.0;
    std::array<Voice, 48> voices;
    std::vector<Trig> pending;
    juce::Random rng;
};
} // namespace may
