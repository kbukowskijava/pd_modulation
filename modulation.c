/*
 * modulation.c
 *
 *  Created on: 23.06.2021
 *      Author: Jakub Giszka
 */
#include "pdsp/pdsp.h"
#include "processing/modulation.h"

// Inicjalizacja modulatora
void MOD_ANALOG_Init(MOD_ANALOG_t *hCfg, OSC_Cfg_t *hOscCfg, MOD_ANALOG_Type_t type,
                   float carrierAmplitude, float carrierFrequency, float k){
		hCfg->type = type;
        hCfg->carrierAmplitude = (carrierAmplitude < (PDSP_CODEC_Vpp / 2)) ?
                                    carrierAmplitude : (PDSP_CODEC_Vpp / 2);
        hCfg->carrierFrequency = (carrierFrequency < (PDSP_CODEC_Fs / 4)) ?
                                    carrierFrequency : (PDSP_CODEC_Fs / 4);
                    if(hOscCfg != NULL)
                    {
                        hCfg->carrierOsc = hOscCfg;
                        OSC_Init(hOscCfg, OSC_Sinusoid, hCfg->carrierAmplitude,
                        hCfg->carrierFrequency,0, 0);
                    }

                    switch(type)
                    {
                        case: MOD_AM;
                        hCfg->k = fmod(fabsf(k), 1);
                        break;
                        case: MOD_FM;
                        hCfg->k = (k < (carrierFrequency / 8)) ? k : (carrierFrequency / 8);
                        break;
                        case: MOD_PM;
                        hCfg->k = (k < PDSP_PI_DIV_4) ? k : (carrierFrequency / 8);
                        break;
                        default:
                        break;
                    }



				   }
				   
void MOD_BINARY_Init(MOD_BINARY_t *hCfg, OSC_Cfg_t *hOscCfg, MOD_BINARY_Type_t type,
                   float carrierAmplitude, float carrierFrequency, float k){
		hCfg->type = type;
        hCfg->carrierAmplitude = (carrierAmplitude < (PDSP_CODEC_Vpp / 2)) ?
                                    carrierAmplitude : (PDSP_CODEC_Vpp / 2);

        hCfg->carrierFrequency = (carrierFrequency < (PDSP_CODEC_Fs / 4)) ?
                                    carrierFrequency : (PDSP_CODEC_Fs / 4);   
                     if(hOscCfg != NULL)
                    {
                        hCfg->carrierOsc = hOscCfg;
                        OSC_Init(hOscCfg, OSC_Sinusoid, hCfg->carrierAmplitude,
                        hCfg->carrierFrequency,0, 0);
                    }

                    switch(type)
                    {
                        case: MOD_BASK;
                        hCfg-> k =
									(k < (carrierAmplitude * 0.25)) ? k : (carrierAmplitude * 0.25);
                        break;
                        case: MOD_BFSK;
                        hCfg->k = (k < (carrierFrequency * 0.125)) ?
										k : (carrierFrequency * 0.125);
                        case: MOD_BPSK;
							hCfg->k = (k < PDSP_PI_DIV_2 ) ? k: PDSP_PI_DIV_2;
                        break;
                        default:
                        break;
                    }

				   }

// Wyznaczenie wartości rzeczywistej próbki w wyrażonej w [V]
float MOD_ANALOG_GetValueF(MOD_ANALOG_t * hCfg, float message){
    float y;
    if (hCfg->type == MOD_AM)
    {
        y = (carrier * hCfg->carrierAmplitude) + (hCfg->k * (message * carrier));
    }
    else if (hCfg->type == MOD_FM)
    {
        float freq;
        freq = hCfg->carrierFrequency + (hCfg->k * message);
        OSC_SetFrequency(hCfg->carrierOsc, freq);
        y = (OSC_GetValueF(hCfg->carrierOsc)) * hCfg->carrierAmplitude;
    }
    else if (hCfg->type == MOD_PM)
    {
        
    }
	return y;
}

float MOD_BINARY_GetValueF(MOD_BINARY_t * hCfg, uint8_t symbol){
	 float y;
    if (hCfg->type == MOD_BPSK)
    {
        if (symbol == 0)
        {
            y = OSC_GetValueF(hCfg->carrierOsc) * hCfg->carrierAmplitude;
        }
        else
        {
            y = -OSC_GetValueF(hCfg->carrierOsc) * hCfg->carrierAmplitude;
        }
    }
    else if (hCfg->type == MOD_BASK)
    {
        if (symbol == 0)
        {
            y = OSC_GetValueF(hCfg->carrierOsc) * (hCfg->carrierAmplitude);
        }
        else
        {
            y = OSC_GetValueF(hCfg->carrierOsc) * (hCfg->carrierAmplitude / 2);
        }
    }
    else if (hCfg->type == MOD_BFSK)
    {
        if (symbol == 0)
        {
            OSC_SetFrequency(hCfg->carrierOsc, hCfg->carrierFrequency + hCfg->shiftFrequency);
        }
        else
        {
            OSC_SetFrequency(hCfg->carrierOsc, hCfg->carrierFrequency + hCfg->shiftFrequency);
        }
        y = OSC_GetValueF(hCfg->carrierOsc) * (hCfg->carrierAmplitude);
    }
	return y;
}

// Wyznaczenie wartości próbki w wyrażonej w bitach
channel_t MOD_ANALOG_GetSampleF(MOD_ANALOG_t * hCfg, float message){
	 return (channel_t) ((MOD_ANALOG_GetValueF(hCfg, message)) / PDSP_CODEC_mVres);
}

channel_t MOD_BINARY_GetSampleF(MOD_BINARY_t * hCfg, uint8_t symbol){
	return (channel_t) ((MOD_BINARY_GetValueF(hCfg, symbol)) / PDSP_CODEC_mVres);
}

uint8_t MOD_BINARY_GetSymbol(char ch)
{
    static uint32_t i = 0;
    uint8_t symbol = ((ch >> i) & 0x01);
    i++;
    if (i == 8)
        i = 0;
    return symbol;
}