/// @file

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>

#include "wavfile_mono.h"
#include "pitch_analyzer.h"

#include "docopt.h"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Estimator 

Usage:
    get_pitch [options] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    -m FLOAT, --umaxnorm = FLOAT  Long-term autocorrelation threshold [default: 0.4]
    -r FLOAT, --r1norm = FLOAT  R(1)/R(0) autocorrelation threshold [default: 0.6]
    -1 FLOAT, --cclip1 FLOAT  Whole signal Center-clipping threshold [default: 0.025]
    -2 FLOAT, --cclip2 FLOAT  Frame Center-clipping threshold [default: 0.008]
    -p FLOAT, --powthr FLOAT  Power threshold [default: -55]
    -h, --help  Show this screen
    --version   Show the version of the project

Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the estimation:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

int main(int argc, const char *argv[]) {
	/// \TODO 
	///  Modify the program syntax and the call to **docopt()** in order to
	///  add options and arguments to the program.

  /// \DONE
  /// The following parameters have been included:
  /// - umaxnorm
  /// - r1norm
  /// - center clipping 1 (cclip1)
  /// - center clipping 2 (cclip2)
  /// - power threshold (powthr)
  
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},	// array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string
        //El docopt devuelve un mapa.

	  std::string input_wav = args["<input-wav>"].asString();
	  std::string output_txt = args["<output-txt>"].asString();
    
  float umaxnorm = stof(args["--umaxnorm"].asString()); // Siempre accedemos con la key larga.
  float r1norm = stof(args["--r1norm"].asString());
  float cclip1 = stof(args["--cclip1"].asString());
  float cclip2 = stof(args["--cclip2"].asString());
  float powthr = stof(args["--powthr"].asString());

  // Read input sound file
  unsigned int rate;
  vector<float> x;
  if (readwav_mono(input_wav, rate, x) != 0) {
    cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
    return -2;
  }

  int n_len = rate * FRAME_LEN;
  int n_shift = rate * FRAME_SHIFT;

  // Define analyzer
  PitchAnalyzer analyzer(n_len, rate, umaxnorm, r1norm, powthr, cclip2, PitchAnalyzer::RECT, 50, 500);

  /// \TODO
  /// Preprocess the input signal in order to ease pitch estimation. For instance,
  /// central-clipping or low pass filtering may be used.
  
  float max = *std::max_element(x.begin(), x.end());
  for(int i = 0; i < (int)x.size(); i++) {
    if(abs(x[i]) < cclip1*max) {
      x[i] = 0.0F;
    } 
  }
  /// \DONE
  /// A center clipping filter has been computed.

  // Iterate for each frame and save values in f0 vector
  vector<float>::iterator iX;
  vector<float> f0;
  for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
    float f = analyzer(iX, iX + n_len);
    f0.push_back(f);
  }

  /// \TODO
  /// Postprocess the estimation in order to supress errors. For instance, a median filter
  /// or time-warping may be used.
  vector<float> f0_final(f0.size());
  vector<float> temp(3);
  int i;
  for(i = 1; i < (int)(f0.size() - 1); i++) {
    temp = {f0[i-1], f0[i], f0[i+1]};
    auto m = temp.begin() + temp.size()/2;
    std::nth_element(temp.begin(), m, temp.end());
    f0_final[i] = temp[temp.size()/2];
  }
  f0_final[i] = f0_final[i-1];
  f0_final[0] = f0_final[1];
  /// \DONE
  /// Non-recursive Median filter computed

  // Write f0 contour into the output file
  ofstream os(output_txt);
  if (!os.good()) {
    cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
    return -3;
  }

  os << 0 << '\n'; //pitch at t=0
  for (iX = f0_final.begin(); iX != f0_final.end(); ++iX) 
    os << *iX << '\n';
  os << 0 << '\n';//pitch at t=Dur

  return 0;
}