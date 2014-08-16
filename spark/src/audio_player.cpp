/**

  Simple audio streaming library for the Spark Core
  Copyright (C) 2014 Nicolas Cortot
  https://github.com/ncortot/spark-audio-streaming

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "audio_player.h"
#include "audio_sample.h"

#include "stm32_it.h"

#define AUDIO_FREQUENCY 44100

AudioPlayer * volatile _player = NULL;
uint16_t volatile _loop = 0;
bool (* volatile _callback)(bool);
void TIM2_Audio_Interrupt_Handler(void);

uint8_t * volatile _b_start;
uint8_t * volatile _b_half;
uint8_t * volatile _b_end;
uint8_t * volatile _offset;


AudioPlayer::AudioPlayer()
{
}


AudioPlayer::~AudioPlayer()
{
}


bool AudioPlayer::available()
{
    return (_player == NULL);
}


void AudioPlayer::play(uint8_t *buffer, size_t size)
{
    _player = this;
    _callback = NULL;
    _loop = 1;
    _play(buffer, size);
}


void AudioPlayer::play(uint8_t *buffer, size_t size, bool (*callback)(bool))
{
    _player = this;
    _callback = callback;
    _loop = 0;
    _play(buffer, size);
}


void AudioPlayer::repeat(uint8_t *buffer, size_t size, uint16_t count)
{
    _player = this;
    _callback = NULL;
    _loop = count;
    _play(buffer, size);
}


void AudioPlayer::beep(uint16_t millis)
{
    uint16_t loop_count = AUDIO_FREQUENCY / SAMPLE_SIZE * millis / 1000;
    repeat((uint8_t *) audio_sample, SAMPLE_SIZE, loop_count);
    delay(millis);
}


void AudioPlayer::_play(uint8_t *buffer, size_t size)
{
    _b_start = buffer;
    _b_half = _b_start + (size / 2);
    _b_end = _b_start + size;

    _offset = _b_start;

    // Connect the timer interrupt handler
    Wiring_TIM2_Interrupt_Handler = TIM2_Audio_Interrupt_Handler;

    _start_timer();
}


/**
 * Configure Timer 2, channel 1 at 44kHz and PWM output on A0.
 *
 * Timer update will trigger DMA1 Channel 2.
 */
void AudioPlayer::_start_timer()
{
    // TIM Counter clock = 44kHz
    uint16_t TIM_ARR = (uint16_t)(SystemCoreClock / AUDIO_FREQUENCY);

    // Audio output on pin A0
    pinMode(A0, AF_OUTPUT_PUSHPULL);

    // AFIO clock enable
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // TIM clock enable
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // Time base configuration
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = TIM_ARR;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // PWM1 Mode configuration on channel 1
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 400;

    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);

    // Configure CC interrupt
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable Timer 2 interrupt
    TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

    // Start Timer 2
    TIM_Cmd(TIM2, ENABLE);
}


void TIM2_Audio_Interrupt_Handler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

        // Update the PWM duty
        TIM2->CCR1 = *_offset << 2;
        ++_offset;

        if (_offset == _b_half) {
            // Call the Half Tranfer callback
            if (_callback && !_callback(false)) {
                // Stop Timer 2
                TIM_Cmd(TIM2, DISABLE);
            }
        } else if (_offset == _b_end) {
            _offset = _b_start;
            // Call the Tranfer Complete callback
            if ((_callback && !_callback(true))  || _loop == 1) {
                // Stop Timer 2
                TIM_Cmd(TIM2, DISABLE);
            } else if (_loop != 0) {
                --_loop;
            }
        }
    }
}
