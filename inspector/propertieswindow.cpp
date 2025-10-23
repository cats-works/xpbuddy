#include "propertieswindow.h"
#include "renderer.h"

#include <QVBoxLayout>
#include <QListWidget>
#include <QCloseEvent>
#include <QLabel>

PropertiesWindow::PropertiesWindow(const QString &filename, QWidget *parent)
    : QDialog(parent)
    , m_render(new CharacterWindow(filename))
    , m_tabs(new QTabWidget(this))
    , m_animationList(new QListWidget(this))
    , m_stateList(new QListWidget(this))
{
    setWindowFlag(Qt::Dialog, false);
    setMinimumSize(150,200);


    auto lblDescription = new QLabel(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(lblDescription);
    layout->addWidget(m_tabs);
    m_tabs->addTab(m_stateList, tr("States"));
    m_tabs->addTab(m_animationList, tr("Animations"));

    lblDescription->setWordWrap(true);
    if(m_render->isLoaded())
    {
        lblDescription->setText(QString::fromStdString(m_render->Character()->Description()));
        for(auto &i : m_render->Character()->States())
            m_stateList->addItem(QString::fromStdString(i.first));

        for(auto &i : m_render->Character()->AnimationNames())
            m_animationList->addItem(QString::fromStdString(i));

        connect(m_animationList, &QListWidget::itemActivated, this, [this](QListWidgetItem *item) {
            m_render->Animate(item->text());
        });

        setWindowTitle(QString::fromStdString(m_render->Character()->Name()));
        setWindowFilePath(filename);

        m_render->show();
    }
}

bool PropertiesWindow::isLoaded()
{
    if(m_render)
        return m_render->isLoaded();

    return false;
}

QString PropertiesWindow::getLastError() const
{
    if(m_render)
        return m_render->getLastError();

    return QString();
}

void PropertiesWindow::closeEvent(QCloseEvent *event)
{
    if(!m_hiding)
    {
        event->ignore();
        m_animationList->setDisabled(true);
        connect(m_render, &CharacterWindow::animationCompleted, [this](){
            close();
        });
        m_render->Animate("Hide");
        m_hiding = true;
    }
    else
    {
        QDialog::closeEvent(event);
        event->accept();
        m_render->close();
    }
}
