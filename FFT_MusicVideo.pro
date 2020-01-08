QT       += core gui multimedia

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#DEFINES += TIME_FRAMES
#DEFINES += TIME_FRAME_COMPS

INCLUDEPATH += "libgwavi/"

msvc {
    INCLUDEPATH += "E:\msys64\home\user\fftw-3.3.5\mingw64\include"
    LIBS += -L"E:\msys64\home\user\fftw-3.3.5\mingw64\bin" -lfftw3f-3 -lfftw3-3
}

macx {
    INCLUDEPATH += "/usr/local/include"
    LIBS += -L"/usr/local/lib" -lfftw3 -lfftw3f
}

SOURCES += opengl2dwindow.cpp \
        QAviWriter.cpp \
        fft_mv.cpp \
        fftwpp/Complex.cc \
        fftwpp/convolution.cc \
        fftwpp/fftw++.cc \
        libgwavi/avi-utils.c \
        libgwavi/fileio.c \
        libgwavi/gwavi.c \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += opengl2dwindow.h \
    AudioFile.h \
    #QAviWriter.h \
    QAviWriter.h \
    fft_mv.h \ \
    fftwpp/Array.h \
    fftwpp/Complex.h \
    fftwpp/align.h \
    fftwpp/cmult-sse2.h \
    fftwpp/convolution.h \
    fftwpp/fftw++.h \
    fftwpp/seconds.h \
    fftwpp/statistics.h \
    fftwpp/transposeoptions.h \
    libgwavi/avi-utils.h \
    libgwavi/fileio.h \
    libgwavi/gwavi.h \
    libgwavi/gwavi_private.h

RESOURCES += \
    Resources.qrc
