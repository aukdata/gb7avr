#ifndef SPEAKER_HPP
#define SPEAKER_HPP

#define GB7_TIMER_USE_INVOKE

#include "port.hpp"
#include "timer.hpp"
#include "queue.hpp"

namespace gb7::sound
{
    enum class Tone: long
    {
        None = -1,
        C    = 3822,
        Cs   = 3677,
        D    = 3405,
        Ds   = 3214,
        E    = 3033,
        F    = 2863,
        Fs   = 2702,
        G    = 2551,
        Gs   = 2407,
        A    = 2272,
        As   = 2145,
        B    = 2024,
        Ch   = 1911,
    };

    template<class SpeakerPin>
    class speaker
    {
        struct Note
        {
            Tone tone;
            uint32_t length; // microseconds
        };

        queue<Note, 16> m_notes;
        Tone m_tone = Tone::None;
        uint32_t m_count = 0;
        uint32_t m_count_to = 0;

    public:
        inline void init()
        {
            gb7::timer::multitimer::init();

            using namespace gb7::timer::literals;
            gb7::timer::multitimer::invoke_every(100_ms, 0_ms, on_timer<SpeakerPin>, this);
        }

        inline void stop_note()
        {
            m_notes.clear();
            m_tone = Tone::None;
            m_count = 0;
            m_count_to = 0;
        }

        inline bool enqueue_note(Tone tone, uint32_t length)
        {
            return m_notes.push({ tone, length });
        }

        template<class SpeakerPin_>
        static void on_timer(void* d)
        {
            SpeakerPin_ pin;
            auto sp = static_cast<speaker<SpeakerPin_>*>(d);

            if(sp->m_count <= sp->m_count_to)
            {
                sp->m_count++;
                if (sp->m_tone != Tone::None)
                {
                    pin = !pin;
                }
            }
            else
            {
                if (!sp->m_notes.empty())
                {
                    Note note_temp;
                    sp->m_notes.pop(note_temp);

                    if (note_temp.tone == Tone::None)
                    {
                        sp->m_tone = Tone::None;
                        sp->m_count = 0;
                        sp->m_count_to = 0;

                        gb7::timer::multitimer::invoke_every(gb7::timer::literals::operator""_us(note_temp.length), 0, on_timer<SpeakerPin>, d);
                    }
                    else
                    {
                        sp->m_tone = note_temp.tone;
                        sp->m_count_to = 2 * note_temp.length / static_cast<long>(sp->m_tone);
                        sp->m_count = 0;

                        gb7::timer::multitimer::invoke_every(gb7::timer::literals::operator""_us(static_cast<long>(sp->m_tone) / 2), 0, on_timer<SpeakerPin>, d);
                    }
                }
                else
                {
                    sp->m_tone = Tone::None;
                    sp->m_count_to = 0;
                    sp->m_count = 0;

                    using namespace gb7::timer::literals;
                    gb7::timer::multitimer::invoke_in(100_ms, on_timer<SpeakerPin>, d);
                }
            }
        }
    };
} // namespace gb7::sound

#endif // SPEAKER_HPP
