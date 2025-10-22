#pragma once

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QMenuBar>
#include <QVector>

#include "propertieswindow.h"
#include "loggerwindow.h"

class XPBApplication : public QApplication
{
    Q_OBJECT

public:
    XPBApplication(int &argc, char **argv);
    void quit();
public slots:
    bool openFile();
private:
    QMenuBar *m_menuBar = nullptr;
    QVector<PropertiesWindow*> m_openAgents{};
    LoggerWindow *m_logger = nullptr;
};
