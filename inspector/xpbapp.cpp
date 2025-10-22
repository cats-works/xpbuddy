#include "xpbapp.h"
#include "propertieswindow.h"

#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMutex>

LoggerWindow* g_logger = nullptr;
QMutex g_logMutex;

void inspectorLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    if (g_logger) {
        QMutexLocker locker(&g_logMutex);
        g_logger->log(msg);
    }

    // Also output to console
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
    fflush(stderr);
}

XPBApplication::XPBApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_menuBar(new QMenuBar(nullptr))
    , m_logger(new LoggerWindow(nullptr))
{
    setApplicationName(tr("XPBInspector"));
    setApplicationDisplayName(tr("Character Inspector"));
    setOrganizationDomain(QString("com.catsworks.xpbinspector"));
    setOrganizationName(tr("SOSUMI BONZIBROS"));

    g_logger = m_logger;
    qInstallMessageHandler(inspectorLogger);
#ifdef Q_OS_MAC
    setQuitOnLastWindowClosed(false);
#endif

    auto fileMenu = m_menuBar->addMenu(tr("&File"));
    auto windowMenu = m_menuBar->addMenu(tr("&Window"));
    auto open = fileMenu->addAction(tr("&Open..."));
    fileMenu->addSeparator();
    auto exportGIF = fileMenu->addAction(tr("Export &GIF..."));
    exportGIF->setDisabled(true);
    connect(open, &QAction::triggered, this, &XPBApplication::openFile);

    auto log = windowMenu->addAction(tr("&Log"));
    log->setShortcut(QKeySequence("Ctrl+Shift+L"));
    connect(log, &QAction::triggered, this, [this]() {
        m_logger->show();
    });
#ifdef DEBUG
    m_logger->show();
#endif
    openFile();
}

bool XPBApplication::openFile()
{
    auto homedir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    auto fn = QFileDialog::getOpenFileName(nullptr, QString("Select ACS File"), homedir, QString("Agent Characters (*.acs);;BOB Actors (*.act)"));
    QFile file(fn);
    if(file.exists())
    {
        auto *w = new PropertiesWindow(fn);
        w->show();
        m_openAgents.append(w);
        connect(w, &QDialog::finished, this, [this]() {
            auto window = qobject_cast<PropertiesWindow*>(sender());
            if(window)
                m_openAgents.removeOne(window);
        });

        return true;
    }

    return false;
}

int main(int argc, char *argv[])
{
    XPBApplication a(argc, argv);
    return a.exec();
}
