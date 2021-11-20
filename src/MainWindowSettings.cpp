/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDir>

void MainWindow::reset_settings(){
	auto &state = this->window_state;
	if (!state)
		state = std::make_shared<WindowState>();
	WindowState defaults;
	state->set_using_checkerboard_pattern(this->app->get_use_checkerboard_pattern());
	state->set_movement_size(defaults.get_movement_size());
	state->set_zoom_mode(this->app->get_zoom_mode_for_new_windows());
	state->set_fullscreen_zoom_mode(this->app->get_fullscreen_zoom_mode_for_new_windows());
}
