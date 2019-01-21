#include "mainwindowimpl.h"
#include <QKeyEvent>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <pulse/pulseaudio.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

#define XF86AudioPlay 0x1008ff14
#define XF86AudioNext 0x1008ff17
#define XF86AudioPrev 0x1008ff16

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

/* Send Fake Key Event */
static void SendKey (Display * disp, KeySym keysym, KeySym modsym)
{
    KeyCode keycode = 0, modcode = 0;

    keycode = XKeysymToKeycode (disp, keysym);
    if (keycode == 0) return;

    XTestGrabControl (disp, True);

    /* Generate modkey press */
    if (modsym != 0)
    {
        modcode = XKeysymToKeycode(disp, modsym);
        XTestFakeKeyEvent (disp, modcode, True, 0);
    }

    /* Generate regular key press and release */
    XTestFakeKeyEvent (disp, keycode, True, 0);
    XTestFakeKeyEvent (disp, keycode, False, 0);

    /* Generate modkey release */
    if (modsym != 0)
        XTestFakeKeyEvent (disp, modcode, False, 0);

    XSync (disp, False);
    XTestGrabControl (disp, False);
}

MainWindowImpl::MainWindowImpl( QWidget * parent, Qt::WindowFlags f)
    : QMainWindow(parent, f)
{
    setupUi(this);
    timer = new QTimer(this);
    yellowTimer = new QTimer(this);
    redTimer = new QTimer(this);
    blueTimer = new QTimer(this);
    greenTimer = new QTimer(this);

    joystick = new Joystick();
    joyDevBox->setText("/dev/input/js0");

    connect( timer, SIGNAL(timeout()), this, SLOT(updateForm()));
    connect( yellowTimer, SIGNAL(timeout()), this, SLOT(yellowButtonEnable()));
    connect( redTimer, SIGNAL(timeout()), this, SLOT(redButtonEnable()));
    connect( blueTimer, SIGNAL(timeout()), this, SLOT(blueButtonEnable()));
    connect( greenTimer, SIGNAL(timeout()), this, SLOT(greenButtonEnable()));

    connect( btnConnect, SIGNAL(clicked()), this, SLOT(joyConnect()));
    connect( continueBtn, SIGNAL(clicked()), this, SLOT(continuePlay()));
    connect( playPauseBtn, SIGNAL(clicked()), this, SLOT(playPause()));
    connect( nextBtn, SIGNAL(clicked()), this, SLOT(next()));
    connect( prevBtn, SIGNAL(clicked()), this, SLOT(previous()));

    connect( yellowButton, SIGNAL(clicked()), this, SLOT(yellowButtonClicked()));
    connect( redButton, SIGNAL(clicked()), this, SLOT(redButtonClicked()));
    connect( blueButton, SIGNAL(clicked()), this, SLOT(blueButtonClicked()));
    connect( greenButton, SIGNAL(clicked()), this, SLOT(greenButtonClicked()));
}

void MainWindowImpl::continuePlay() {
    if ( joystick->connected != 1   ) {
        qDebug("No Buzzer connected.");
    } else {
        for ( int i=0; i<=19; i++) {
            while( joystick->getButton(i) > 0 );
        }
    }

    if ( isSpotifyPlaying() ) {
        playPause();
    }

    buttonIsHit = false;
    label->setText("Ready!");
    label->setStyleSheet("QLabel { background-color : lightgrey; color : black; }");
    timer->start(1);
    qDebug("Reset is done.");
}

void MainWindowImpl::playPause() {
    SendKey (XOpenDisplay (NULL), XF86AudioPlay, 0);
}

void MainWindowImpl::next() {
    SendKey (XOpenDisplay (NULL), XF86AudioNext, 0);
}

void MainWindowImpl::previous() {
    SendKey (XOpenDisplay (NULL), XF86AudioPrev, 0);
}

void MainWindowImpl::joyConnect() {
    if( joystick->init( joyDevBox->text().toStdString().c_str() ) > -1 ) {
        btnConnect->setText("Connected!");
        continuePlay();
        qDebug("Connected.");
    }
}

