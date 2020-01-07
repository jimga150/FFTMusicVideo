#include "fft_mv.h"

FFT_MV::FFT_MV()
{
    Q_ASSERT(!this->background.isNull());
    
    this->player.setMedia(QUrl::fromLocalFile(this->wav_path));
    
    QScreen* screen = this->screen();
    this->fps = screen->refreshRate();
    
    QRect screenRect = screen->availableGeometry();
    int screen_width = screenRect.width();
    int screen_height = screenRect.height();
    
    QSize window_size;
    QPoint fullscreen_offset;
    
    if (screen_width*1.0/screen_height > this->aspect_ratio){ //screen is relatively wider than the app
        
        int window_width = static_cast<int>(screen_height*this->aspect_ratio);
        
        window_size = QSize(window_width, screen_height);
        
        fullscreen_offset = QPoint((screen_width - window_width)/2, 0);
        
    } else { //screen is relatively taller than app, or it's the same ratio
        
        int window_height = static_cast<int>(screen_width*1.0/this->aspect_ratio);
        
        window_size = QSize(screen_width, window_height);
        
        fullscreen_offset = QPoint(0, (screen_height - window_height)/2);
    }
    
    this->ui_rect = QRect(QPoint(0, 0), window_size);
    this->setGeometry(QRect(fullscreen_offset, window_size));
    
    fftw::maxthreads = static_cast<uint>(get_max_threads());
    
    
    AudioFile<double> audioFile;
    
    printf("Loading audio file...\n");
    fflush(stdout);
    
    audioFile.load(this->wav_path.toUtf8().constData());
    
    uint samplespersec = audioFile.getSampleRate(); //samples/second
    
    double samples_between_windows_f = samplespersec*1.0/this->fps;
    //uint samples_between_windows = static_cast<uint>(qCeil(samples_between_windows_f));
    uint samples_per_window = static_cast<uint>(qCeil(samples_between_windows_f*this->bar_multiplier));
    
    this->windows_per_second = samplespersec*1.0/samples_between_windows_f;    
    
    this->num_bars = static_cast<uint>(samples_per_window*(this->max_singing_Hz/samplespersec)/2.0);
    
    this->bar_width = this->ui_rect.width()*1.0/(this->num_bars*this->bar_separation_factor);  
    
    this->pen = QPen(QBrush(QColor(201, 54, 160)), this->bar_width, Qt::SolidLine, Qt::RoundCap);
    
    
    uint numsamples = static_cast<uint>(audioFile.getNumSamplesPerChannel());
    
    uint num_windows = static_cast<uint>(qCeil(numsamples*this->fps/samplespersec));
    
    printf("Calculating windows\n");
    fflush(stdout);
    
    for (uint w = 0; w < num_windows; ++w){
        
        array1<Complex> f(samples_per_window, sizeof(Complex));
        
        fft1d Forward(-1,f);
        
        for(uint s = 0; s < samples_per_window; s++){
            ulong sample_index = static_cast<ulong>(w*samples_between_windows_f + s);
            if (sample_index >= numsamples){
                f[static_cast<int>(s)] = 0;
            } else {
                f[static_cast<int>(s)] = audioFile.samples[0][sample_index];
            }
        }
        
        Forward.fft(f);
        
        double* fft_result_arr = new double[this->num_bars];
        for(uint b = 0; b < this->num_bars; b++){
            int bint = static_cast<int>(b);
            fft_result_arr[b] = 200*qLn(this->magnitude(f[bint]))/this->log_denom;
        }
        
        this->ffts.push_back(fft_result_arr);
    }
    
    this->player.play();
}

FFT_MV::~FFT_MV(){
    for (double* arr : this->ffts){
        delete []  arr;
    }
}

void FFT_MV::render(QPainter &painter){
    
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(this->pen);
    
    painter.drawPixmap(this->ui_rect, this->background);
    
    double* fft = this->ffts.at(this->current_window); 
    double* last_fft = this->ffts.at(this->current_window == 0 ? 0 : this->current_window);
    
    int screen_height = this->ui_rect.height();
    
    for (uint i = 0; i < this->num_bars; ++i){
        
        int x = static_cast<int>((i+0.5)*this->bar_width*this->bar_separation_factor);
        int y = screen_height - qMax(static_cast<int>(fft[i]), static_cast<int>(this->last_val_coeff*last_fft[i]));
        
        painter.drawLine(x, screen_height, x, y);
    }
}

void FFT_MV::doGameStep(){
    double current_pos_s = this->player.position()*1.0/1000.0;
    this->current_window = static_cast<uint>(this->windows_per_second*current_pos_s);
}
