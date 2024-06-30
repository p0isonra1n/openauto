/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QLabel>
#include <f1x/openauto/autoapp/Projection/QtVideoOutput.hpp>
#include <f1x/openauto/Common/Log.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

QtVideoOutput::QtVideoOutput(configuration::IConfiguration::Pointer configuration)
    : VideoOutput(std::move(configuration))
{
    this->moveToThread(QApplication::instance()->thread());
    connect(this, &QtVideoOutput::startPlayback, this, &QtVideoOutput::onStartPlayback, Qt::QueuedConnection);
    connect(this, &QtVideoOutput::stopPlayback, this, &QtVideoOutput::onStopPlayback, Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "createVideoOutput", Qt::BlockingQueuedConnection);
}

void QtVideoOutput::createVideoOutput()
{
    OPENAUTO_LOG(debug) << "[QtVideoOutput] create.";
    videoWidget_ = std::make_unique<QVideoWidget>();
    mediaPlayer_ = std::make_unique<QMediaPlayer>(nullptr, QMediaPlayer::StreamPlayback);
    widget_ = std::make_unique<QWidget>();
    textLabel_ = std::make_unique<QLabel>("", nullptr);
}


bool QtVideoOutput::open()
{
    return videoBuffer_.open(QIODevice::ReadWrite);
}

bool QtVideoOutput::init()
{
    emit startPlayback();
    return true;
}

void QtVideoOutput::stop()
{
    emit stopPlayback();
}

void QtVideoOutput::write(uint64_t, const aasdk::common::DataConstBuffer& buffer)
{
    videoBuffer_.write(reinterpret_cast<const char*>(buffer.cdata), buffer.size);
}

void QtVideoOutput::onStartPlayback()
{
    widget_->showFullScreen();
    widget_->setFixedSize(1920, 1080);
    widget_->adjustSize();

    QPalette palette;
    palette.setBrush(widget_->backgroundRole(), QBrush(QPixmap("./bg.png")));
    widget_->setPalette(palette);
    widget_->setAutoFillBackground(true);

    videoWidget_->setFixedWidth(1920);
    videoWidget_->setFixedHeight(750);

    QVBoxLayout *mainLayout = new QVBoxLayout();

    textLabel_->setText("12:34");
    QFont f( "Arial", 50, QFont::Bold);
    textLabel_->setFont(f);
    QPalette sample_palette;
    sample_palette.setColor(QPalette::Window, Qt::transparent);
    sample_palette.setColor(QPalette::WindowText, Qt::white);

    textLabel_->setAutoFillBackground(true);
    textLabel_->setPalette(sample_palette);

    QHBoxLayout *textLayout = new QHBoxLayout();
    textLayout->addStretch();  // Add stretchable space before the label
    textLayout->addWidget(textLabel_.get());
    textLayout->addStretch();  // Add stretchable space after the label

    // Add the horizontal layout for the text label to the main layout
    mainLayout->addLayout(textLayout);

    // Add a spacer item to take up the space between the text label and the video widget
    mainLayout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // Create a horizontal layout to center the video widget
    QHBoxLayout *videoLayout = new QHBoxLayout();
    videoLayout->addStretch();  // Add stretchable space before the widget
    videoLayout->addWidget(videoWidget_.get());
    videoLayout->addStretch();  // Add stretchable space after the widget

    // Add the horizontal layout for the video widget to the main layout
    mainLayout->addLayout(videoLayout);
    widget_->setLayout(mainLayout);
    videoWidget_->setAspectRatioMode(Qt::IgnoreAspectRatio);
    videoWidget_->setFocus();
    //videoWidget_->setWindowFlags(Qt::WindowStaysOnTopHint);
    //videoWidget_->setFullScreen(true);
    //videoWidget_->setWindowFlags(Qt::FramelessWindowHint);
    //videoWidget_->setGeometry(75, 550, 1720, 550);
    //videoWidget_->show();


    mediaPlayer_->setVideoOutput(videoWidget_.get());
    mediaPlayer_->setMedia(QMediaContent(), &videoBuffer_);
    mediaPlayer_->play();
    OPENAUTO_LOG(debug) << "Player error state -> " << mediaPlayer_->errorString().toStdString();
}

void QtVideoOutput::onStopPlayback()
{
    videoWidget_->hide();
    mediaPlayer_->stop();
}

}
}
}
}
