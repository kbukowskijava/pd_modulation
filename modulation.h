/*
 * modulation.h
 *
 *  Created on: ....
 *      Author: ....
 */

#ifndef PROCESSING_MODULATION_H_
#define PROCESSING_MODULATION_H_

#include "processing/generator.h"

// Typy modulacji analogowej
typedef enum {
    MOD_AM,     // Modulacja amplitudy - https://en.wikipedia.org/wiki/Amplitude_modulation
    MOD_FM,     // Modulacja częstotliwości - https://en.wikipedia.org/wiki/Frequency_modulation
    MOD_PM      // Modulacja fazy - https://en.wikipedia.org/wiki/Phase_modulation
}MOD_ANALOG_Type_t;

// Typy modulacji binarnej
typedef enum {
    MOD_BASK,   // Modulacja dwuwartościowa z przełączaniem amplitudy - https://en.wikipedia.org/wiki/Amplitude-shift_keying
    MOD_BFSK,   // Modulacja dwuwartościowa z przełączaniem częstotliwości - https://en.wikipedia.org/wiki/Frequency-shift_keying
    MOD_BPSK,   // Modulacja dwuwartościowa z przełączaniem fazy - https://en.wikipedia.org/wiki/Phase-shift_keying
}MOD_BINARY_Type_t;

// Struktura konfiguracyjna modulatora analogowy
typedef struct{
    float k;                // Parametr modulacji: AM głębokość modulacji, FM dewiacja częstotliwości, PM dewiacja fazy
    MOD_ANALOG_Type_t type; // Typ modulacji
    float carrierAmplitude; // Amplituda fali nośnej [V]
    float carrierFrequency; // Częstotliwość fali nośnej - bazowa [Hz]
    OSC_Cfg_t * carrierOsc; // Oscylator fali nośnej - wbudowany w układ modulatora
}MOD_ANALOG_t;

/*
 * Dla modulacji AM: s(t) = Ac * [1 + k * m(t)] * cos(2*pi*f*t), pod warunkiem [1 + k * m(t)] >= 0
 *                   Jeżeli m(t) = Am *cos(2*pi*fm*t) to 'mikro' = k*Am - indeks modulacji od 0 do 1
 *                   Należy mieć na uwadze w związku z powyższym:
 *                   - ograniczyć maksymalną wartość ustawienia 'carrierAmplitude' do PDSP_CODEC_V = PDSP_CODEC_Vpp/2 = 3.3V/2;
 *                   - ograniczyć maksymalną wartość carrierFrequency <= Fs/4;               
 *                   - wykonać normalizację amplitudy sygnału modulującego message/PDSP_CODEC_V;
 *                   - wtedy k może przyjąć wartość od 0 do 1 lub wyrażone w [%] od 0% do 100%
 *
 * Dla modulacji FM: s(t) = Ac * cos(2*pi*f*t + k*2*pi*m(t)), pod warunkiem k > 0
 *                   Jeżeli m(t) = Am *cos(2*pi*fm*t) to s(t) = Ac * cos(2*pi*f*t + (k*Am/2*pi*Fm) * cos(2*pi*fm*t)),
 *                   gdzie Beta = (k*Am/2*pi*Fm) - indeks modulacji, a Fm to maksymalna częstotliwość sygnału m(t)
 *                   Przy normalizacji spełniony jest warunek Am <= 1, daje to k tożsame dewiacji częstotliwości dF;
 *                   Należy mieć na uwadze w zwiazku z powyższym:
 *                   - ograniczyć maksymalną wartość ustawienia 'carrierAmplitude' do PDSP_CODEC_V = PDSP_CODEC_Vpp/2 = 3.3V/2;
 *                   - ograniczyć maksymalną wartość k <= carrierFrequency/8;
 *                   - wykonać normalizację amplitudy sygnału modulującego message/PDSP_CODEC_V;
 *
 * Dla modulacji PM: s(t) = Ac *  cos(2*pi*f*t + k*m(t)), pod warunkiem PI/4 >= k >= 0
 *                   Należy mieć na uwadze w zwiazku z powyższym:
 *                   - ograniczyć maksymalną wartość ustawienia 'carrierAmplitude' do PDSP_CODEC_V = PDSP_CODEC_Vpp/2 = 3.3V/2;
 *                   - ograniczyć maksymalną wartość ustawienia 'k' do PI/4;
 *                   - wykonać normalizację amplitudy sygnału modulującego message/PDSP_CODEC_V;
 * */


// Struktura konfiguracyjna modulatora binarnego
typedef struct{
    float k;                // Parametr modulacji: ASK różnica amplitudy, FSK różnica częstotliwości, PSK różnica fazy
    uint32_t cnt;           // Licznik próbek wysłanych dla danego symbolu
    uint32_t cntMax;        // Maksymalna liczba próbek dla wysłania symbolu - najlepiej aby symbol składał się z pełnych okresów fali nośnej
    MOD_BINARY_Type_t type;      // Typ modulacji
    float carrierAmplitude; // Amplituda fali nośnej
    float carrierFrequency; // Częstotliwość fali nośnej - bazowa
    OSC_Cfg_t * carrierOsc; // Oscylator fali nośnej - wbudowany w układ modulatora
}MOD_BINARY_t;

/* Należy przyjąć:
 * ASK k: carrierAmplitude-(k/2) dla '0', carrierAmplitude+(k/2) dla '1', k <= carrierAmplitude/8, carrierFrequency <= Fs/4
 * FSK k: carrierFrequency-(k/2) dla '0', carrierFrequency+(k/2) dla '1', k <= carrierFrequency/8, carrierFrequency <= Fs/4
 * PSK k: (carrierFrequency*PDSP_2PI_DIV_FS)-(PDSP_PI/2) dla '0', (carrierFrequency*PDSP_2PI_DIV_FS)+(PDSP_PI/2) dla '1', carrierFrequency <= Fs/4
 *
 * Należy pamiętać aby:
 * - ograniczyć maksymalną wartość ustawienia 'carrierAmplitude' do PDSP_CODEC_V = PDSP_CODEC_Vpp/2 = 3.3V/2;
 */

// Inicjalizacja modulatora
void MOD_ANALOG_Init(MOD_ANALOG_t *hCfg, OSC_Cfg_t *hOscCfg, MOD_ANALOG_Type_t type,
                   float carrierAmplitude, float carrierFrequency, float k);
void MOD_BINARY_Init(MOD_BINARY_t *hCfg, OSC_Cfg_t *hOscCfg, MOD_BINARY_Type_t type,
                   float carrierAmplitude, float carrierFrequency, float k);

// Wyznaczenie wartości rzeczywistej próbki w wyrażonej w [V]
float MOD_ANALOG_GetValueF(MOD_ANALOG_t * hCfg, float message);
float MOD_BINARY_GetValueF(MOD_BINARY_t * hCfg, uint8_t symbol);

// Wyznaczenie wartości próbki w wyrażonej w bitach
channel_t MOD_ANALOG_GetSampleF(MOD_ANALOG_t * hCfg, float message);
channel_t MOD_BINARY_GetSampleF(MOD_BINARY_t * hCfg, uint8_t symbol);

uint8_t MOD_BINARY_GetSymbol(char ch);

#endif /* PROCESSING_MODULATION_H_ */
