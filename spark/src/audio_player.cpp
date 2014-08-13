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

void _write_dma(uint16_t *buffer, size_t size);


AudioPlayer::AudioPlayer()
{
}


AudioPlayer::~AudioPlayer()
{
}


void AudioPlayer::begin()
{
    _setup_dma();
    _setup_spi();
    _setup_timer();
    Wiring_TIM2_Interrupt_Handler = TIM2_Audio_Interrupt_Handler;
}


/**
 * Configure DMA 1 Channel 2 to transfer buffer data to SPI.
 */
void AudioPlayer::_setup_dma()
{
    NVIC_InitTypeDef NVIC_InitStructure;

    // Configure DMA interrupt
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable the DMA peripheral clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitTypeDef DMA_InitStructure;

    // Configure DMA1 without a memory address
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(SPI1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = 0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  // For repeat playing
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_DeInit(DMA1_Channel2);
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);

    // Enable Half Transfer and Transfer Complete interrupt
    DMA_ITConfig(DMA1_Channel2, DMA_IT_HT | DMA_IT_TC, ENABLE);
}


/**
 * Enable SPI1 in Tx mode with 16-bit words.
 */
void AudioPlayer::_setup_spi()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    pinMode(SCK, AF_OUTPUT_PUSHPULL);
    pinMode(MOSI, AF_OUTPUT_PUSHPULL);
    pinMode(MISO, INPUT);

    SPI_InitTypeDef SPI_InitStructure;

    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;

    // SPI Mode 3
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;

    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
}


/**
 * Configure Timer 2, channel 1 at 44kHz and PWM output on A0.
 *
 * Timer update will trigger DMA1 Channel 2.
 */
void AudioPlayer::_setup_timer()
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    // TIM Counter clock = 44kHz
    uint16_t TIM_ARR = (uint16_t)(SystemCoreClock / AUDIO_FREQUENCY) - 1;

    // TIM Channel Duty Cycle, must be enough to transmit an SPI frame
    uint16_t TIM_CCR = (uint16_t)(TIM_ARR / 4);

    // AFIO clock enable
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    pinMode(A0, AF_OUTPUT_PUSHPULL);

    // TIM clock enable
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // Time base configuration
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    TIM_TimeBaseStructure.TIM_Period = TIM_ARR;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // PWM1 Mode configuration on channel 1
    TIM_OCStructInit(&TIM_OCInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_Pulse = TIM_CCR;

    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM2, ENABLE);

    // Triger a DMA transfer on timer update
    TIM_DMACmd(TIM2, TIM_DMA_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;

    // Configure CC interrupt used to shut down the timer
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


bool AudioPlayer::available()
{
    return (_player == NULL);
}


void AudioPlayer::play(uint16_t *buffer, size_t size)
{
    _player = this;
    _callback = NULL;
    _loop = 1;
    _write_dma(buffer, size);
}


void AudioPlayer::play(uint16_t *buffer, size_t size, bool (*callback)(bool))
{
    _player = this;
    _callback = callback;
    _loop = 0;
    _write_dma(buffer, size);
}


void AudioPlayer::repeat(uint16_t *buffer, size_t size, uint16_t count)
{
    _player = this;
    _callback = NULL;
    _loop = count;
    _write_dma(buffer, size);
}


void AudioPlayer::beep(uint16_t millis)
{
    uint16_t loop_count = AUDIO_FREQUENCY / SAMPLE_SIZE * millis / 1000;
    repeat((uint16_t *) audio_sample, SAMPLE_SIZE, loop_count);
    delay(millis);
}


void _write_dma(uint16_t *buffer, size_t size)
{
    DMA_Cmd(DMA1_Channel2, DISABLE);

    DMA1_Channel2->CNDTR = (uint32_t) size;
    DMA1_Channel2->CMAR = (uint32_t) buffer;

    DMA_Cmd(DMA1_Channel2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}


extern "C" void DMA1_Channel2_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_HT2) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_HT2);

        // Call the Half Tranfer callback
        if (_callback && !_callback(false)) {
            // Stop playing at the end of the buffer
            _loop = 1;
        }
    }

    if (DMA_GetITStatus(DMA1_IT_TC2) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC2);

        // Call the Tranfer Complete callback
        if (_callback && !_callback(true)) {
            // Stop playing
            _loop = 1;
        }

        if (_loop == 1) {
            // Enable Timer 2 interrupt for shutdown
            TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

            // Disable DMA
            DMA_Cmd(DMA1_Channel2, DISABLE);
        } else {
            if (_loop != 0) {
                --_loop;
            }
        }
    }
}


void TIM2_Audio_Interrupt_Handler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

        // Stop Timer 2 interrupt
        TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);

        // Wait for the output to go HIGH then stop the timer
        while (!TIM_GetFlagStatus(TIM2, TIM_FLAG_CC1));
        TIM_Cmd(TIM2, DISABLE);

        // Ready
        _player = NULL;
    }
}
