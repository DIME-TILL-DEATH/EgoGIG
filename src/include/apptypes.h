/*
 * apptypes.h
 *
 *  Created on: 17.07.2011
 *      Author: klen
 */

#ifndef __APPTYPES_H__
#define __APPTYPES_H__


// demo struct for stored data
#include "data_struct_storage.h"


typedef struct
{
  TickType_t OnTicks  ;
  TickType_t OffTicks ;
} __PACKED__ led_conf_t ;

typedef struct
{
  char*  dest_sound  ;
  char*  dest_click  ;
  size_t size ;
} target_t ;

typedef struct
{
  bool led_timer_state ;
  led_conf_t ledconf[LED_COUNT];
} __PACKED__ application_data_t ;

#include "crc32.h"

typedef data_struct_storage_t<application_data_t, crc32 > application_storage_t ;

extern application_storage_t application_storage ;





// Структура, описывающая заголовок WAV файла.
typedef struct
{
    // WAV-формат начинается с RIFF-заголовка:

    // Содержит символы "RIFF" в ASCII кодировке
    // (0x52494646 в big-endian представлении)
    char chunkId[4];

    // 36 + subchunk2Size, или более точно:
    // 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
    // Это оставшийся размер цепочки, начиная с этой позиции.
    // Иначе говоря, это размер файла - 8, то есть,
    // исключены поля chunkId и chunkSize.
    unsigned long chunkSize;

    // Содержит символы "WAVE"
    // (0x57415645 в big-endian представлении)
    char format[4];

    // Формат "WAVE" состоит из двух подцепочек: "fmt " и "data":
    // Подцепочка "fmt " описывает формат звуковых данных:

    // Содержит символы "fmt "
    // (0x666d7420 в big-endian представлении)
    char subchunk1Id[4];

    // 16 для формата PCM.
    // Это оставшийся размер подцепочки, начиная с этой позиции.
    unsigned long subchunk1Size;

    // Аудио формат, полный список можно получить здесь http://audiocoding.ru/wav_formats.txt
    // Для PCM = 1 (то есть, Линейное квантование).
    // Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
    unsigned short audioFormat;

    // Количество каналов. Моно = 1, Стерео = 2 и т.д.
    unsigned short numChannels;

    // Частота дискретизации. 8000 Гц, 44100 Гц и т.д.
    unsigned long sampleRate;

    // sampleRate * numChannels * bitsPerSample/8
    unsigned long byteRate;

    // numChannels * bitsPerSample/8
    // Количество байт для одного сэмпла, включая все каналы.
    unsigned short blockAlign;

    // Так называемая "глубиная" или точность звучания. 8 бит, 16 бит и т.д.
    unsigned short bitsPerSample;

    // Подцепочка "data" содержит аудио-данные и их размер.

    // Содержит символы "data"
    // (0x64617461 в big-endian представлении)
    char subchunk2Id[4];

    // numSamples * numChannels * bitsPerSample/8
    // Количество байт в области данных.
    unsigned long subchunk2Size;

    // Далее следуют непосредственно Wav данные.
} wave_header_t ;

// Структура, описывающая заголовок MIDI файла.
struct midi_mthd_header_t
{
    // Содержит символы "MThd" в ASCII кодировке
    char chunkId[4];


    // длинна данных заголовка (всегда 6)
    unsigned long chunkSize;

    uint16_t format ;
    uint16_t track_cont ;
    uint16_t division ;
    // Далее следуют непосредственно треки MIDI - заголовок MTrk + данные.
}  ;
struct midi_mtrk_header_t
{
    // Содержит символы "MTtr" в ASCII кодировке
    char chunkId[4];


    // длинна данных трека
    unsigned long chunkSize;
    // Далее следуют непосредственно данные.
}  ;



class wav_sample_t
{
public:
  int16_t left  ;
  int16_t right ;

  wav_sample_t& operator+ ( const wav_sample_t& val )
  {
    left  += val.left ;
    right += val.right ;
    return *this ;
  }

  inline void swap()
  {
    int16_t tmp = left ;
    left = right ;
    right = tmp ;
  }

} __PACKED__  ;

class dac_sample_t
{
public:
  int16_t left       ;
  int16_t zero_0_pad ;
  int16_t right      ;
  int16_t zero_1_pad ;

  dac_sample_t& operator = ( const wav_sample_t& val )
  {
    left = val.left ;
    right = val.right ;
    return *this ;
  }

} __PACKED__ ;

#endif /* APPTYPES_H_ */
