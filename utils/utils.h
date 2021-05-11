#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QImage>

#define MAX_IMAGE_LENGTH 1024

int saveCropFace(const QString& imagePath, const QString& imageName, QRect faceRect, const QString& savePath);

QImage readImage(const QString& path);

QImage fitintImage(const QString& path, QSize& size);


#endif // UTILS_H
