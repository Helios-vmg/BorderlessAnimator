/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "LoadedImage.h"
#include "DirectoryListing.h"
#include <QImage>
#include <QtConcurrent/QtConcurrentRun>
#include <QLabel>

extern const char *supported_extensions[];

LoadedImage::LoadedImage(ImageViewerApplication &app, const QString &path){
	auto img = app.load_image(path);
	if ((this->null = img.isNull())){
		//Weird QImage behavior. Passing "*" allows it to load images where the
		//file extension doesn't match the file contents.
		img = QImage(path, "*");
		if ((this->null = img.isNull()))
			return;
	}
	this->compute_average_color(img);
	this->image = QtConcurrent::run([](QImage img){ return QPixmap::fromImage(img); }, img);
	this->size = img.size();
	this->alpha = img.hasAlphaChannel();
}

LoadedImage::LoadedImage(const QImage &image){
	this->compute_average_color(image);
	this->image = QtConcurrent::run([](QImage img){ return QPixmap::fromImage(img); }, image);
	this->size = image.size();
	this->alpha = image.hasAlphaChannel();
}

LoadedImage::~LoadedImage(){
	this->background_color.cancel();
}

QColor get_average_color(QImage src){
	if (src.depth() < 32)
		src = src.convertToFormat(QImage::Format_ARGB32);
	quint64 avg[3] = {0};
	unsigned pixel_count=0;
	for (auto y = src.height() * 0; y < src.height(); y++){
		const uchar *p = src.constScanLine(y);
		for (auto x = src.width() * 0; x < src.width(); x++){
			avg[0] += quint64(p[2]) * quint64(p[3]) / 255;
			avg[1] += quint64(p[1]) * quint64(p[3]) / 255;
			avg[2] += quint64(p[0]) * quint64(p[3]) / 255;
			p += 4;
			pixel_count++;
		}
	}
	for (int a = 0; a < 3; a++)
		avg[a] /= pixel_count;
	return QColor(avg[0], avg[1], avg[2]);
}

QColor background_color_parallel_function(QImage img){
	QColor avg = get_average_color(img),
		negative = avg,
		background;
	negative.setRedF(1 - negative.redF());
	negative.setGreenF(1 - negative.greenF());
	negative.setBlueF(1 - negative.blueF());
	if (negative.saturationF() <= .05 && negative.valueF() >= .45 && negative.valueF() <= .55)
		background = Qt::white;
	else
		background = negative;
	return background;
}

void LoadedImage::compute_average_color(QImage img){
	this->background_color = QtConcurrent::run(background_color_parallel_function, img);
}

void LoadedImage::assign_to_QLabel(QLabel &label){
	//label.setPixmap(this->image);
}

QImage LoadedImage::get_QImage() const{
	return ((QPixmap)this->image).toImage();
}

LoadedAnimation::LoadedAnimation(ImageViewerApplication &app, const QString &path){
	this->animation = app.load_animation(path);
	this->null = !this->animation->isValid();
	if (!this->null){
		(void)this->animation->jumpToNextFrame();
		this->size = this->animation->currentPixmap().size();
		this->alpha = true;
	}
}

void LoadedAnimation::assign_to_QLabel(QLabel &label){
	label.setMovie(this->animation.get());
	this->animation->start();
}

QImage LoadedAnimation::get_QImage() const{
	return this->animation->currentImage();
}

std::unique_ptr<LoadedGraphics> LoadedGraphics::create(ImageViewerApplication &app, const QString &path){
	std::unique_ptr<LoadedGraphics> ret;
	if (app.is_animation(path))
		ret.reset(new LoadedAnimation(app, path));
	else
		ret.reset(new LoadedImage(app, path));
	return ret;
}
