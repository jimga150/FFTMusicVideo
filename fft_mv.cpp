#include "fft_mv.h"

FFT_MV::FFT_MV()
{
    Q_ASSERT(!this->background.isNull());
    
    this->color_pool.push_back(QColor(201, 54, 160));
    this->color_pool.push_back(QColor(254, 152, 225));
    this->color_pool.push_back(QColor(124, 27, 96));
    this->color_pool.push_back(QColor(141, 226, 99));
    this->color_pool.push_back(QColor(99, 201, 115));
    this->color_pool.push_back(QColor(186, 250, 127));
    this->color_pool.push_back(QColor(Qt::white));
    
    this->player.setMedia(QUrl::fromLocalFile(this->wav_path));
    
    QScreen* screen = this->screen();
    this->fps = screen->refreshRate();
    
    //must be sorted
    this->transitions.push_back(color_transition(static_cast<uint>(20.646*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(35.298*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(46.497*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(50.199*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(50.666*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(51.126*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(51.603*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(52.077*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(52.738*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(53.552*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(54.499*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(55.259*this->fps)));
    this->transitions.push_back(color_transition(static_cast<uint>(55.879*this->fps)));
    
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
    
    this->pen = QPen(QBrush(this->color_pool.at(this->old_color_index)), this->bar_width, Qt::SolidLine, Qt::RoundCap);
    
    
    uint numsamples = static_cast<uint>(audioFile.getNumSamplesPerChannel());
    
    uint num_windows = static_cast<uint>(qCeil(numsamples*this->fps/samplespersec));
    
    printf("Calculating windows\n");
    fflush(stdout);
    
    double max_magnitude = -__DBL_MAX__;
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
            fft_result_arr[b] = qMax(0.0, qLn(this->magnitude(f[bint]))/this->log_denom);
            
            if (fft_result_arr[b] > max_magnitude) max_magnitude = fft_result_arr[b];
        }
        
        this->ffts.push_back(fft_result_arr);
    }
    
    this->max_fft_value = this->ui_rect.height()/2;
    double mag_coeff = this->max_fft_value/max_magnitude;
    
    for (double* fft : this->ffts){
        for (uint i = 0; i < this->num_bars; ++i){
            fft[i] *= mag_coeff;
        }
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
    double* last_fft = this->ffts.at(this->current_window == 0 ? 0 : this->current_window-1);
    
    int screen_height = this->ui_rect.height();
    
    for (uint i = 0; i < this->num_bars; ++i){
        
        int x = static_cast<int>((i+0.5)*this->bar_width*this->bar_separation_factor);
        int y = qMax(static_cast<int>(fft[i]), static_cast<int>(this->last_val_coeff*last_fft[i]));
        
        painter.drawLine(x, screen_height, x, screen_height - y);
    }
}

void FFT_MV::doGameStep(){
    double current_pos_s = this->player.position()*1.0/1000.0;
    this->current_window = static_cast<uint>(this->windows_per_second*current_pos_s);
    
    for (uint c = 0; c < this->transitions.size(); ++c){
        color_transition ct = this->transitions.at(c);
        if (ct.window_index <= this->current_window && !ct.done){
            
            ulong new_color_index = this->old_color_index;
            while (new_color_index == this->old_color_index){
                new_color_index = static_cast<ulong>(this->rng.bounded(0, static_cast<int>(this->color_pool.size())));
            }
            
            this->pen.setColor(this->color_pool.at(new_color_index));
            
            this->transitions.at(c).done = true;
            break;
        } else if (ct.window_index > this->current_window){
            break;
        }
    }
}