void MainWindowImpl::updateForm() {
    if ( buttonIsHit ) {
        return;
    }

    if ( joystick->connected != 1 ) {
        return;
    }

    char button[4];
    button[0] = joystick->getButton(0);
    button[1] = joystick->getButton(5);
    button[2] = joystick->getButton(10);
    button[3] = joystick->getButton(15);

    if ( button[0] > 0 ||
         button[1] > 0 ||
         button[2] > 0 ||
         button[3] > 0 ) {
        if ( !isSpotifyPlaying() ) {
            return;
        }
    }

    if ( button[0] > 0 && greenButton->isEnabled()) {
        playPause();
        label->setText("Green");
        label->setStyleSheet("QLabel { background-color : green; color : black; }");
        buttonIsHit = true;
        timer->stop();
        return;
    } else if ( button[1] > 0 && yellowButton->isEnabled()) {
        playPause();
        label->setText("Yellow");
        label->setStyleSheet("QLabel { background-color : yellow; color : black; }");
        buttonIsHit = true;
        timer->stop();
        return;
    } else if ( button[2] > 0 && blueButton->isEnabled()) {
        playPause();
        label->setText("Blue");
        label->setStyleSheet("QLabel { background-color : blue; color : black; }");
        buttonIsHit = true;
        timer->stop();
        return;
    } else if ( button[3] > 0 && redButton->isEnabled()) {
        playPause();
        label->setText("Red");
        label->setStyleSheet("QLabel { background-color : red; color : black; }");
        buttonIsHit = true;
        timer->stop();
        return;
    }
}

void MainWindowImpl::keyPressEvent(QKeyEvent *event) {
    if ( buttonIsHit ) {
        return;
    }

    if ( joystick->connected == 1 ) {
        return;
    }

    if ( event->key() == Qt::Key_1 ||
         event->key() == Qt::Key_2 ||
         event->key() == Qt::Key_3 ||
         event->key() == Qt::Key_4 ) {
        if ( !isSpotifyPlaying() ) {
            return;
        }
    }

    if (event->key() == Qt::Key_1 && greenButton->isEnabled() ) {
        playPause();
        label->setText("Green");
        label->setStyleSheet("QLabel { background-color : green; color : black; }");
        buttonIsHit = true;
        timer->stop();
    } else if (event->key() == Qt::Key_2 && yellowButton->isEnabled() ) {
        playPause();
        label->setText("Yellow");
        label->setStyleSheet("QLabel { background-color : yellow; color : black; }");
        buttonIsHit = true;
        timer->stop();
    } else if (event->key() == Qt::Key_3 && blueButton->isEnabled() ) {
        playPause();
        label->setText("Blue");
        label->setStyleSheet("QLabel { background-color : blue; color : black; }");
        buttonIsHit = true;
        timer->stop();
    } else if (event->key() == Qt::Key_4 && redButton->isEnabled() ) {
        playPause();
        label->setText("Red");
        label->setStyleSheet("QLabel { background-color : red; color : black; }");
        buttonIsHit = true;
        timer->stop();
    }
}

void MainWindowImpl::yellowButtonClicked() {
    yellowButton->setEnabled(false);
    yellowTimer->start(10*1000);
    continuePlay();
}

void MainWindowImpl::yellowButtonEnable() {
    yellowButton->setEnabled(true);
    yellowTimer->stop();
}

void MainWindowImpl::blueButtonClicked() {
    blueButton->setEnabled(false);
    blueTimer->start(10*1000);
    continuePlay();
}

void MainWindowImpl::blueButtonEnable() {
    blueButton->setEnabled(true);
    blueTimer->stop();
}

void MainWindowImpl::redButtonClicked() {
    redButton->setEnabled(false);
    redTimer->start(10*1000);
    continuePlay();
}

void MainWindowImpl::redButtonEnable() {
    redButton->setEnabled(true);
    redTimer->stop();
}

void MainWindowImpl::greenButtonClicked() {
    greenButton->setEnabled(false);
    greenTimer->start(10*1000);
    continuePlay();
}

void MainWindowImpl::greenButtonEnable() {
    greenButton->setEnabled(true);
    greenTimer->stop();
}

bool MainWindowImpl::isSpotifyPlaying() {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("pacmd list-sink-inputs | grep -c \'state: RUNNING\'", "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if ( trim(result).compare( "1" ) == 0 ) {
        qDebug("Spotify is playing");
        return true;
    }
    qDebug("Spotify is stopped/paused");
    return false;
}

