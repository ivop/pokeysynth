#pragma once
#define REMEZ_BANDPASS       1
#define REMEZ_DIFFERENTIATOR 2
#define REMEZ_HILBERT        3

void REMEZ_CreateFilter(double h[], int numtaps, int numband, double bands[],
                        const double des[], const double weight[], int type);
