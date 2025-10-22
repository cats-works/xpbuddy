#include "loggerwindow.h"

LoggerWindow::LoggerWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_log(new QTextEdit(this))
{
    resize(500,350);
    setCentralWidget(m_log);
    setWindowTitle(tr("Debug Log"));
    m_log->setAcceptRichText(false);
    m_log->setReadOnly(true);
}

void LoggerWindow::log(const QString &text)
{
    m_log->append(text.trimmed());
}
