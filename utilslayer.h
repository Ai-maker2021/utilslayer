#ifndef ARCFACIAL_H
#define ARCFACIAL_H

#include <QString>
#include <QImage>

#define MAX_IMAGE_LENGTH 1024

Q_DECL_EXPORT int saveImage(const QImage& image, const QString& imageName, int quality = -1);

Q_DECL_EXPORT int saveCropImage(const QString& imagePath, const QString& imageName, QRect faceRect, const QString& savePath);

Q_DECL_EXPORT QImage readQImage(const QString& path);

Q_DECL_EXPORT QImage fitinQImage(const QString& path, QSize& size);

#endif // ARCFACIAL_H
