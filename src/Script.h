/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#pragma once

#include <memory>
#include <vector>
#include <limits>
#include <chrono>
#include <string>
#include <QChar>
#include <deque>
#include <QString>

class QString;
class ImageViewport;

class ParserException : public std::exception{
	std::string error;
public:
	ParserException(){
		this->error = "parsing error";
	}
	ParserException(const char *s){
		using namespace std::string_literals;
		this->error = "parsing error: "s + s;
	}
	ParserException(const std::string &s){
		this->error = "parsing error: " + s;
	}
	char const *what() const override{
		return this->error.c_str();
	}
};

class InterpreterState{
public:
	size_t currently_running = 0;
	size_t last_line = std::numeric_limits<size_t>::max();
	static const size_t invalid_last_while = std::numeric_limits<size_t>::max();
	size_t last_while = invalid_last_while;
};

class ScriptCommand{
public:
	virtual ~ScriptCommand(){}
	static std::unique_ptr<ScriptCommand> parse(std::deque<QChar> &&);
	virtual void resume(InterpreterState &, ImageViewport &) = 0;
};

class Script{
	std::vector<std::unique_ptr<ScriptCommand>> commands;
	InterpreterState state;
	void process_line(std::deque<QChar> &&);
public:
	Script(const QString &path);
	Script(const Script &) = delete;
	Script &operator=(const Script &) = delete;
	Script(Script &&other){
		*this = std::move(other);
	}
	Script &operator=(Script &&other){
		this->commands = std::move(other.commands);
		this->state = other.state;
		other.state = {};
		return *this;
	}
	bool resume(ImageViewport &);
};

typedef std::pair<int, bool> relabsint;
typedef std::pair<double, bool> relabsdouble;

int expect_integer(std::deque<QChar> &input);
double expect_real(std::deque<QChar> &input);
relabsint expect_relabs_integer(std::deque<QChar> &input);
relabsdouble expect_relabs_real(std::deque<QChar> &input);
int expect_integer(const QString &input);
double expect_real(const QString &input);
relabsint expect_relabs_integer(const QString &input);
relabsdouble expect_relabs_real(const QString &input);
std::deque<QChar> to_queue(const QString &s);
