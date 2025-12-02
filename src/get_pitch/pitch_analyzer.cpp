/// @file

#include <iostream>
#include <fstream>
#include <math.h>
#include "pitch_analyzer.h"

using namespace std;

/// Name space of UPC
namespace upc {
  void PitchAnalyzer::autocorrelation(const vector<float> &x, vector<float> &r) const {

    for (unsigned int l = 0; l < r.size(); ++l) {
  		/// \TODO Compute the autocorrelation r[l]
      /** \DONE
       * We implement Autocorrelation
       * - We initialized to 0...
       * - Acumulated for all the signal...
       * - Divided by length... 
       * */
      r[l] = 0;
      for(unsigned int n = 0; n < x.size()-l; n++){
        r[l] += x[n]*x[n+l];
      }
      r[l] /= x.size();
    }

    if (r[0] == 0.0F) //to avoid log() and divide zero 
      r[0] = 1e-10; 
  }

  void PitchAnalyzer::set_window(Window win_type) {
    if (frameLen == 0)
      return;

    window.resize(frameLen);

    switch (win_type) {
    case HAMMING:
      /// \TODO Implement the Hamming window
      for(unsigned int i=0; i<frameLen; i++){
          window[i] = 0.53836 - 0.46164*cos(2*M_PI*i/(frameLen - 1)); // https://es.wikipedia.org/wiki/Ventana_(funci%C3%B3n)#Hamming
        }
      /** 
       * \DONE Hamming window implemented
       * - We have used the formula given by https://es.wikipedia.org/wiki/Ventana_(funci%C3%B3n)#Hamming
       */
      break;
    case RECT:
    default:
      window.assign(frameLen, 1);
    }
  }

  void PitchAnalyzer::set_f0_range(float min_F0, float max_F0) {
    npitch_min = (unsigned int) samplingFreq/max_F0;
    if (npitch_min < 2)
      npitch_min = 2;  // samplingFreq/2

    npitch_max = 1 + (unsigned int) samplingFreq/min_F0;

    //frameLen should include at least 2*T0
    if (npitch_max > frameLen/2)
      npitch_max = frameLen/2;
  }

  bool PitchAnalyzer::unvoiced(float pot, float r1norm, float rmaxnorm) const {
    /// \TODO Implement a rule to decide whether the sound is voiced or not.
    /// * You can use the standard features (pot, r1norm, rmaxnorm),
    ///   or compute and use other ones.
    if(rmaxnorm>umaxnorm && r1norm > r1thr && pot > powthr) return false; //Autocorrelación en el candidato a pitch.
    return true; //Considera que todas las tramas son sordas.

    /** 
     * \DONE Criteria for differencing between voiced/unvoiced established
     * It has been considered the autocorrelation at long term, the relation R(1)/R(0) and the power.
     */
  }

  float PitchAnalyzer::compute_pitch(vector<float> & x) const {
    //Calculate autoccorrelation
    if (x.size() != frameLen)
      return -1.0F;

    //Frame center-clipping
    float max = *std::max_element(x.begin(), x.end());
    for(int i = 0; i < (int)x.size(); i++) {
      if(abs(x[i]) < cclip) {
        x[i] = 0.0F;
      }
    }

    //Frame normalization
    max = *std::max_element(x.begin(), x.end());
    for (int i = 0; i < (int)x.size(); i++)
      x[i] /= max;

    //Window input frame
    for (unsigned int i=0; i<x.size(); ++i)
      x[i] *= window[i];

    vector<float> r(npitch_max);

    //Compute correlation
    autocorrelation(x, r);

    vector<float>::const_iterator iR = r.begin(), iRMax = iR;

    /// \TODO 
	/// Find the lag of the maximum value of the autocorrelation away from the origin.<br>
	/// Choices to set the minimum value of the lag are:
	///    - The first negative value of the autocorrelation.
	///    - The lag corresponding to the maximum value of the pitch.
    ///	   .
	/// In either case, the lag should not exceed that of the minimum value of the pitch.
    for(iR = iRMax =  r.begin() + npitch_min; iR < r.begin() + npitch_max; iR++){ // The maximum has to be located between the minimum and maximum pitch, so it is a reasonable value.
      // begin() is used to return an iterator pointing to the first element of the vector container
      if(*iR > * iRMax) iRMax = iR; //Localizamos el máximo --> Se actualiza iRMax si el valor concreto que se está estudiando en ese momento es mayor.
    }
    unsigned int lag = iRMax - r.begin();
     /** 
      * \DONE Lag of the maximum value computed
      * - Iteration through the autocorrelation's vector.
      * - Selection of the highest value while iterating.
      * - Difference between the highest value's position and the initial position.
    */

    float pot = 10 * log10(r[0]);

    //You can print these (and other) features, look at them using wavesurfer
    //Based on that, implement a rule for unvoiced
    //change to #if 1 and compile
#if 0 //This 'if 0' is used to see the values of the autocorrelation. If we put if 1 we will see the values on the screen
    if (r[0] > 0.0F)
      cout << pot << '\t' << r[1]/r[0] << '\t' << r[lag]/r[0] << endl;
#endif
    
    if (unvoiced(pot, r[1]/r[0], r[lag]/r[0]))
      return 0; //Unvoiced frame
    else
      return (float) samplingFreq/(float) lag; //If it is voiced, the pitch frequency of the maximum of the autocorrelation is returned
  }
}