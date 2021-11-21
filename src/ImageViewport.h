/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef IMAGEVIEWPORT_H
#define IMAGEVIEWPORT_H

#include "Quadrangular.h"
#include "Settings.h"
#include "Script.h"
#include <chrono>
#include <functional>
#include <QLabel>
#include <QImage>
#include <QMatrix>
#include <QTimer>

class LoadedGraphics;

class ImageViewport : public QLabel
{
	Q_OBJECT
	std::unique_ptr<LoadedGraphics> image;
	QPointF origin;
	QPointF translation;
	double rotation = 0;
	double zoom = 1;
	bool flip_h = false;
	bool flip_v = false;
	QSize image_size;
	bool update_transform = false;
	QMatrix transform;
	
	std::string name;

	class Animator{
	protected:
		ImageViewport *image;
		bool active;
		std::chrono::time_point<std::chrono::high_resolution_clock> t0;
	public:
		Animator(ImageViewport &image){
			this->image = &image;
			this->active = true;
			this->t0 = std::chrono::high_resolution_clock::now();
		}
		virtual ~Animator(){}
		bool get_active() const{
			return this->active;
		}
		virtual bool resume() = 0;
		double elapsed() const;
	};
	
	class MoveAnimator : public Animator{
		QPointF src;
		QPointF dst;
		double duration;
		std::function<void()> on_complete;
	public:
		MoveAnimator(ImageViewport &image, QPointF src, QPointF dst, double duration, std::function<void()> &&on_complete)
			: Animator(image)
			, src(src)
			, dst(dst)
			, duration(duration)
			, on_complete(on_complete){}
		bool resume() override;
	};
	
	class RotateAnimator : public Animator{
		double rotation0;
		double speed;
	public:
		RotateAnimator(ImageViewport &image, double r0, double s)
			: Animator(image)
			, rotation0(r0)
			, speed(s){}
		bool resume() override;
	};

	std::unique_ptr<QTimer> timer;
	QMetaObject::Connection timer_connection;

	std::unique_ptr<MoveAnimator> move_animator;
	std::unique_ptr<RotateAnimator> rotate_animator;
	std::unique_ptr<Script> script;

	QMatrix get_transform(){
		if (!this->update_transform)
			return this->transform;
		this->update_transform = false;
		auto first = QMatrix().translate(-this->origin.x(), -this->origin.y());
		auto second = QMatrix().rotate(this->rotation).scale(this->zoom * (this->flip_h ? -1 : 1), this->zoom * (this->flip_v ? -1 : 1));;
		return this->transform = first * second * QMatrix().translate(this->translation.x(), this->translation.y());
	}
	void check_timer();
public:
	explicit ImageViewport(QWidget *parent = 0);
	explicit ImageViewport(std::string &&name, const QSize &size, QWidget *parent = 0);
	QSize get_image_size() const{
		return this->image_size;
	}

	void paintEvent(QPaintEvent *) override;
	void set_image(std::unique_ptr<LoadedGraphics> &&li, const QSize &geom);
	const std::string &get_name() const{
		return this->name;
	}

	void move_by_command(const QPointF &);
	void set_scale(double scale);
	void set_origin(int x, int y);
	double get_rotation() const{
		return this->rotation;
	}
	QPointF get_position() const{
		return this->translation;
	}
	void set_rotation(double theta);
	void anim_move(int x, int y, double speed, std::function<void()> &&f = {});
	void anim_rotate(double speed);
	void fliph();
	void flipv();
	void load_script(const QString &path);

public slots:
	void timer_timeout();
};

template <typename T>
QPointF set(const QPointF &pos, const relabsint &x, const std::pair<T, bool> &y){
	auto ret = pos;
	if (x.second)
		ret.setX(ret.x() + x.first);
	else
		ret.setX(x.first);
	if (y.second)
		ret.setY(ret.y() + y.first);
	else
		ret.setY(y.first);
	return ret;
}

#endif // IMAGEVIEWPORT_H
