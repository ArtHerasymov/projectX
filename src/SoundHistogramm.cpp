#include "SoundHistogramm.h"
#include "ui_SoundHistogramm.h"
#include <QPainter>
#include <QPointF>
#include <climits>
#include <iostream>
using namespace std;

double getPoint8(char *data, int heigth) {
  double val = 0;
  int max = CHAR_MAX;
  val = data[0];
  val /= max;
  val /= 2.1;
  return heigth / 2 + val * heigth;
}

double getPoint16(short *data, int heigth) {
  double val = 0;
  int max = SHRT_MAX;
  val = data[0];
  val /= max;
  val /= 2.1;
  return heigth / 2 + val * heigth;
}

QPixmap *createImage(const AudioData *sound) {
  QPixmap *image = new QPixmap(1920, 500);
  image->fill(QColor(255, 255, 255, 0));
  QPainter painter(image);

  painter.setBrush(Qt::NoBrush);
  painter.drawRect(0, 0, image->width(), image->height());
  painter.setRenderHint(QPainter::Antialiasing, true);

  double oldPos = image->height() / 2;
  double currPos = 0;
  int width = image->width() - image->height() / 3;
  int range = sound->get_samples() / width;
  int channels = sound->get_channels();
  int imHeigth = image->height();
  painter.setPen(QPen(QColor(255, 255, 255), 2, Qt::SolidLine, Qt::RoundCap,
                      Qt::RoundJoin));

  if (sound->get_bitsPerSample() == 16) {
    short *data = (short *)sound->get_source();
    for (int i = image->height() / 3, j = 0; i < image->width(); i++, j++) {
      currPos = getPoint16(data + j * range * channels, imHeigth);
      painter.drawLine(i - 1, oldPos, i, currPos);
      oldPos = currPos;
    }
  } else {
    char *data = (char *)sound->get_source();
    for (int i = image->height() / 3, j = 0; i < image->width(); i++, j++) {
      currPos = getPoint8(data + j * range * channels, imHeigth);
      painter.drawLine(i - 1, oldPos, i, currPos);
      oldPos = currPos;
    }
  }

  return image;
}

SoundHistogramm::SoundHistogramm(AudioPlayer *player, AudioData *sound,
                                 QWidget *parent)
    : QWidget(parent), ui(new Ui::SoundHistogramm) {
  ui->setupUi(this);
  this->timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(updateHistogramm()));
  timer->setInterval(300);
  timer->start();
  this->image = createImage(sound);
  this->player = player;
  this->sound = sound;
  this->remainingTime = sound->get_samples() / sound->get_sampleRate() * 1000;
}

SoundHistogramm::~SoundHistogramm() {
  timer->stop();
  delete image;
  delete timer;
  delete ui;
}

QSize SoundHistogramm::minimumSizeHint() const { return QSize(100, 150); }

QSize SoundHistogramm::sizeHint() const { return QSize(200, 150); }

void SoundHistogramm::setPen(const QPen &pen) { this->pen = pen; }

void SoundHistogramm::setBrush(const QBrush &brush) { this->brush = brush; }

void SoundHistogramm::setAntialiased(bool antialiased) {
  this->antialiased = antialiased;
}

void SoundHistogramm::updateHistogramm() { update(); }

void SoundHistogramm::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setBrush(this->brush);
  painter.setPen(this->pen);
  painter.setRenderHint(QPainter::Antialiasing, antialiased);
  painter.drawPixmap(this->rect(), *image);

  double circleRadius = height() / 2.5;
  double center = height() / 2;
  double progress = (double)player->getTime() / remainingTime;
  QRectF progressRect(center, center - circleRadius,
                      center + (width() - center) * progress - circleRadius,
                      circleRadius * 2);
  painter.drawRect(progressRect);
  // static ellips
  painter.drawEllipse(QPointF(center, center), circleRadius, circleRadius);
  painter.drawEllipse(QPointF(progressRect.x() + progressRect.width(), center),
                      circleRadius, circleRadius);

  painter.setFont(QFont("Ubuntu", 35));
  painter.setPen(QPen(QColor(255, 255, 255)));
  painter.drawText(progressRect.x(), 0, progressRect.x() + progressRect.width(),
                   height(), Qt::AlignHCenter | Qt::AlignVCenter,
                   QString::number((int)(progress * 100)) + "%");
}