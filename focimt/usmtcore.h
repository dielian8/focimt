//---------------------------------------------------------------------------
#ifndef usmtcoreH
#define usmtcoreH
//---------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Source: usmtcore.h
// Module: focimt
// Moment tensor inversion core.
//
// Copyright (c) 2003-2015, Grzegorz Kwiatek.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include <list>
#include <math.h>
#include "inputdata.h"
#include "faultsolution.h"

//---------------------------------------------------------------------------
// USMTCORE
//  This is a part of USMT core (unix seismic moment tensor) application,
//  created by Pawel Wiejacz. The original Fortran 77 code was converted to
//  object C++ language without any profound improvements.
//
//  rev.
//   1.4.0 Removed some unnecessary variables.
//   1.2.0 Conditional #define USMTCORE_DEBUG directive included to prevent
//    the unnecessary standard debug output for Windows application.
//   1.1.0 Initial release for version 2.1.8 of Foci.
//---------------------------------------------------------------------------

// Additional debug information to standard output (switched off).
#undef USMTCORE_DEBUG

//---------------------------------------------------------------------------
#ifdef USMTCORE_DEBUG
#include <iostream>
#endif

void USMTCore(Taquart::NormType ANormType, int QualityType,
    Taquart::SMTInputData &InputData, int * const AThreadProgress);

void TransferSolution(Taquart::SolutionType AType,
    std::list<Taquart::FaultSolution> &ASolution);

void TransferSolution(Taquart::SolutionType AType,
    Taquart::FaultSolution &ASolution);

namespace Taquart {
  namespace UsmtCore {
    extern int NDAE[10];
    //extern char PS[MAXCHANNEL+1];
    extern double U[MAXCHANNEL + 1];
    //extern double ARR[MAXCHANNEL+1];
    extern double AZM[MAXCHANNEL + 1];
    extern double TKF[MAXCHANNEL + 1];
    extern double GA[MAXCHANNEL + 1][3 + 1];
    extern double A[MAXCHANNEL + 1][6 + 1];
    extern double FIJ[3 + 1][3 + 1][MAXCHANNEL + 1];
    extern double RM[6 + 1][3 + 1];
    extern double COV[6 + 1][6 + 1][3 + 1];
    extern int RO[MAXCHANNEL + 1];
    extern int VEL[MAXCHANNEL + 1];
    extern int R[MAXCHANNEL + 1];
    extern double UTH[MAXCHANNEL + 1];
    extern int N;
    extern double TROZ;
    extern double QSD;
    extern double QF;
    extern bool FSTCLL;
    extern int ICOND;
    extern Taquart::FaultSolution Solution[4];
    //extern int RPSTID[MAXCHANNEL+1];
    //extern int KNID[MAXCHANNEL+1];
    //extern int ACTIV[MAXCHANNEL+1];
    //extern char RPSTCP[MAXCHANNEL+1];
    extern int ISTA;
    extern int * ThreadProgress;

    void PROGRESS(double Progress, double Max);
    bool ANGGA(void);
    bool JEZ(void);
    void MOM1(int &IEXP, int QualityType);
    void GSOL(double x[], int &iexp);
    void f1(double X[], double &fff);
    void EIG3(double RM[], int ISTER, double E[]);
    void EIGGEN(double &E1, double &E2, double &E3, double &ALFA, double &BETA,
        double &GAMA);
    void EIGGEN_NEW(double e1, double e2, double e3, double &iso, double &clvd,
        double &dbcp);
    void GSOL5(double x[], int &IEXP);
    void GSOLA(double x[], int &IEXP);
    void XTRINF(int &ICOND, int LNORM, double Moment0[], double MomentErr[]);
    void f2(double x[], double &ffg);
    void POSTEP(int &METH, int &ITER, int &IND1);
    double DETR(double T[], double X);
    void RENUM(double &TRY, double &VAL, int ix[], int &j1, int &j2, int &j3,
        int &j4);
    void RDINP(Taquart::SMTInputData &InputData);
    void SIZEMM(int &IEXP);
    void MOM2(bool REALLY, int QualityType);
    void INVMAT(double A[][10], double B[][10], int NP);
    void FIJGEN(void);
    void BETTER(double &RMY, double &RMZ, double &RM0, double &RMT, int &ICOND);

    void LUBKSB2(double A[][10], int INDX[], double C[][10], double B[][10],
        int &NP, int jj);
    void LUDCMP(double B[][10], double A[][10], int INDX[], double &D, int &NP);
    void VEIG(double &s1, double &s2, double &s3, double &s4, double &s5,
        double &s6, double v[]);
    void ORT(double VE[], double VN[], double DE[], double DN[]);
    void Zero(double * Address, int C);

    double mw(double m0);
  }
}

//---------------------------------------------------------------------------
#endif
