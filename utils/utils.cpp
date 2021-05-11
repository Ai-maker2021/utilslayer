#include "utilslayer.h"
#include <QUuid>
#include <QImageReader>
#include <QCryptographicHash>

QString getUuid() {
    QUuid id = QUuid::createUuid();
    return id.toString();
}

QString generatePersonSerial() {
    QString uuid = getUuid();
    return uuid.mid(1,uuid.size()-2);
}

QString md5(const QString &string) {
    QString md5;
    QByteArray bb = QCryptographicHash::hash(string.toUtf8(),QCryptographicHash::Md5);
    return md5.append(bb.toHex());
}

int saveImage(const QImage& image, const QString& imageName, int quality) {
    return image.save(imageName,nullptr,quality);
}

int saveCropImage(const QString& imagePath, const QString& imageName, QRect faceRect, const QString& savePath) {
    QImage image = readQImage(imagePath+"/"+imageName);

    if (image.isNull()){
        return -1;
    }

    int length = qMax(faceRect.width(),faceRect.height())/2;
    faceRect.adjust(-length,-length,length,length);
    QRect rc = faceRect  & image.rect();
    rc.setWidth(rc.width()&~3);
    rc.setHeight(rc.height()&~3);

    QImage cropImage = image.copy(rc);

    return cropImage.save(savePath);
}

QImage readQImage(const QString &path){
    QImageReader reader(path);
    reader.setAutoTransform(true);
    reader.setDecideFormatFromContent(true);
    QSize size = reader.size();
    int length = qMax(size.width(),size.height());

    if (length > MAX_IMAGE_LENGTH) {
        int width = size.width();
        int height = size.height();
        int newWidth;
        int newHeight;
        if (width>height){
            newWidth = MAX_IMAGE_LENGTH;
            newHeight = (int)(height*1.0*MAX_IMAGE_LENGTH/width)&~3;
        } else {
            newHeight = MAX_IMAGE_LENGTH;
            newWidth = (int)(width*1.0*MAX_IMAGE_LENGTH/height)&~3;
        }
        QSize size = QSize(newWidth,newHeight);

        reader.setScaledSize(size);
        reader.setScaledClipRect(QRect(QPoint(0,0),size));
    } else {
        int width = size.width()&~3;
        int height = size.height()&~3;
        reader.setClipRect(QRect(0,0,width,height));
    }

    return reader.read();
}


QImage fitinQImage(const QString& path, QSize &size){
    QImageReader reader(path);
    reader.setDecideFormatFromContent(true);
    QSize imageSize = reader.size();
    imageSize.scale(size,Qt::KeepAspectRatio);
    reader.setScaledSize(imageSize);
    size = imageSize;
    return reader.read();
}


