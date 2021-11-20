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

class QAction;
class CustomProtocolHandler;
struct lua_State;

class NoWindowsException : public std::exception{};

class ImageViewerApplication : public SingleInstanceApplication{
	Q_OBJECT

	typedef std::shared_ptr<MainWindow> sharedp_t;
	std::map<uintptr_t, sharedp_t> windows;
	std::vector<std::shared_ptr<QAction> > actions;
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

	void save_current_windows(std::vector<std::shared_ptr<WindowState>> &);
	void restore_current_windows(const std::vector<std::shared_ptr<WindowState>> &);
	QString get_config_location();
	QString get_config_subpath(QString &dst, const char *sub);
	QString get_settings_filename();
	QString get_state_filename();
	void setup_slots();
	void reset_tray_menu();

protected:
	void new_instance(const QStringList &args) override;
	void add_window(sharedp_t window);
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
	void turn_transparent(MainWindow &window, bool yes);

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
