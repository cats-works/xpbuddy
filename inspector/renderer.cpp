#include "renderer.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include <QTimer>
#include <QRgb>
#include <QUuid>
#include <random>

#if QT_VERSION >= 0x060200
#include <QAudioSink>
typedef QAudioSink AudioSink;
#endif
#if QT_VERSION >= 0x060000 && QT_VERSION < 0x060200
#error "Qt Multimedia on this version is unsupported"
#endif
#if QT_VERSION >= 0x050400 && QT_VERSION <= 0x051500
#include <QtAudioOutput>
typedef QtAudioOutput AudioSink;
#endif

#ifdef HAVE_QTSPEECH
#include <QtTextToSpeech>
#endif

using namespace std;
using namespace libacsfile;

#define CHAR_LOG(a) qDebug() << QString("[R:%1] %2 %3") \
                        .arg(__LINE__) \
                        .arg(QString::fromStdString(d_ptr->m_char->Name())) \
                        .arg(a);
#define ANI_LOG(a, b) qDebug() << QString("[R:%1] %2 %3 %4") \
                        .arg(__LINE__) \
                        .arg(QString::fromStdString(d_ptr->m_char->Name())) \
                        .arg(a.data()) \
                        .arg(b);


class CharacterWindowPrivate {
public:
    libacsfile::Character *m_char = nullptr;
    libacsfile::Animation *m_currentAnimation = nullptr;
    QList<libacsfile::Animation*> m_animationQueue;

    bool m_dragging = false;
    bool m_animating = false;
    QPoint m_dragPosition;
    QImage currentFrame;
    bool m_hasAnimation = false;
    bool m_stopRequested = false;
    uint m_frame = 0;
    QString m_doNextAfterReverse{};

    bool m_idle = false;
};

CharacterWindow::CharacterWindow(const QString &filename, QWidget *parent)
    : QWidget(parent)
    , d_ptr(new CharacterWindowPrivate)
{
    d_ptr->m_char = new libacsfile::Character();
    if(parent == nullptr)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Window);
    }

    if(d_ptr->m_char->Load(filename.toStdString()))
    {
        CHAR_LOG("Loaded");
        setWindowTitle(QString::fromStdString(d_ptr->m_char->Name()));
        setMaximumWidth(d_ptr->m_char->Width());
        setMaximumHeight(d_ptr->m_char->Height());
        setMinimumWidth(d_ptr->m_char->Width());
        setMinimumHeight(d_ptr->m_char->Height());
    }
}

CharacterWindow::~CharacterWindow() {}

void CharacterWindow::Animate(const QString &name)
{
    Q_D(CharacterWindow);
    string cname = name.toStdString();
    auto animation = d->m_char->GetAnimation(cname);
    if(animation != nullptr)
    {
        if(d->m_animating)
        {
            gracefulStop();
            queueAnimation(animation);
        }
        else
        {
            if(d->m_currentAnimation != nullptr)
            {
                // see if we need to process a return animation
                if(d->m_currentAnimation->Transition() == libacsfile::Animation::TransitionExitBranches)
                {
                    d->m_stopRequested = true;
                    d->m_frame--;
                    d->m_doNextAfterReverse = name;
                    doAnimation(d->m_currentAnimation);
                    return;
                }
            }
            d->m_currentAnimation = animation;
            d->m_frame = 0;
            doAnimation(animation);
        }
    }
}

bool CharacterWindow::isLoaded() const
{
    Q_D(const CharacterWindow);
    if(!d->m_char)
        return false;

    return d->m_char->Loaded();
}

QString CharacterWindow::getLastError() const
{
    return QString::fromStdString(Character()->GetLastError());
}

QString CharacterWindow::characterName() const
{
    return QString::fromStdString(Character()->Name());
}

QString CharacterWindow::description() const
{
    return QString::fromStdString(Character()->Description());
}

QUuid CharacterWindow::guid() const
{
    QUuid uuid;
    return uuid;
}

bool CharacterWindow::speechEnabled() const
{
    return Character()->TTSEnabled();
}

bool CharacterWindow::idleEnabled() const
{
    Q_D(const CharacterWindow);
    return d->m_idle;
}

void CharacterWindow::setIdleEnabled(const bool idle)
{
    Q_D(CharacterWindow);
    d->m_idle = idle;
}

libacsfile::Character *CharacterWindow::Character() const
{
    Q_D(const CharacterWindow);
    return d->m_char;
}

void CharacterWindow::mousePressEvent(QMouseEvent *event)
{
    Q_D(CharacterWindow);
    if (event->button() == Qt::LeftButton) {
        d->m_dragging = true;
#if QT_VERSION >= 0x060000
        d->m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
#else
        d->m_dragPosition = event->globalPos() - frameGeometry().topLeft();
#endif
        event->accept();
    }
}

void CharacterWindow::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(CharacterWindow);
    if (d->m_dragging && (event->buttons() & Qt::LeftButton)) {
#if QT_VERSION >= 0x060000
        move(event->globalPosition().toPoint() - d->m_dragPosition);
#else
        move(event->globalPos() - d->m_dragPosition);
#endif
        event->accept();
    }
}

void CharacterWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(CharacterWindow);
    if (event->button() == Qt::LeftButton) {
        d->m_dragging = false;
        event->accept();
    }
}

void CharacterWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    Q_D(CharacterWindow);
    if(d->m_currentAnimation)
        drawFrame(d->m_currentAnimation->Frames()[d->m_frame]);
}

void CharacterWindow::showEvent(QShowEvent *event)
{
    Q_D(CharacterWindow);
    QWidget::showEvent(event);
    if(d->m_char->Name() == "Qmark")
    {
        // qmark is special, they don't have proper state
        // or Show animation.
        Animate("Welcome");
        return;
    }

    if(d->m_char->HasState("SHOWING"))
    {
        setState("SHOWING");
        return;
    }

    if(d->m_char->HasAnimation("SHOW"))
    {
        Animate("SHOW");
        return;
    }
}

void CharacterWindow::hideEvent(QHideEvent *event)
{
    Animate("HIDE");
    QWidget::hideEvent(event);
}

void CharacterWindow::queueAnimation(libacsfile::Animation *a)
{
    Q_D(CharacterWindow);
    if(a)
    {
        CHAR_LOG(QString("queueing up animation %1").arg(QString::fromStdString(a->Name())));
        d->m_animationQueue.append(a);
    }
}

void CharacterWindow::setState(const QString &state)
{
    Q_D(CharacterWindow);
    if(d->m_char->States().find(state.toStdString()) != d->m_char->States().end())
    {
        CHAR_LOG(QString("Setting state %1").arg(state));
        auto animations = d->m_char->States()[state.toStdString()];
        for(auto &a : animations)
        {
            CHAR_LOG(QString("State %1: Animation %2").arg(state).arg(QString::fromStdString(a)));
            Animate(QString::fromStdString(a));
        }
    }
}

void CharacterWindow::gracefulStop()
{
    Q_D(CharacterWindow);
    ANI_LOG(d->m_currentAnimation->Name(), QString("gracefully stopping animation"));
    d->m_stopRequested = true;
}

