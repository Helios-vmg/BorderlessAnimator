/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef IMAGEVIEWERAPPLICATION_H
#define IMAGEVIEWERAPPLICATION_H

#include "SingleInstanceApplication.h"
#include "Settings.h"
#include "Enums.h"
#include <QMenu>
#include <memory>
#include <exception>
#include <QSystemTrayIcon>
#include <QWindow>
#include <map>

class QAction;
class CustomProtocolHandler;
struct lua_State;

class NoWindowsException : public std::exception{};

class ImageViewerApplication : public SingleInstanceApplication{
	Q_OBJECT

	typedef std::shared_ptr<MainWindow> sharedp_t;
	sharedp_t main_window;
	std::vector<std::shared_ptr<QAction>> actions;
	MainWindow *context_menu_last_requester;
	QString config_location,
		config_filename,
		state_filename;

	MainSettings settings;

	QSystemTrayIcon tray_icon;
	std::shared_ptr<QMenu> tray_context_menu,
		last_tray_context_menu;
	QByteArray last_saved_settings_digest;
	QByteArray last_saved_state_digest;
	typedef void (ImageViewerApplication::*command_handler_t)(const QStringList &);
	std::map<std::string, command_handler_t> command_handlers;

	QString get_config_location();
	QString get_config_subpath(QString &dst, const char *sub);
	QString get_settings_filename();
	QString get_state_filename();
	void setup_slots();
	void reset_tray_menu();
	void setup_command_handlers();

	void handle_load(const QStringList &);
	void handle_scale(const QStringList &);
	void handle_setorigin(const QStringList &);
	void handle_move(const QStringList &);
	void handle_rotate(const QStringList &);
	void handle_animmove(const QStringList &);
	void handle_animrotate(const QStringList &);
	void handle_loadscript(const QStringList &);

protected:
	void new_instance(const QStringList &args) override;
	static QJsonDocument load_json(const QString &, QByteArray &digest);
	static void conditionally_save_file(const QByteArray &contents, const QString &path, QByteArray &last_digest);

public:
	ImageViewerApplication(int &argc, char **argv, const QString &unique_name);
	~ImageViewerApplication();
	bool get_clamp_to_edges() const{
		return this->settings.get_clamp_to_edges();
	}
	int get_clamp_strength() const{
		return this->settings.get_clamp_strength();
	}
	bool get_use_checkerboard_pattern() const{
		return this->settings.get_use_checkerboard_pattern();
	}
	bool get_center_when_displayed() const{
		return this->settings.get_center_when_displayed();
	}
	ZoomMode get_zoom_mode_for_new_windows() const{
		return this->settings.get_zoom_mode_for_new_windows();
	}
	ZoomMode get_fullscreen_zoom_mode_for_new_windows() const{
		return this->settings.get_fullscreen_zoom_mode_for_new_windows();
	}
	void minimize_all();
	std::shared_ptr<QMenu> build_context_menu(MainWindow *caller = nullptr);
	const MainSettings &get_option_values() const{
		return this->settings;
	}
	QImage load_image(const QString &);
	std::unique_ptr<QMovie> load_animation(const QString &);
	bool is_animation(const QString &);

public slots:
	void window_closing(MainWindow *);
	void resolution_change(int screen);
	void work_area_change(int screen);
	void quit_slot(){
		this->quit();
	}
};

QString get_per_user_unique_id();

#endif // IMAGEVIEWERAPPLICATION_H
