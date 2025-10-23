#pragma once

#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QDialog>

class CharacterWindow;
class PropertiesWindow : public QDialog
{
    Q_OBJECT
public:
    PropertiesWindow(const QString &filename, QWidget *parent = nullptr);
    bool isLoaded();
    QString getLastError() const;
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    CharacterWindow *m_render = nullptr;
    QTabWidget *m_tabs = nullptr;
    QListWidget *m_stateList = nullptr;
    QListWidget *m_animationList = nullptr;
    bool m_hiding = false;
};
