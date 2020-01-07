#include <QGuiApplication>

#include "fft_mv.h"

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
    
    QSurfaceFormat format;
    format.setSamples(16);
    
    FFT_MV movie;
    movie.setFormat(format);

    movie.show();
    movie.setAnimating(true);
    
    return a.exec();
}
