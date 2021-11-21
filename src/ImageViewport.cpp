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
	return{ (int)round(p.x()), (int)round(p.y()) };
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

void ImageViewport::anim_move(int x, int y, double speed, std::function<void()> &&f){
	if (!speed){
		this->move_animator.reset();
		this->check_timer();
		return;
	}
	auto duration = norm(QPointF(x, y) - this->translation) / speed;
	this->move_animator.reset(new MoveAnimator(*this, this->translation, QPointF(x, y), duration, std::move(f)));
	this->check_timer();
}

void ImageViewport::anim_rotate(double speed){
	if (!speed){
		this->rotate_animator.reset();
		this->check_timer();
		return;
	}
	this->rotate_animator.reset(new RotateAnimator(*this, this->rotation, speed * (360.0 / 60.0)));
	this->check_timer();
}

void ImageViewport::check_timer(){
	if (!this->move_animator && !this->rotate_animator && !this->script){
		if (this->timer_connection)
			this->disconnect(this->timer_connection);
		this->timer.reset();
		return;
	}
	if (this->timer)
		return;
	this->timer = std::make_unique<QTimer>(this);
	this->timer->setTimerType(Qt::PreciseTimer);
	this->timer_connection = connect(this->timer.get(), &QTimer::timeout, this, &ImageViewport::timer_timeout);
	this->timer->start(10);
}


double ImageViewport::Animator::elapsed() const{
	return (double)(T::now() - this->t0).count() * T::period::num / T::period::den;
}

bool ImageViewport::MoveAnimator::resume(){
	auto t = this->elapsed();
	t /= this->duration;
	if (t >= 1){
		this->image->move_by_command(to_QPoint(this->dst));
		this->active = false;
		if (this->on_complete)
			this->on_complete();
		return false;
	}
	auto pos = this->src * (1 - t) + this->dst * t;
	this->image->move_by_command(to_QPoint(pos));
	return true;
}

bool ImageViewport::RotateAnimator::resume(){
	auto elapsed_seconds = this->elapsed();
	this->image->set_rotation(this->rotation0 + this->speed * elapsed_seconds);
	return true;
}

void ImageViewport::fliph(){
	this->flip_h = !this->flip_h;
	this->update_transform = true;
	this->repaint();
}

void ImageViewport::flipv(){
	this->flip_v = !this->flip_v;
	this->update_transform = true;
	this->repaint();
}

void ImageViewport::load_script(const QString &path){
	std::unique_ptr<Script> script;
	try{
		script = std::make_unique<Script>(path);
	}catch (std::exception &e){
		//TODO: Do something with the error.
		return;
	}
	this->script = std::move(script);
	this->check_timer();
}

void ImageViewport::timer_timeout(){
	if (this->move_animator && !this->move_animator->resume())
		this->move_animator.reset();
	if (this->rotate_animator && !this->rotate_animator->resume())
		this->rotate_animator.reset();
	if (this->script && !this->script->resume(*this))
		this->script.reset();
	this->check_timer();
}
