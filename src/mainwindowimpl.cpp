#include "mainwindowimpl.h"
#include <QKeyEvent>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
#include <unistd.h>
#include <pulse/pulseaudio.h>

#define XF86AudioPlay 0x1008ff14
#define XF86AudioNext 0x1008ff17
#define XF86AudioPrev 0x1008ff16

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
        qDebug("No reset. No Buzzer connected.");
    } else {
        for ( int i=0; i<=19; i++) {
            while( joystick->getButton(i) > 0 );
        }
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

    if ( button[0] > 0 && greenButton->isEnabled()) {
        SendKey (XOpenDisplay (NULL), XF86AudioPlay, 0);
        label->setText("Green");
        label->setStyleSheet("QLabel { background-color : green; color : black; }");
        buttonIsHit = true;
        timer->stop();
        return;
    } else if ( button[1] > 0 && greenButton->isEnabled()) {
        SendKey (XOpenDisplay (NULL), XF86AudioPlay, 0);
        label->setText("Yellow");
        label->setStyleSheet("QLabel { background-color : yellow; color : black; }");
        buttonIsHit = true;
        timer->stop();
        return;
    } else if ( button[2] > 0 && greenButton->isEnabled()) {
        SendKey (XOpenDisplay (NULL), XF86AudioPlay, 0);
        label->setText("Blue");
        label->setStyleSheet("QLabel { background-color : blue; color : black; }");
        buttonIsHit = true;
        timer->stop();
        return;
    } else if ( button[3] > 0 && greenButton->isEnabled()) {
        SendKey (XOpenDisplay (NULL), XF86AudioPlay, 0);
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

    if (event->key() == Qt::Key_1 && greenButton->isEnabled() ) {
        SendKey (XOpenDisplay (NULL), XF86AudioPlay, 0);
        label->setText("Green");
        label->setStyleSheet("QLabel { background-color : green; color : black; }");
        buttonIsHit = true;
        timer->stop();
    } else if (event->key() == Qt::Key_2 && yellowButton->isEnabled() ) {
        SendKey (XOpenDisplay (NULL), XF86AudioPlay, 0);
        label->setText("Yellow");
        label->setStyleSheet("QLabel { background-color : yellow; color : black; }");
        buttonIsHit = true;
        timer->stop();
    } else if (event->key() == Qt::Key_3 && blueButton->isEnabled() ) {
        SendKey (XOpenDisplay (NULL), XF86AudioPlay, 0);
        label->setText("Blue");
        label->setStyleSheet("QLabel { background-color : blue; color : black; }");
        buttonIsHit = true;
        timer->stop();
    } else if (event->key() == Qt::Key_4 && redButton->isEnabled() ) {
        SendKey (XOpenDisplay (NULL), XF86AudioPlay, 0);
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
