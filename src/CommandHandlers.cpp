/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ImageViewerApplication.h"
#include "MainWindow.h"
#include <sstream>

void ImageViewerApplication::handle_load(const QStringList &args){
	if (args.size() < 4)
		return;
	this->main_window->load(args[2], args[3].toStdString());
}

void ImageViewerApplication::handle_scale(const QStringList &args){
	if (args.size() < 4)
		return;
	auto name = args[2].toStdString();
	auto &scale_string = args[3];
	bool ok;
	auto scale = scale_string.toDouble(&ok);
	if (!ok)
		return;
	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->set_scale(scale);
}

void ImageViewerApplication::handle_setorigin(const QStringList &args){
	if (args.size() < 5)
		return;
	auto name = args[2].toStdString();
	auto &x_string = args[3];
	auto &y_string = args[4];
	bool ok;
	auto x = x_string.toInt(&ok);
	if (!ok)
		return;
	auto y = y_string.toInt(&ok);
	if (!ok)
		return;
	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->set_origin(x, y);
}

bool parse_move_string(int &dst, bool &relative, const QString &s){
	if (!s.size())
		return false;
	int mul = 1;
	if (s[0] == '+'){
		relative = true;
	}else if (s[0] == '-'){
		relative = true;
		mul = -1;
	}else
		relative = false;

	bool ok;
	if (relative)
		dst = s.mid(1).toInt(&ok);
	else
		dst = s.toInt(&ok);
	if (!ok)
		return false;
	
	dst *= mul;
	return true;
}

bool parse_rotate_string(double &dst, bool &relative, const QString &s){
	if (!s.size())
		return false;
	double mul = 1;
	if (s[0] == '+'){
		relative = true;
	}else if (s[0] == '-'){
		relative = true;
		mul = -1;
	}else
		relative = false;

	bool ok;
	if (relative)
		dst = s.mid(1).toDouble(&ok);
	else
		dst = s.toDouble(&ok);
	if (!ok)
		return false;

	dst *= mul;
	return true;
}

void ImageViewerApplication::handle_move(const QStringList &args){
	if (args.size() < 5)
		return;
	auto name = args[2].toStdString();
	auto &x_string = args[3];
	auto &y_string = args[4];
	int x, y;
	bool xrel, yrel;
	if (!parse_move_string(x, xrel, x_string))
		return;
	if (!parse_move_string(y, yrel, y_string))
		return;

	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	auto pos = window->pos();
	if (xrel)
		pos.setX(pos.x() + x);
	else
		pos.setX(x);
	if (yrel)
		pos.setY(pos.y() + y);
	else
		pos.setY(y);
	window->move_by_command(pos);
}

void ImageViewerApplication::handle_rotate(const QStringList &args){
	if (args.size() < 4)
		return;
	auto name = args[2].toStdString();
	auto &theta_string = args[3];
	double theta;
	bool thetarel;
	if (!parse_rotate_string(theta, thetarel, theta_string))
		return;

	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	auto rotation = window->get_rotation();
	if (thetarel)
		rotation += theta;
	else
		rotation = theta;
	window->set_rotation(rotation);
}

void ImageViewerApplication::handle_animmove(const QStringList &args){
	if (args.size() < 6)
		return;
	auto name = args[2].toStdString();
	bool ok;
	auto x = args[3].toInt(&ok);
	if (!ok)
		return;
	auto y = args[4].toInt(&ok);
	if (!ok)
		return;
	auto speed = args[5].toDouble(&ok);
	if (!ok)
		return;

	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->anim_move(x, y, speed);
}

void ImageViewerApplication::handle_animrotate(const QStringList &args){
	if (args.size() < 4)
		return;
	auto name = args[2].toStdString();
	bool ok;
	auto speed = args[3].toDouble(&ok);
	if (!ok)
		return;

	auto window = this->main_window->get_window(name);
	if (!window)
		return;
	window->anim_rotate(speed);
}

void ImageViewerApplication::handle_loadscript(const QStringList &){
}
