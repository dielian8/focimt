//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <trilib/string.h>
#include <trilib/georoutines.h>
#include <tricairo/tricairo_meca.h>
//-----------------------------------------------------------------------------
#include "getopts.h"
#include "faultsolution.h"
#include "inputdata.h"
#include "usmtcore.h"
//-----------------------------------------------------------------------------

using namespace std;

// Default values.
bool DrawStations = true;
bool DrawAxes = true;
bool DrawCross = true;
bool DrawDC = true;
bool WulffProjection = false;
bool LowerHemisphere = true;

//-----------------------------------------------------------------------------
class FaultSolutions {
  public:
    char Type;
    int Channel;
    Taquart::FaultSolution FullSolution;
    Taquart::FaultSolution TraceNullSolution;
    Taquart::FaultSolution DoubleCoupleSolution;
};

//-----------------------------------------------------------------------------
void GenerateBallCairo(Taquart::TriCairo_Meca &Meca,
    std::vector<FaultSolutions> &FSList, Taquart::SMTInputData &InputData,
    Taquart::String Type);

//-----------------------------------------------------------------------------
bool Dispatch(Taquart::String &Input, Taquart::String &Chunk,
    Taquart::String delimiter) {
  if (Input.Pos(delimiter) == 0)
    return false;
  else {
    Chunk = Input.SubString(1, Input.Pos(delimiter) - 1);
    Input = Input.SubString(Input.Pos(delimiter) + 1, 10000);
    return true;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
  try {
    Taquart::String FilenameIn;
    Taquart::String FilenameOut;
    unsigned int N = 14;

    Options listOpts;
    int switchInt;
    listOpts.addOption("i", "input", "Full path to the input file", true);
    listOpts.addOption("o", "output", "Output file name (without extension)",
        true);
    listOpts.addOption("s", "solution",
        "Output solution type.                                \n\n"
            "    Arguments: [F][T][D] for the full, trace-null and double-couple solution.  \n"
            "    Default option is '-s D'. Combine options to get multiple solutions, e.g.  \n"
            "    '-s DFT' produces all three solutions at once.                             \n",
        true);
    listOpts.addOption("t", "type",
        "Output file type.                                    \n\n"
            "    Arguments: [NONE|PNG|SVG|PS|PDF] for different file types (only one can) be\n"
            "    specified as an output. The default value is '-t PNG'                      \n",
        true);
    listOpts.addOption("n", "norm",
        "Norm type.               \n\n"
            "    Arguments: [L1|L2] for L1 and L2 norm, respectively. The default option is \n"
            "    is '-n L2' (faster). When Jacknife method is used the option is ignored and\n"
            "    L2 norm is used.                                                           \n",
        true);
    listOpts.addOption("p", "projection",
        "Projection type.                                     \n\n"
            "    Arguments: [W|S][U|L]: Choose either (W)ulff projection or (S)chmidt       \n"
            "    projection. Then select (U)pper hemisphere or (L)ower hemispere projection \n"
            "    The default option is '-p SL'.                                             \n",
        true);
    listOpts.addOption("b", "ball",
        "The details of the beach ball picture                \n\n"
            "    Arguments: [S][A][C][D]: Plot (S)tations, (A)xes, (C)enter cross, best     \n"
            "    (D)ouble-couple lines. The default option is '-b SACD' (all features are   \n"
            "    displayed                                                                  \n",
        true);
    listOpts.addOption("d", "dump",
        "Output data format and order.                        \n\n"
            "    Arguments: [M][C][F][D][A][W][Q][T][U].                                    \n"
            "    [M]: Moment tensor components in Aki's convention: M11,M12,M13,M22,M23,M33.\n"
            "         The moment tensor components are in [Nm]                              \n"
            "    [C]: Moment tensor components in CMT conventions: M33,M11,M22,M13,-M23,-M12\n"
            "         The moment tensor components are in [Nm]                              \n"
            "    [F]: Fault plane solutions in format: STRIKEA/DIPA/RAKEA/STRIKEB/DIPB/RAKEB\n"
            "         (all values in degrees)                                               \n"
            "    [D]: Decomposition of the moment tensor into Isotropic, Compensated linear \n"
            "         vector dipole and double couple in format: ISO/CLVD/DBCP. The numbers \n"
            "         are provided in %.                                                    \n"
            "    [A]: P/T/B Axes orientations in format:                                    \n"
            "         PTREND/PPLUNGE/TTREND/TPLUNGE/BTREND/BPLUNGE                          \n"
            "    [W]: Seismic moment, total seismic moment, maximum error of the seismic    \n"
            "         moment tensor estiamte and the moment magnitude calculated using      \n"
            "         using Hanks&Kanamori formula. The first three values are in [Nm]      \n"
            "    [Q]: Quality index                                                         \n"
            "    [T]: Fault Type (strike slip/normal/inverse)                               \n"
            "    [U]: Vector of synthetic moments calculated (the number of exported numbers\n"
            "         correspond to the number of amplitudes in the input file (specified   \n"
            "         using '-l' option.                                                    \n"
            "    [E]: RMS Error calculated from theoretical and measured ground             \n"
            "         displacements.                                                        \n"
            "                                                                               \n"
            "    NOTE:                                                                      \n"
            "    The order of arguments determine to order of output. For example -d FAD    \n"
            "    exports firstly fault plane solutions, then P, T and B axes directions and \n"
            "    finally the moment tensor decomposition into ISO/CLVD/DBCP. The output file\n"
            "    will have the following structure:                                         \n"
            "    STRIKEA/DIPA/RAKEA/STRIKEB/DIPB/RAKEB/PTREND/PPLUNGE/TTREND/TPLUNGE/BTREND \n"
            "    /BPLUNGE/ISO/CLVD/DBCP                                                     \n",
        true);
    listOpts.addOption("l", "length",
        "Input data length.                                   \n\n"
            "    Argument: number of input lines, e.g. -l 12                                \n",
        true);
    listOpts.addOption("j", "jacknife", "Switches on/off Jacknife test.\n");
    listOpts.addOption("a", "amplitude",
        "Perform amplitude test.                              \n\n"
            "    Arguments: x[/y] where x is a floating-point positive number that describes\n"
            "    the level of noise applied to each amplitude: A+x*A*N(0,1)/3 where N is a  \n"
            "    normal distribution with mean 0 and std 1. The default value of x is 1     \n"
            "    (i.e.amplitude vary by a max. factor of ~2). Optional parameter /y is      \n"
            "    a number of samples (default value is 100).                                \n",
        true);
    listOpts.addOption("f", "fault",
        "Draw fault plane solution directly (and stations).   \n\n"
            "    Arguments: strike/dip/rake[:azimuth1/takeoff1][:azimuth2/takeoff2]...      \n"
            "                                                                               \n",
        true);
    listOpts.addOption("g", "faults",
        "Draw fault plane solution and bootstrap solutions.   \n\n"
            "    Arguments: strike/dip/rake[:s1/d1/r1][:s2/d2/r2]...      \n"
            "                                                                               \n",
        true);
    listOpts.addOption("v", "version", "Display version number");

    Taquart::String SolutionTypes = "D";
    Taquart::String NormType = "L2";
    Taquart::String Projection = "SL";
    Taquart::String BallContent = "SACD";
    Taquart::String DumpOrder = "";
    Taquart::String OutputFileType = "PNG";
    bool JacknifeTest = false;
    bool NoiseTest = false;
    bool DrawFaultOnly = false;
    bool DrawFaultsOnly = false;
    double AmpFactor = 1.0f;
    unsigned int AmplitudeN = 100;
    Taquart::String Temp;
    Taquart::String FaultString;
    if (listOpts.parse(argc, argv))
      while ((switchInt = listOpts.cycle()) >= 0) {
        switch (switchInt) {
          case 0:
            FilenameIn =
                Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim();
            break;
          case 1:
            FilenameOut =
                Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim();
            break;
          case 2:
            SolutionTypes =
                Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim();
            break;
          case 3:
            OutputFileType = Taquart::String(
                listOpts.getArgs(switchInt).c_str()).Trim().UpperCase();
            break;
          case 4:
            NormType =
                Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim().UpperCase();
            break;
          case 5:
            Projection =
                Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim().UpperCase();
            break;
          case 6:
            BallContent =
                Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim().UpperCase();
            break;
          case 7:
            DumpOrder =
                Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim().UpperCase();
            break;
          case 8:
            N =
                Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim().ToInt();
            break;
          case 9:
            JacknifeTest = true;
            break;
          case 10:
            NoiseTest = true;
            Temp = Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim();
            if (Temp.Pos("/")) {
              AmpFactor = Temp.SubString(1, Temp.Pos("/") - 1).ToDouble();
              AmplitudeN = Temp.SubString(Temp.Pos("/") + 1, 1000).ToInt();
            }
            else {
              AmpFactor = Temp.ToDouble();
            }

            //std::cout << "Amplitude factor: " << AmpFactor << std::endl;
            break;
          case 11:
            // Draw fault plane solutions only.
            DrawFaultOnly = true;
            FaultString =
                Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim();
            break;
          case 12:
            // Draw fault plane solutions only.
            DrawFaultsOnly = true;
            FaultString =
                Taquart::String(listOpts.getArgs(switchInt).c_str()).Trim();
            break;
          case 13:
            std::cout << "Rev. 3.0.1, 2014.11.20\n"
                "(c) 2011-2015 Grzegorz Kwiatek, GPL license applies.\n";
            break;
        }
      }

    if (FilenameIn.Length() == 0 && DrawFaultOnly == false
        && DrawFaultsOnly == false) {
      std::cout << "You must provide a valid filename." << std::endl;
    }

    if (FilenameOut.Length() == 0 && DrawFaultOnly == false
        && DrawFaultsOnly == false) {
      FilenameOut = Taquart::ExtractFileName(FilenameIn);
      if (FilenameOut.Pos("."))
        FilenameOut = FilenameOut.SubString(1, FilenameOut.Pos(".") - 1);
    }
    else if (FilenameOut.Length() == 0
        && (DrawFaultOnly == true || DrawFaultsOnly == true)) {
      FilenameOut = "default";
    }

    // Draw fault only and return to dos...
    if (DrawFaultsOnly) {
      // Read strike, dip and rake.
      Taquart::String temp;
      double strike = 0, dip = 0, rake = 0;
      Dispatch(FaultString, temp, "/");
      strike = temp.ToDouble();
      Dispatch(FaultString, temp, "/");
      dip = temp.ToDouble();
      if (FaultString.Pos(":")) {
        rake = FaultString.SubString(1, FaultString.Pos(":") - 1).ToDouble();

        // Dispatch station data...
        Dispatch(FaultString, temp, ":"); // Cut rake part first, as it was already interpreted.

      }
      else {
        rake = FaultString.Trim().ToDouble();
        FaultString = "";
      }

      const double S = strike * DEG2RAD;
      const double D = dip * DEG2RAD;
      const double R = rake * DEG2RAD;

      // Transfer strike/dip/rake to tensor.
      double M11, M22, M33, M12, M13, M23;
      Taquart::StrikeDipRake2MT(S, D, R, M11, M22, M33, M12, M13, M23);

      std::vector<FaultSolutions> FSList;
      Taquart::SMTInputData InputData;

      FaultSolutions fs;
      Taquart::FaultSolution fu;
      Taquart::nodal_plane NP;

      fs.Type = 'N';
      fs.Channel = 0;

      fu.M[1][1] = M11;
      fu.M[1][2] = M12;
      fu.M[1][3] = M13;
      fu.M[2][1] = M12;
      fu.M[2][2] = M22;
      fu.M[2][3] = M23;
      fu.M[3][1] = M13;
      fu.M[3][2] = M23;
      fu.M[3][3] = M33;
      fu.FIA = strike;
      fu.DLA = dip;
      fu.RAKEA = rake;
      NP.str = strike;
      NP.dip = dip;
      NP.rake = rake;
      fu.FIB = Taquart::computed_strike1(NP);
      fu.DLB = Taquart::computed_dip1(NP);
      fu.RAKEB = Taquart::computed_rake1(NP);

      fs.FullSolution = fu;
      fs.TraceNullSolution = fu;
      fs.DoubleCoupleSolution = fu;

      FSList.push_back(fs);

      while (FaultString.Length()) {
        Dispatch(FaultString, temp, "/");
        strike = temp.ToDouble();
        Dispatch(FaultString, temp, "/");
        dip = temp.ToDouble();
        if (FaultString.Pos(":")) {
          rake = FaultString.SubString(1, FaultString.Pos(":") - 1).ToDouble();
          // Dispatch station data...
          Dispatch(FaultString, temp, ":"); // Cut rake part first, as it was already interpreted.
        }
        else {
          rake = FaultString.Trim().ToDouble();
          FaultString = "";
        }
        //
        double S = strike * DEG2RAD;
        double D = dip * DEG2RAD;
        double R = rake * DEG2RAD;

        double M11, M22, M33, M12, M13, M23;
        Taquart::StrikeDipRake2MT(S, D, R, M11, M22, M33, M12, M13, M23);

        fs.Type = 'J';
        fs.Channel = 0;
        fu.M[1][1] = M11;
        fu.M[1][2] = M12;
        fu.M[1][3] = M13;
        fu.M[2][1] = M12;
        fu.M[2][2] = M22;
        fu.M[2][3] = M23;
        fu.M[3][1] = M13;
        fu.M[3][2] = M23;
        fu.M[3][3] = M33;
        fu.FIA = strike;
        fu.DLA = dip;
        fu.RAKEA = rake;
        NP.str = strike;
        NP.dip = dip;
        NP.rake = rake;
        fu.FIB = Taquart::computed_strike1(NP);
        fu.DLB = Taquart::computed_dip1(NP);
        fu.RAKEB = Taquart::computed_rake1(NP);

        fs.FullSolution = fu;
        fs.TraceNullSolution = fu;
        fs.DoubleCoupleSolution = fu;

        FSList.push_back(fs);
      }

      Taquart::String OutName = FilenameOut + ".png";
      Taquart::TriCairo_Meca Meca(500, 500, Taquart::ctSurface);
      GenerateBallCairo(Meca, FSList, InputData, "dbcp");
      Meca.Save(OutName);

      return 0;
    }

    // Draw fault only and return to dos...
    if (DrawFaultOnly) {
      // Read strike, dip and rake.
      Taquart::String temp;
      double strike = 0, dip = 0, rake = 0;
      Dispatch(FaultString, temp, "/");
      strike = temp.ToDouble();
      Dispatch(FaultString, temp, "/");
      dip = temp.ToDouble();
      if (FaultString.Pos(":")) {
        rake = FaultString.SubString(1, FaultString.Pos(":") - 1).ToDouble();

        // Dispatch station data...
        Dispatch(FaultString, temp, ":"); // Cut rake part first, as it was already interpreted.

      }
      else {
        rake = FaultString.Trim().ToDouble();
        FaultString = "";
      }

      //std::cout << strike << " " << dip << " " << rake;

      double S = strike * DEG2RAD;
      double D = dip * DEG2RAD;
      double R = rake * DEG2RAD;

      // Transfer strike/dip/rake to tensor.
      double M11, M22, M33, M12, M13, M23;
      Taquart::StrikeDipRake2MT(S, D, R, M11, M22, M33, M12, M13, M23);

      std::vector<FaultSolutions> FSList;
      Taquart::SMTInputData InputData;

      FaultSolutions fs;
      Taquart::FaultSolution fu;
      Taquart::nodal_plane NP;

      fs.Type = 'N';
      fs.Channel = 0;

      fu.M[1][1] = M11;
      fu.M[1][2] = M12;
      fu.M[1][3] = M13;
      fu.M[2][1] = M12;
      fu.M[2][2] = M22;
      fu.M[2][3] = M23;
      fu.M[3][1] = M13;
      fu.M[3][2] = M23;
      fu.M[3][3] = M33;
      fu.FIA = strike;
      fu.DLA = dip;
      fu.RAKEA = rake;
      NP.str = strike;
      NP.dip = dip;
      NP.rake = rake;
      fu.FIB = Taquart::computed_strike1(NP);
      fu.DLB = Taquart::computed_dip1(NP);
      fu.RAKEB = Taquart::computed_rake1(NP);

      fs.FullSolution = fu;
      fs.TraceNullSolution = fu;
      fs.DoubleCoupleSolution = fu;

      FSList.push_back(fs);

      Taquart::String OutName = FilenameOut + ".png";
      Taquart::TriCairo_Meca Meca(500, 500, Taquart::ctSurface);
      GenerateBallCairo(Meca, FSList, InputData, "dbcp");
      Meca.Save(OutName);

      return 0;
    }

    // Prepare processing structs.
    Taquart::NormType InversionNormType =
        (NormType == "L2") ? Taquart::ntL2 : Taquart::ntL1;
    int QualityType = 1;
    Taquart::SMTInputData InputData;

    const unsigned int Size = 500;
    int id = 0.0;
    double duration = 0.0, displacement = 0.0;
    double azimuth = 0.0, takeoff = 0.0, velocity = 0.0, distance = 0.0,
        density = 0.0;

    // Load input data
    std::ifstream InputFile;
    InputFile.open(FilenameIn.c_str());
    //char TempBuffer[100];
    for (unsigned int i = 0; i < N; i++) {
      InputFile >> id;
      InputFile >> duration;
      InputFile >> displacement;
      InputFile >> azimuth;
      InputFile >> takeoff;
      InputFile >> velocity;
      InputFile >> distance;
      InputFile >> density;

      // Dump to input file.
      Taquart::SMTInputLine il;
      //sprintf(TempBuffer, "%02d", id);
      il.Name = Taquart::FormatFloat("%02d", id); /*!< Station name.*/
      il.Id = id; /*!< Station id number.*/
      il.Component = "ZZ"; //"ZZ";       /*!< Component.*/
      il.MarkerType = ""; //"p*ons/p*max";      /*!< Type of the marker used.*/
      il.Start = 0.0; //tstart;;           /*!< Start time [s].*/
      il.End = duration; //tend;;             /*!< End time [s].*/
      il.Duration = duration; /*!< Duration of signal [s].*/
      il.Displacement = displacement; /*!< Displacement [m]. */
      il.Incidence = 0; //incidence;       /*!< Angle of incidence [deg]. */
      il.Azimuth = azimuth; /*!< Azimuth between station and source [deg]. */
      il.TakeOff = takeoff; /*!< Takeoff angle [deg]. */
      il.Distance = distance; /*!< Distance between station and source [m]. */
      il.Density = density; /*!< Density [km/m**3]. */
      il.Velocity = velocity; /*!< Average velocity [m/s]. */
      il.PickActive = true;
      il.ChannelActive = true;

      InputData.Add(il);

    }
    InputFile.close();
    bool Result = false;
    InputData.CountRuptureTime(Result);

    // Depending on the method, calculate moment tensor once or N times (Jackknife test)

    // Output structures.

    std::vector<FaultSolutions> FSList;

    // Perform regular SMT inversion with all stations.
    Taquart::FaultSolution fu;
    Taquart::FaultSolution tr;
    Taquart::FaultSolution dc;

    try {
      int ThreadProgress = 0;
      USMTCore(InversionNormType, QualityType, InputData, &ThreadProgress);
    }
    catch (...) {
      std::cout << "Inversion error." << std::endl;
      return 1;
    }

    // Transfer solution.
    TransferSolution(Taquart::stFullSolution, fu);
    TransferSolution(Taquart::stTraceNullSolution, tr);
    TransferSolution(Taquart::stDoubleCoupleSolution, dc);

    FaultSolutions fs;
    fs.Type = 'N';
    fs.Channel = 0;
    fs.FullSolution = fu;
    fs.TraceNullSolution = tr;
    fs.DoubleCoupleSolution = dc;

    FSList.push_back(fs);

    if (NoiseTest) {
      srand((unsigned) time(0));

      const Taquart::SMTInputData fd = InputData;
      for (unsigned int i = 0; i < AmplitudeN; i++) {
        Taquart::SMTInputData td = fd;
        Taquart::SMTInputLine InputLine;

        //std::cout << i << std::endl;
        int sample;
        double u1, u2, z;
        for (unsigned int j = 0; j < td.Count(); j++) {
          td.Get(j, InputLine);
          sample = rand();
          u1 = (sample + 1) / (double(RAND_MAX) + 1);
          sample = rand();
          u2 = (sample + 1) / (double(RAND_MAX) + 1);
          z = sqrt(-2.0 * log(u1)) * cos(2 * M_PI * u2);
          //std::cout << RAND_MAX << " " << sample << " " << u1 << " " << u2<< " " << z << std::endl;
          InputLine.Displacement = InputLine.Displacement
              + z / 3.0 * InputLine.Displacement * AmpFactor;
          td.Set(j, InputLine);
        }

        // Calculate SMT with one station removed.
        try {
          int ThreadProgress = 0;
          USMTCore(InversionNormType, QualityType, td, &ThreadProgress);
        }
        catch (...) {
          std::cout << "Inversion error." << std::endl;
          return 1;
        }

        // Transfer solution.
        TransferSolution(Taquart::stFullSolution, fu);
        TransferSolution(Taquart::stTraceNullSolution, tr);
        TransferSolution(Taquart::stDoubleCoupleSolution, dc);

        FaultSolutions fs;
        fs.Type = 'A';
        fs.Channel = 0;
        fs.FullSolution = fu;
        fs.TraceNullSolution = tr;
        fs.DoubleCoupleSolution = dc;

        FSList.push_back(fs);
      }
    }
    else {
      // Perform additional jacknife tests.
      if (JacknifeTest) {
        const Taquart::SMTInputData fd = InputData;
        const unsigned int Count = InputData.Count();

        // Remove one channel, calculate the solution,
        for (unsigned int i = 0; i < Count; i++) {
          Taquart::SMTInputData td = fd;
          Taquart::SMTInputLine InputLine;
          td.Get(i, InputLine);
          int channel = InputLine.Id;
          td.Remove(i);

          // Calculate SMT with one station removed.
          try {
            int ThreadProgress = 0;
            USMTCore(InversionNormType, QualityType, td, &ThreadProgress);
          }
          catch (...) {
            std::cout << "Inversion error." << std::endl;
            return 1;
          }

          // Transfer solution.
          TransferSolution(Taquart::stFullSolution, fu);
          TransferSolution(Taquart::stTraceNullSolution, tr);
          TransferSolution(Taquart::stDoubleCoupleSolution, dc);

          FaultSolutions fs;
          fs.Type = 'J';
          fs.Channel = channel;
          fs.FullSolution = fu;
          fs.TraceNullSolution = tr;
          fs.DoubleCoupleSolution = dc;

          FSList.push_back(fs);
        }
      }
    }

    // Produce output file and graphical represntation of the moment tensor
    // using cairo library.

    // Projection type.
    if (Projection.Pos("W")) WulffProjection = true;
    if (Projection.Pos("S")) WulffProjection = false;

    if (Projection.Pos("U")) LowerHemisphere = false;
    if (Projection.Pos("L")) LowerHemisphere = true;

    // Content of ball.
    if (BallContent.Pos("S"))
      DrawStations = true;
    else
      DrawStations = false;

    if (BallContent.Pos("A"))
      DrawAxes = true;
    else
      DrawAxes = false;

    if (BallContent.Pos("C"))
      DrawCross = true;
    else
      DrawCross = false;

    if (BallContent.Pos("D"))
      DrawDC = true;
    else
      DrawDC = false;

    for (unsigned int j = 0; j < FSList.size(); j++) {
      Taquart::FaultSolution Solution = FSList[j].DoubleCoupleSolution;
      char Type = FSList[j].Type;
      int Channel = FSList[j].Channel;

      Taquart::String FSuffix = "dbcp";
      for (int i = 1; i <= SolutionTypes.Length(); i++) {
        switch (SolutionTypes[i]) {
          case 'F':
            Solution = FSList[j].FullSolution;
            FSuffix = "full";
            break;
          case 'T':
            Solution = FSList[j].TraceNullSolution;
            FSuffix = "clvd";
            break;
          case 'D':
            Solution = FSList[j].DoubleCoupleSolution;
            FSuffix = "dbcp";
            break;
        }

        // Output text data if necessary.
        if (DumpOrder.Length()) {
          Taquart::String OutName = FilenameOut + "-" + FSuffix + ".asc";
          ofstream OutFile(OutName.c_str(),
              std::ofstream::out | std::ofstream::app);

          if (JacknifeTest) {
            // Dump additional information when Jacknife test performed.
            OutFile << Type << "\t" << Channel << "\t";
          }

          for (int i = 1; i <= DumpOrder.Length(); i++) {
            // M - moment, D - decomposition, A - axis, F - fault planes,
            // C - moment in CMT convention.

            // Dump moment tensor components.
            if (DumpOrder[i] == 'M') {
              OutFile << Solution.M[1][1] << "\t";
              OutFile << Solution.M[1][2] << "\t";
              OutFile << Solution.M[1][3] << "\t";
              OutFile << Solution.M[2][2] << "\t";
              OutFile << Solution.M[2][3] << "\t";
              OutFile << Solution.M[3][3] << "\t";
            }

            // Dump moment tensor components in CMT convention.
            if (DumpOrder[i] == 'C') {
              OutFile << Solution.M[3][3] << "\t";
              OutFile << Solution.M[1][1] << "\t";
              OutFile << Solution.M[2][2] << "\t";
              OutFile << Solution.M[1][3] << "\t";
              OutFile << -Solution.M[2][3] << "\t";
              OutFile << -Solution.M[1][2] << "\t";
            }

            if (DumpOrder[i] == 'D') {
              OutFile << Solution.EXPL << "\t";
              OutFile << Solution.CLVD << "\t";
              OutFile << Solution.DBCP << "\t";
            }

            if (DumpOrder[i] == 'A') {
              OutFile << Solution.PXTR << "\t";
              OutFile << Solution.PXPL << "\t";
              OutFile << Solution.TXTR << "\t";
              OutFile << Solution.TXPL << "\t";
              OutFile << Solution.BXTR << "\t";
              OutFile << Solution.BXPL << "\t";
            }

            if (DumpOrder[i] == 'F') {
              OutFile << Solution.FIA << "\t";
              OutFile << Solution.DLA << "\t";
              OutFile << Solution.RAKEA << "\t";
              OutFile << Solution.FIB << "\t";
              OutFile << Solution.DLB << "\t";
              OutFile << Solution.RAKEB << "\t";
            }

            if (DumpOrder[i] == 'W') {
              OutFile << Solution.M0 << "\t";
              OutFile << Solution.MT << "\t";
              OutFile << Solution.ERR << "\t";
              OutFile << Solution.MAGN << "\t";
            }

            if (DumpOrder[i] == 'Q') {
              OutFile << Solution.QI << "\t";
            }

            if (DumpOrder[i] == 'T') {
              OutFile << Solution.Type.c_str() << "\t";
            }

            if (DumpOrder[i] == 'U') {
              for (int r = 0; r < Solution.U_n; r++)
                OutFile << Solution.U_th[r] << "\t";
            }

            if (DumpOrder[i] == 'E') {
              OutFile << Solution.UERR << "\t";
            }
          }

          OutFile << "\n";
          OutFile.close();
        }

        // Output picture name

        // Do not dump anything.
        if (OutputFileType.Pos("NONE") && j == 0) continue;

        // Dump to PNG.
        if (OutputFileType.Pos("PNG") && j == 0) {
          try {
            Taquart::String OutName = FilenameOut + "-" + FSuffix + ".png";
            Taquart::TriCairo_Meca Meca(Size, Size, Taquart::ctSurface);
            GenerateBallCairo(Meca, FSList, InputData, FSuffix);
            Meca.Save(OutName);
          }
          catch (...) {
            return 2;
          }
        }

        // Dump to SVG.
        if (OutputFileType.Pos("SVG") && j == 0) {
          try {
            Taquart::String OutName = FilenameOut + "-" + FSuffix + ".svg";
            Taquart::TriCairo_Meca Meca(Size, Size, Taquart::ctSVG, OutName);
            GenerateBallCairo(Meca, FSList, InputData, FSuffix);
          }
          catch (...) {
            return 2;
          }
        }

        // Dump to PS.
        if (OutputFileType.Pos("PS") && j == 0) {
          try {
            Taquart::String OutName = FilenameOut + "-" + FSuffix + ".ps";
            Taquart::TriCairo_Meca Meca(Size, Size, Taquart::ctPS, OutName);
            GenerateBallCairo(Meca, FSList, InputData, FSuffix);
          }
          catch (...) {
            return 2;
          }
        }

        // Dump to PDF.
        if (OutputFileType.Pos("PDF") && j == 0) {
          try {
            Taquart::String OutName = FilenameOut + "-" + FSuffix + ".pdf";
            Taquart::TriCairo_Meca Meca(Size, Size, Taquart::ctPDF, OutName);
            GenerateBallCairo(Meca, FSList, InputData, FSuffix);
          }
          catch (...) {
            return 2;
          }
        }
      }
    } // Loop for all solution types.

    return 0;
  }
  catch (...) {
    return 1; // Some undefined error occurred, error code 1.
  }

}

void GenerateBallCairo(Taquart::TriCairo_Meca &Meca,
    std::vector<FaultSolutions> &FSList, Taquart::SMTInputData &id,
    Taquart::String Type) {

  Taquart::FaultSolution s;

  if (Type == Taquart::String("dbcp")) {
    s = FSList[0].DoubleCoupleSolution;
  }
  else if (Type == "clvd") {
    s = FSList[0].TraceNullSolution;
  }
  else if (Type == "full") {
    s = FSList[0].FullSolution;
  }

  // Setup solution properties.
  Meca.DrawAxis = DrawAxes;
  Meca.DrawStations = DrawStations;
  Meca.DrawCross = DrawCross;
  Meca.DrawDC = DrawDC;

  // Draw seismic moment tensor solution.
  Meca.Projection = WulffProjection ? Taquart::prWulff : Taquart::prSchmidt;
  Meca.Hemisphere = LowerHemisphere ? Taquart::heLower : Taquart::heUpper;

  double cmt[6];
  cmt[0] = s.M[3][3];
  cmt[1] = s.M[1][1];
  cmt[2] = s.M[2][2];
  cmt[3] = s.M[1][3];
  cmt[4] = s.M[2][3] * -1.0;
  cmt[5] = s.M[1][2] * -1.0;

  Taquart::TriCairo_MomentTensor mt;
  for (int i = 0; i < 6; i++)
    mt.f[i] = cmt[i];

#define squared(x) (pow(x,2.0))

  const double scal =
      sqrt(
          squared(mt.f[0]) + squared(mt.f[1]) + squared(mt.f[2])
              + 2.
                  * (squared(mt.f[3]) + squared(mt.f[4]) + squared(mt.f[5]))) / M_SQRT2;
  for (int i = 0; i < 6; i++)
    mt.f[i] = mt.f[i] / scal;

  Taquart::TriCairo_Axis P, T, N;
  Meca.GMT_momten2axe(mt, &T, &N, &P);
  //P.str = s.PXTR;
  //P.dip = s.PXPL;
  //T.str = s.TXTR;
  //T.dip = s.TXPL;
  //N.str = s.BXTR;
  //N.dip = s.BXPL;

  // Draw circle + tensional & compressional parts of the moment tensor.
  Meca.Tensor(T, N, P);

  // Draw stations.
  if (DrawStations) {
    for (unsigned int i = 0; i < id.Count(); i++) {
      Taquart::SMTInputLine il;
      id.Get(i, il);

      // Calculate Gamma and Displacement.
      double GA[5];

      double Tko = il.TakeOff;
      if (Tko == 90.0f) Tko = 89.75f;

      GA[3] = cos(Tko * DEG2RAD);
      double help = sqrt(1.0f - GA[3] * GA[3]);
      GA[1] = cos(il.Azimuth * DEG2RAD) * help;
      GA[2] = sin(il.Azimuth * DEG2RAD) * help;
      double U = il.Displacement;
      Taquart::String Name = il.Name;

      double mx, my;
      Meca.Station(GA, U, Name, mx, my);
    }

  }

  // Draw P and T axes' directions.
  if (DrawAxes) {
    Meca.Axis(P, "P");
    Meca.Axis(T, "T");
  }

  if (DrawCross) Meca.CenterCross();

  // Draw double-couple lines.
  if (DrawDC) {
    Meca.BDCColor = Taquart::TCColor(0.0, 0.0, 0.0, 1.0);
    Meca.DoubleCouple(s.FIA, s.DLA);
    Meca.DoubleCouple(s.FIB, s.DLB);
  }

  // if MORE than one solution on the list, plot additiona DC lines.

  if (FSList.size() > 1) {
    for (unsigned int i = 1; i < FSList.size(); i++) {
      Meca.BDCColor = Taquart::TCColor(0.5, 0.5, 0.5, 0.7);

      Taquart::FaultSolution * s;

      if (Type == "dbcp") {
        s = &FSList[i].DoubleCoupleSolution;
      }
      if (Type == "clvd") {
        s = &FSList[i].TraceNullSolution;
      }
      if (Type == "full") {
        s = &FSList[i].FullSolution;
      }

      // "Normal fault","Strike fault","Reverse fault"
      if (s->Type == "Normal fault") {
        Meca.BDCColor = Taquart::TCColor(0.0, 0.0, 1.0, 0.7);
      }
      else if (s->Type == "Reverse fault") {
        Meca.BDCColor = Taquart::TCColor(1.0, 0.0, 0.0, 0.7);
      }
      else {
        Meca.BDCColor = Taquart::TCColor(0.0, 1.0, 0.0, 0.7);
      }

      Meca.DoubleCouple(s->FIA, s->DLA);
      Meca.DoubleCouple(s->FIB, s->DLB);
    }
  }

}

