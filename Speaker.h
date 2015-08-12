/**
 * @file    Speaker.h
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
#include <PropWare/runnable.h>

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
