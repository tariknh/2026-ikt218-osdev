
#ifndef UIAOS_FREQUENCIES_H
#define UIAOS_FREQUENCIES_H

// Note frequencies (in Hz)
#define C0 16
#define Cs0 17
#define D0 18
#define Ds0 19
#define E0 21
#define F0 22
#define Fs0 23
#define G0 25
#define Gs0 26
#define A0 27
#define As0 29
#define B0 31

#define C1 33
#define Cs1 35
#define D1 37
#define Ds1 39
#define E1 41
#define F1 44
#define Fs1 46
#define G1 49
#define Gs1 52
#define A1 55
#define As1 58
#define B1 62

#define C2 65
#define Cs2 69
#define D2 73
#define Ds2 78
#define E2 82
#define F2 87
#define Fs2 92
#define G2 98
#define Gs2 104
#define A2 110
#define As2 117
#define B2 123

#define C3 131
#define Cs3 139
#define D3 147
#define Ds3 156
#define E3 165
#define F3 175
#define Fs3 185
#define G3 196
#define Gs3 208
#define A3 220
#define As3 233
#define B3 247

#define C4 262
#define Cs4 277
#define D4 294
#define Ds4 311
#define E4 330
#define F4 349
#define Fs4 370
#define G4 392
#define Gs4 415
#define A4 440
#define As4 466
#define B4 494

#define C5 523
#define Cs5 554
#define D5 587
#define Ds5 622
#define E5 659
#define F5 698
#define Fs5 740
#define G5 784
#define Gs5 831
#define A5 880
#define As5 932
#define B5 988

#define C6 1047
#define Cs6 1109
#define D6 1175
#define Ds6 1245
#define E6 1319
#define F6 1397
#define Fs6 1480
#define G6 1568
#define Gs6 1661

#define A6 1760
#define As6 1865
#define B6 1976

#define C7 2093
#define Cs7 2217
#define D7 2349
#define Ds7 2489
#define E7 2637
#define F7 2794
#define Fs7 2960
#define G7 3136
#define Gs7 3322
#define A7 3520
#define As7 3729
#define B7 3951

#define C8 4186
#define Cs8 4435
#define D8 4699
#define Ds8 4978
#define E8 5274
#define F8 5588
#define Fs8 5919
#define G8 6272
#define Gs8 6645
#define A8 7040
#define As8 7459
#define B8 7902
#define C9 8372
#define Cs9 8870
#define D9 9397
#define Ds9 9956
#define E9 10548
#define F9 11175
#define Fs9 11839
#define G9 12543
#define Gs9 13290
#define A9 14080
#define As9 14917
#define B9 15804

#define A_SHARP4 466
#define G_SHARP4 415
#define R 0 // R (no sound) Rest note

// Orders the frequencies to be used by the piano.c
static const unsigned int freqs[] = {
    C0,  Cs0, D0,  Ds0, E0,  F0,  Fs0, G0,  Gs0, A0,  As0, B0,  C1,  Cs1, D1,
    Ds1, E1,  F1,  Fs1, G1,  Gs1, A1,  As1, B1,  C2,  Cs2, D2,  Ds2, E2,  F2,
    Fs2, G2,  Gs2, A2,  As2, B2,  C3,  Cs3, D3,  Ds3, E3,  F3,  Fs3, G3,  Gs3,
    A3,  As3, B3,  C4,  Cs4, D4,  Ds4, E4,  F4,  Fs4, G4,  Gs4, A4,  As4, B4,
    C5,  Cs5, D5,  Ds5, E5,  F5,  Fs5, G5,  Gs5, A5,  As5, B5,  C6,  Cs6, D6,
    Ds6, E6,  F6,  Fs6, G6,  Gs6, A6,  As6, B6,  C7,  Cs7, D7,  Ds7, E7,  F7,
    Fs7, G7,  Gs7, A7,  As7, B7,  C8,  Cs8, D8,  Ds8, E8,  F8,  Fs8, G8,  Gs8,
    A8,  As8, B8,  C9,  Cs9, D9,  Ds9, E9,  F9,  Fs9, G9,  Gs9, A9,  As9, B9,
};

#endif // UIAOS_FREQUENCIES_H
