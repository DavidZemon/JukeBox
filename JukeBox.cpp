/**
 * @file    JukeBox.cpp
 *
 * @author  David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <PropWare/pin.h>
#include <PropWare/printer/printer.h>
#include <PropWare/scanner.h>
#include <PropWare/runnable.h>
#include <PropWare/staticstringbuilder.h>
#include <PropWare/c++allocate.h>

const unsigned int MIN_FREQUENCY      = 200;
const unsigned int MAX_FREQUENCY      = 20000;
const unsigned int CHANNELS           = 1;
const char         FAILURE_RESPONSE[] = "Enter a frequency between 200 and 20,000 (no comma). 0 for off";

class Speaker : public PropWare::Runnable {
    public:
        Speaker (const uint32_t *stack, const size_t stackSizeInBytes, const PropWare::Pin::Mask pinMask)
                : Runnable(stack, stackSizeInBytes),
                  m_currentFrequency(0) {
            this->m_pin.set_mask(pinMask);
        }


        virtual void run () {
            unsigned int delay;

            this->m_pin.set_dir_out();
            while (1) {
                while (!this->m_currentFrequency);
                delay = SECOND / this->m_currentFrequency;
                this->m_pin.toggle();
                waitcnt(delay + CNT);
            }
        }

        void set_frequency (const unsigned int frequency) {
            this->m_currentFrequency = frequency;
        }

    private:
        PropWare::Pin         m_pin;
        volatile unsigned int m_currentFrequency;
};

class FrequencyDude : public PropWare::Comparator<unsigned int> {
    public:
        FrequencyDude () { }

        virtual bool valid (const unsigned int *lhs) const {
            return 0 == *lhs || (MIN_FREQUENCY <= *lhs && *lhs <= MAX_FREQUENCY);
        }
};

const FrequencyDude frequencyDude;

void start_speakers (Speaker **speakers, const size_t channels) {
    uint32_t          *stack;
    for (unsigned int i = 0; i < channels; ++i) {
        size_t stackSize = 64 * sizeof(uint32_t);
        stack = (uint32_t *) malloc(stackSize);
        speakers[i] = new Speaker(stack, stackSize, (PropWare::Port::Mask) (PropWare::Port::P16 << i));
        Speaker::invoke(*speakers[i]);
    }
    waitcnt(MILLISECOND + CNT);
}

void fill_buffer (unsigned int *frequencyBuffers[], const unsigned int notesPerChannel) {
    char                          inputPromptBuffer[512];
    PropWare::StaticStringBuilder inputPrompt(inputPromptBuffer);
    PropWare::Printer             inputPromptPrinter(&inputPrompt);
    unsigned int                  frequency;

    for (unsigned int note = 0; note < notesPerChannel; ++note) {
        for (unsigned int channel = 0; channel < CHANNELS; ++channel) {
            inputPrompt.clear();
            inputPromptPrinter << "Please input the next frequency for channel " << channel << ": ";
            pwIn.input_prompt(inputPromptBuffer, FAILURE_RESPONSE, &frequency, frequencyDude);
            frequencyBuffers[channel][note] = frequency;
        }
    }
}

int main () {
    Speaker *speakers[CHANNELS];
    start_speakers(speakers, CHANNELS);

    int          userInput;
    unsigned int *frequencyBuffers[CHANNELS];
    while (1) {
        pwIn.input_prompt("How many notes to your song?\n>>> ", "Enter a non-negative integer", &userInput,
                          NON_NEGATIVE_COMP);
        const unsigned int notesPerChannel = (unsigned int) userInput;
        if (userInput) {
            for (unsigned int i = 0; i < CHANNELS; ++i)
                frequencyBuffers[i] = (unsigned int *) malloc(sizeof(**frequencyBuffers) * notesPerChannel);

            fill_buffer((unsigned int **) frequencyBuffers, notesPerChannel);

            for (unsigned int i = 0; i < notesPerChannel; ++i) {
                for (unsigned int channel = 0; channel < CHANNELS; ++channel)
                    speakers[channel]->set_frequency(frequencyBuffers[channel][i]);
                waitcnt(SECOND/2 + CNT);
            }

            for (unsigned int i = 0; i < CHANNELS; ++i)
                free(frequencyBuffers[i]);
        }

        // Song is done, stop the speakers
        for (unsigned int channel = 0; channel < CHANNELS; ++channel)
            speakers[channel]->set_frequency(0);
    }
}
