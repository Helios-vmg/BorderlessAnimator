/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ImageViewport.h"
#include "LoadedImage.h"
#include <QPaintEvent>
#include <QPainter>

ImageViewport::ImageViewport(QWidget *parent): QLabel(parent){
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

ImageViewport::ImageViewport(std::string &&name, const QSize &size, QWidget *parent): QLabel(parent), name(std::move(name)){
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->setScaledContents(true);
}

void ImageViewport::flip(bool hor){
	auto x = hor ? -1.0 : 1.0;
	auto y = -x;
	//TODO
}

template <typename T>
typename std::enable_if<std::is_enum<T>::value, T>::type or_flags(const T &a, const T &b){
	return (T)((unsigned)a | (unsigned)b);
}

QMatrix translate(const QMatrix &m, const QPointF &offset){
	return QMatrix(m.m11(), m.m12(), m.m21(), m.m22(), m.dx() + offset.x(), m.dy() + offset.y());
}

void ImageViewport::paintEvent(QPaintEvent *ev){
	QPainter painter(this);
	if (!this->pixmap() && !this->movie()){
		return;
		painter.setBrush(QBrush(Qt::white));
		auto font = painter.font();
		font.setPixelSize(48);
		painter.setFont(font);
		QRect rect(0, 0, 800, 600);
		painter.fillRect(rect, Qt::black);
		painter.drawText(rect, "No image");
		return;
	}

	painter.setRenderHint(or_flags(QPainter::SmoothPixmapTransform, QPainter::Antialiasing));
	painter.setClipping(false);

	painter.setMatrix(this->get_transform());
#if 0
	painter.drawPixmap(QRect(QPoint(0, 0), this->image_size), static_cast<LoadedAnimation &>(*this->image).get_movie().currentPixmap());
#else
	if (this->pixmap())
		painter.drawPixmap(QRect(QPoint(0, 0), this->image_size), *this->pixmap());
	else{
		painter.drawPixmap(QRect(QPoint(0, 0), this->image_size), this->movie()->currentPixmap());
	}
#endif
}

void ImageViewport::set_image(std::unique_ptr<LoadedGraphics> &&li, const QSize &geom){
	this->image = std::move(li);
	this->image_size = this->image->get_size();
	this->image->assign_to_QLabel(*this);
	auto s = this->image_size;
	this->resize(geom);
	this->move(0, 0);
}

QPoint to_QPoint(const QPointF &p){
	return{ (int)floor(p.x()), (int)floor(p.y()) };
}

void ImageViewport::move_by_command(const QPointF &p){
	this->translation = p;
	this->update_transform = true;
	this->repaint();
}

void ImageViewport::set_scale(double scale){
	this->zoom = scale;
	this->update_transform = true;
	this->repaint();
}

void ImageViewport::set_origin(int x, int y){
	QPointF new_origin(x, y);
	auto delta = new_origin - this->origin;
	delta = this->get_transform().map(delta) - this->translation;
	this->translation += delta;
	this->origin = new_origin;
	this->update_transform = true;
}

void ImageViewport::set_rotation(double theta){
	this->rotation = theta;
	this->update_transform = true;
	this->repaint();
}

typedef std::chrono::high_resolution_clock T;

void ImageViewport::anim_move(int x, int y, double speed){
	this->move_src = this->translation;
	this->move_dst = QPointF(x, y);
	auto l = norm(this->move_dst - this->move_src);
	auto duration = l / speed;
	this->move_t0 = T::now();
	this->move_duration = duration;
	if (this->move_connection)
		this->disconnect(this->move_connection);
	this->move_timer = std::make_unique<QTimer>(this);
	this->move_timer->setTimerType(Qt::PreciseTimer);
	this->move_connection = connect(this->move_timer.get(), &QTimer::timeout, this, &ImageViewport::move_timeout);
	this->move_timer->start(10);
}

void ImageViewport::anim_rotate(double speed){
	if (!speed){
		if (this->rotate_timer){
			if (this->rotate_connection)
				this->disconnect(this->rotate_connection);
			this->rotate_timer.reset();
		}
		return;
	}
	this->rotate_rotation0 = this->rotation;
	this->rotate_speed = speed * (360.0 / 60.0);
	this->rotate_t0 = T::now();
	if (!this->rotate_timer){
		this->rotate_timer = std::make_unique<QTimer>(this);
		this->rotate_timer->setTimerType(Qt::PreciseTimer);
		this->rotate_connection = connect(this->rotate_timer.get(), &QTimer::timeout, this, &ImageViewport::rotate_timeout);
		this->rotate_timer->start(10);
	}
}

void ImageViewport::move_timeout(){
	auto t = (double)(T::now() - this->move_t0).count() * T::period::num / T::period::den;
	t /= this->move_duration;
	if (t >= 1){
		this->move_by_command(to_QPoint(this->move_dst));
		this->disconnect(this->move_connection);
		this->move_timer.reset();
		return;
	}
	auto pos = (QPointF)this->move_src * (1 - t) + (QPointF)this->move_dst * t;
	this->move_by_command(to_QPoint(pos));
}

void ImageViewport::rotate_timeout(){
	auto elapsed_seconds = (double)(T::now() - this->rotate_t0).count() * T::period::num / T::period::den;
	this->set_rotation(this->rotate_rotation0 + this->rotate_speed * elapsed_seconds);
}
