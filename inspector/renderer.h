#pragma once

#include <QMainWindow>
#include <QWidget>
#include <QUuid>
#include <QAudioOutput>

#include <acsfile.h>

class CharacterWindowPrivate;
class CharacterWindow : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString characterName  READ characterName CONSTANT FINAL)
    Q_PROPERTY(QString description  READ description CONSTANT FINAL)
    Q_PROPERTY(QUuid guid READ guid CONSTANT FINAL)
    Q_PROPERTY(bool speechEnabled READ speechEnabled CONSTANT FINAL)
    Q_PROPERTY(bool idleEnabled READ idleEnabled WRITE setIdleEnabled FINAL)
public:
    CharacterWindow(const QString &filename, QWidget *parent = nullptr);
    ~CharacterWindow();
    void Animate(const QString &name);
    bool isLoaded() const;
    QString getLastError() const;
    QString characterName() const;
    QString description() const;

    QUuid guid() const;
    bool speechEnabled() const;
    bool idleEnabled() const;
    void setIdleEnabled(const bool idle);
    libacsfile::Character* Character() const;
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
signals:
    void animationCompleted();
private:
    void queueAnimation(libacsfile::Animation *a);
    void setState(const QString &state);
    void gracefulStop();
    int chooseOptionPercent(const std::vector<int>& probabilities);
    void doAnimation(libacsfile::Animation *a);
    void drawFrame(libacsfile::Frame *frame);
    void playSoundEffect(libacsfile::Sound *sound);
private:
    Q_DECLARE_PRIVATE(CharacterWindow)
    QScopedPointer<CharacterWindowPrivate> d_ptr;
};
