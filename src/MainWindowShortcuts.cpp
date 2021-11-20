/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "MainWindow.h"
#include "ui_MainWindow.h"

void MainWindow::quit_slot(){
	this->app->quit();
}

void MainWindow::background_swap_slot(){
}

void MainWindow::close_slot(){
}

void MainWindow::zoom_in_slot(){
}

void MainWindow::zoom_out_slot(){
}

void MainWindow::up_slot(){
}

void MainWindow::down_slot(){
}

void MainWindow::left_slot(){
}

void MainWindow::right_slot(){
}

void MainWindow::up_big_slot(){
}

void MainWindow::down_big_slot(){
}

void MainWindow::left_big_slot(){
}

void MainWindow::right_big_slot(){
}

void MainWindow::offset_image(const QPoint &offset){
}

void MainWindow::reset_zoom_slot(){
}

void cycle_zoom_mode(ZoomMode &mode){
}

void MainWindow::cycle_zoom_mode_slot(){
}

void toggle_lock_zoom(ZoomMode &mode){
}

void MainWindow::toggle_lock_zoom_slot(){
}

void MainWindow::toggle_fullscreen(){
}

void MainWindow::rotate(bool right, bool fine){
}

void MainWindow::rotate_left(){
}

void MainWindow::rotate_right(){
}

void MainWindow::rotate_left_fine(){
}

void MainWindow::rotate_right_fine(){
}

void MainWindow::minimize_slot(){
}

void MainWindow::minimize_all_slot(){
}

void MainWindow::flip_h(){
}

void MainWindow::flip_v(){
}
