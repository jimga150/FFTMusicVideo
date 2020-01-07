#ifndef FFT_MV_H
#define FFT_MV_H

#include <QMediaPlayer>
#include <QtMath>
#include <QRandomGenerator>

#include "fftwpp/Array.h"
#include "fftwpp/fftw++.h"

#include "AudioFile.h"

#include "opengl2dwindow.h"

using namespace std;
using namespace utils;
using namespace Array;
using namespace fftwpp;

struct color_transition{
    uint window_index;
    bool done = false;
    
    color_transition(uint w){
        this->window_index = w;
    }
};

class FFT_MV : public OpenGL2DWindow
{
public:
    FFT_MV();
    ~FFT_MV() override;
    
    void render(QPainter &painter) override;
    
    void doGameStep() override;
    
    double magnitude(Complex c){
        return qSqrt(c.imag()*c.imag() + c.real()*c.real());
    }
    
    
    QString wav_path = "/Users/jim/Desktop/Code/C_Cplusplus/FFT_MusicVideo/Threatened.wav";
    //QString wav_path = "/Users/jim/Desktop/Code/C_Cplusplus/FFT_MusicVideo/Destroid_Crusaders.wav";
    
    QMediaPlayer player;
        
    double fps;
        
    vector<double*> ffts;
    
    uint current_window = 0;
    
    QRect ui_rect;
    
    double windows_per_second;
    
    double log_denom = qLn(10);
    
    uint num_bars;
    
    double bar_width;
    
    QPen pen;
    ulong old_color_index = 0;
    
    vector<QColor> color_pool;
    
    vector<color_transition> transitions;
    
    QRandomGenerator rng = QRandomGenerator::securelySeeded();
    
    QPixmap background = QPixmap("/Users/jim/Desktop/Code/C_Cplusplus/FFT_MusicVideo/threatened.jpg");
    
    double aspect_ratio = background.rect().width()*1.0/background.rect().height();
    
    const double max_singing_Hz = 1760;
    
    const double bar_separation_factor = 1.2;
    
    const double last_val_coeff = 0.9;
        
    const uint bar_multiplier = 4;
    
    double max_fft_value;
};

#endif // FFT_MV_H
