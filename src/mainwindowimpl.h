#ifndef MAINWINDOWIMPL_H
#define MAINWINDOWIMPL_H

#include <QtWidgets/QMainWindow>
#include <QTimer>
#include "joystick.h"
#include "ui_mainwindow.h"

class MainWindowImpl : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindowImpl( QWidget * parent = 0, Qt::WindowFlags f = 0 );
private:
    QTimer *timer;
    Joystick *joystick;
    bool buttonIsHit;

    QTimer *yellowTimer;
    QTimer *redTimer;
    QTimer *blueTimer;
    QTimer *greenTimer;
protected:
    void keyPressEvent(QKeyEvent *event);

    bool isSpotifyPlaying();
    void playAlarm(QString Str, QString color);
private slots:
    void updateForm();
    void joyConnect();
    void continuePlay();
    void playPause();
    void next();
    void previous();

    void yellowButtonClicked();
    void yellowButtonEnable();

    void redButtonClicked();
    void redButtonEnable();

    void blueButtonClicked();
    void blueButtonEnable();

    void greenButtonClicked();
    void greenButtonEnable();
};
#endif