int CharacterWindow::chooseOptionPercent(const std::vector<int>& probabilities)
{
    int sum = 0;
    for (int p : probabilities) sum += p;

    if (sum > 100) {
        throw std::runtime_error("Total probability cannot exceed 100%");
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    int r = dis(gen);
    int cumulative = 0;

    for (size_t i = 0; i < probabilities.size(); ++i) {
        cumulative += probabilities[i];
        if (r <= cumulative) return static_cast<int>(i);
    }

    return -1;
}

void CharacterWindow::doAnimation(libacsfile::Animation *a)
{
    Q_D(CharacterWindow);

    // stop if we are on the last frame
    if(d->m_frame > a->Frames().size()-1)
    {
        ANI_LOG(a->Name(), "last frame");
        d->m_animating = false;
        d->m_frame--;
        d->m_stopRequested = false;
        d->m_currentAnimation = nullptr;
        if(!d->m_animationQueue.isEmpty())
        {
            auto nextAnimation = d->m_animationQueue.takeFirst();
            Animate(QString::fromStdString(nextAnimation->Name()));
        }
        d->m_frame = 0;
        emit animationCompleted();
        return;
    }

    d->m_animating = true;

    auto frame = a->Frames().at(d->m_frame);
    if(frame == nullptr)
    {
        if(d->m_frame > a->Frames().size())
        {
            ANI_LOG(a->Name(), QString("animation completed"));
            emit animationCompleted();
            d->m_stopRequested = false;
            d->m_animating = false;
            return;
        }
    }

    if(frame->Images().size() == 0)
    {
        ANI_LOG(a->Name(), QString("frame %1 has no images").arg(QString::number(d->m_frame)));
        if(a->Frames().size() == d->m_frame)
        {
            d->m_animating = false;
            d->m_frame--;
            update();
            return;
        }
        else
        {
            d->m_frame++;
            doAnimation(a);
            return;
        }
    }

    if(frame->AudioIndex() != 65535)
    {
        ANI_LOG(a->Name(), QString("Playing RIFF #%1").arg(QString::number(frame->AudioIndex())));
        playSoundEffect(frame->Sound());
    }

    ANI_LOG(a->Name(), QString("drawing frame %1 (duration %2ms)").arg(QString::number(d->m_frame))
                           .arg(QString::number(frame->Duration()*10)));
    repaint();

    QTimer::singleShot(frame->Duration()*10, [this,a,frame]() {
        Q_D(CharacterWindow);
        bool hasExitFrame = false;
        if(d->m_stopRequested)
        {
            if(frame->ExitFrame() >= 0)
            {
                hasExitFrame = true;
                d->m_frame = frame->ExitFrame();
            }
        }
        if(!hasExitFrame)
        {
            if(frame->Branches().size() == 0)
            {
                if(d->m_frame >= a->Frames().size()-1)
                {
                    ANI_LOG(a->Name(), QString("animation completed"));
                    d->m_stopRequested = false;
                    d->m_animating = false;
                    if(!d->m_animationQueue.isEmpty())
                    {
                        // do we need to process the return?
                        d->m_currentAnimation = d->m_animationQueue.takeFirst();
                        ANI_LOG(a->Name(), QString("proceeding with next animation %1")
                                .arg(QString::fromStdString(d->m_currentAnimation->Name())));
                        d->m_frame = 0;
                        doAnimation(d->m_currentAnimation);
                    }
                    emit animationCompleted();
                    return;
                }
                d->m_frame++;
            }
            else
            {
                std::vector<int> probabilities;
                for(auto b : frame->Branches())
                    probabilities.push_back(b->Probability());

                int branch = chooseOptionPercent(probabilities);

                if(branch == -1) // do nothing
                    d->m_frame++;
                else
                    d->m_frame = frame->Branches()[branch]->FrameID();
                ANI_LOG(a->Name(), QString("probability solver chose option %1 to frame %2")
                                       .arg(QString::number(branch)).arg(QString::number(d->m_frame)));

            }
        }
        doAnimation(a);
    });
}

void CharacterWindow::drawFrame(libacsfile::Frame *frame)
{
    Q_D(CharacterWindow);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    //The Frame Images are composited in reverse order from last to first.
    //for(uint i = frame->Images().size()-1; i <= 0; i--)
    for(uint i = 0; i < frame->Images().size(); i++)
    {
        auto offsetX = frame->Images()[i]->OffsetX();
        auto offsetY = frame->Images()[i]->OffsetY();

        auto img = frame->Images()[i]->GetImage();
        if(img == nullptr)
            continue;

        int stride = ((width() + 3) & ~3);

        QImage frameImage(img->Data().data(), img->Width(), img->Height(), stride, QImage::Format_Indexed8);

        frameImage.setColorCount(d->m_char->ColorPalette().size());
        QList<QRgb> colors;
        for(uint i = 0; i < d->m_char->ColorPalette().size(); i++)
        {
            QRgb color = qRgb(d->m_char->ColorPalette()[i].rgbRed,
                              d->m_char->ColorPalette()[i].rgbGreen,
                              d->m_char->ColorPalette()[i].rgbBlue);
            colors.append(color);
        }

        frameImage.setColorTable(colors);
        frameImage = frameImage.convertToFormat(QImage::Format_ARGB32);
        QRgb maskColor = qRgb(d->m_char->TransparentColor().rgbRed,
                              d->m_char->TransparentColor().rgbGreen,
                              d->m_char->TransparentColor().rgbBlue);

        QImage mask = frameImage.createMaskFromColor(maskColor, Qt::MaskOutColor);
        frameImage.setAlphaChannel(mask);
        frameImage.flip();
        // TODO: hacky, removing the last line from the frame
        // because it contains a weird artifact (from padding???)
        auto newimg = frameImage.copy(0,0,img->Width(), img->Height()-1);
        p.drawImage(QPoint(offsetX,offsetY), newimg);
    }
}

void CharacterWindow::playSoundEffect(libacsfile::Sound *sound)
{
    if (sound->Size() < 44) {
        CHAR_LOG("Invalid RIFF data (too small)");
        return;
    }

    QByteArray wavData(reinterpret_cast<const char*>(sound->Data().data()),
                       static_cast<int>(sound->Size()));

    const char *data = wavData.constData();

    // Parse WAV header (simple PCM)
    quint16 audioFormat   = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(data + 20));
    quint16 numChannels   = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(data + 22));
    quint32 sampleRate    = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(data + 24));
    quint16 bitsPerSample = qFromLittleEndian<quint16>(reinterpret_cast<const uchar*>(data + 34));

    if (audioFormat != 1)
    {
        CHAR_LOG(QString("Unsupported PCM Format format: %1").arg(QString::number(audioFormat)));
        return;
    }

    int dataPos = wavData.indexOf("data");
    if (dataPos == -1) {
        CHAR_LOG("No data chunk in RIFF block");
        return;
    }

    int pcmStart = dataPos + 8; // skip "data" and its size field
    QByteArray pcm = wavData.mid(pcmStart);

    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(numChannels);
    format.setSampleFormat(bitsPerSample == 16 ? QAudioFormat::Int16
                                               : QAudioFormat::UInt8);

    QBuffer *buffer = new QBuffer(this);
    buffer->setData(pcm);
    buffer->open(QIODevice::ReadOnly);

    AudioSink *audioOutput = new AudioSink(format, this);
    audioOutput->start(buffer);

    QObject::connect(audioOutput, &AudioSink::stateChanged, this, [audioOutput, buffer](QAudio::State state) {
        if (state == QAudio::IdleState) {
            audioOutput->stop();
            buffer->close();
            audioOutput->deleteLater();
            buffer->deleteLater();
        }
    });
}
