#pragma once
#include <QMainWindow>
#include <QTextEdit>

class LoggerWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit LoggerWindow(QWidget *parent = nullptr);
    void log(const QString &text);
signals:
private:
    QTextEdit *m_log = nullptr;
};
