/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "Script.h"
#include "ScriptCommand.h"
#include <QString>
#include <QFile>
#include <sstream>

using namespace std::string_literals;

Script::Script(const QString &path){
	QFile file(path);
	file.open(QFile::ReadOnly);
	if (!file.isOpen())
		return;

	int lineno = 1;
	try{
		bool seen_cr = false;
		std::deque<QChar> accum;
		for (auto c : QString::fromUtf8(file.readAll())){
			if (c == 10){
				this->process_line(std::move(accum));
				lineno++;
				seen_cr = false;
				continue;
			}
			if (c == 13){
				if (seen_cr){
					this->process_line(std::move(accum));
					lineno++;
					continue;
				}
				seen_cr = true;
				continue;
			}
			accum.push_back(c);
		}
		if (accum.size())
			this->process_line(std::move(accum));
	}catch (ParserException &e){
		std::stringstream stream;
		stream << "error while parsing line " << lineno << ": " << e.what();
		throw ParserException(stream.str());
	}
}

void Script::process_line(std::deque<QChar> &&line){
	auto cmd = ScriptCommand::parse(std::move(line));
	if (!cmd)
		return;
	this->commands.emplace_back(std::move(cmd));
}

bool Script::resume(ImageViewport &image){
	if (this->state.currently_running >= this->commands.size())
		return false;
	auto &cmd = *this->commands[this->state.currently_running];
	auto old = this->state.currently_running;
	cmd.resume(this->state, image);
	this->state.last_line = old;
	return true;
}

std::deque<QChar> to_queue(const QString &s){
	std::deque<QChar> ret;
	for (auto c : s)
		ret.push_back(c);
	return ret;
}

template <typename T>
T pop(std::deque<T> &q){
	auto ret = q.front();
	q.pop_front();
	return ret;
}

bool is_identifier_first_char(QChar c){
	return c.isLetter() || c == '_';
}

bool is_identifier_nth_char(QChar c){
	return is_identifier_first_char(c) || c.isDigit();
}

#define PEEK input.front()
#define POP pop(input)
#define EMPTY !input.size()

void skip_whitespace(std::deque<QChar> &input){
	while (!EMPTY && PEEK.isSpace())
		input.pop_front();
}

QString expect_identifier(std::deque<QChar> &input){
	skip_whitespace(input);
	if (EMPTY)
		throw ParserException("expected identifier but found end of line");
	if (!is_identifier_first_char(PEEK))
		throw ParserException("expected identifier but found "s + PEEK.toLatin1());
	QString ret;
	while (!EMPTY && is_identifier_nth_char(PEEK))
		ret += POP;
	return ret;
}

std::string to_string(const std::deque<QChar> &input){
	std::string ret;
	for (auto c : input)
		ret += c.toLatin1();
	return ret;
}

void expect_eol(std::deque<QChar> &input){
	skip_whitespace(input);
	if (!EMPTY)
		throw ParserException("expected EOL but found " + to_string(input));
}

int expect_integer(const QString &input){
	auto temp = to_queue(input);
	return expect_integer(temp);
}

double expect_real(const QString &input){
	auto temp = to_queue(input);
	return expect_real(temp);
}

relabsint expect_relabs_integer(const QString &input){
	auto temp = to_queue(input);
	return expect_relabs_integer(temp);
}

relabsdouble expect_relabs_real(const QString &input){
	auto temp = to_queue(input);
	return expect_relabs_real(temp);
}

int expect_positive_integer(std::deque<QChar> &input){
	if (EMPTY)
		throw ParserException("expected integer but found end of line");
	int ret = 0;
	while (!EMPTY && PEEK.isDigit()){
		if (ret > std::numeric_limits<int>::max() / 10)
			throw ParserException("overflow in integer literal");
		ret *= 10;
		ret += POP.digitValue();
	}
	return ret;
}

int expect_integer_no_whitespace(std::deque<QChar> &input){
	if (EMPTY)
		throw ParserException("expected integer but found end of line");
	int sign = 1;
	if (PEEK == '-'){
		POP;
		sign = -1;
	}
	return sign * expect_positive_integer(input);
}

int expect_integer(std::deque<QChar> &input){
	skip_whitespace(input);
	return expect_integer_no_whitespace(input);
}

double expect_exponent(std::deque<QChar> &input){
	if (EMPTY)
		throw ParserException("expected scientific notation exponent but found end of line");
	int sign;
	if (PEEK == '+'){
		POP;
		sign = 1;
	}else if (PEEK == '-'){
		POP;
		sign = -1;
	}else
		throw ParserException("expected scientific notation exponent but found invalid character " + PEEK.toLatin1());
	try{
		return sign * expect_positive_integer(input);
	}catch (ParserException &e){
		throw ParserException("while parsing scientific notation exponent got: "s + e.what());
	}
}

double expect_positive_real(std::deque<QChar> &input){
	if (EMPTY)
		throw ParserException("expected real but found end of line");
	double ret = 0;
	//Integral part
	while (!EMPTY && PEEK.isDigit()){
		ret *= 10;
		ret += POP.digitValue();
	}
	//Fractional part
	if (EMPTY || PEEK != '.')
		return ret;
	POP;
	double multiplier = 1;
	while (!EMPTY && PEEK.isDigit()){
		multiplier /= 10;
		ret += POP.digitValue() * multiplier;
	}
	if (EMPTY || PEEK.toLower() != 'e')
		return ret;
	POP;
	auto mantissa = ret;
	auto exponent = expect_exponent(input);
	return mantissa * pow(10, exponent);
}

double expect_real(std::deque<QChar> &input){
	skip_whitespace(input);
	if (EMPTY)
		throw ParserException("expected real but found end of line");
	double sign = 1;
	if (PEEK == '-'){
		POP;
		sign = -1;
	}
	return sign * expect_positive_real(input);
}

relabsint expect_relabs_integer(std::deque<QChar> &input){
	skip_whitespace(input);
	if (EMPTY)
		throw ParserException("expected integer but found end of line");
	bool relative = false;
	if (PEEK == '@'){
		POP;
		relative = true;
	}
	return { expect_integer(input), relative };
}

relabsdouble expect_relabs_real(std::deque<QChar> &input){
	skip_whitespace(input);
	if (EMPTY)
		throw ParserException("expected real but found end of line");
	bool relative = false;
	if (PEEK == '@'){
		POP;
		relative = true;
	}
	return{ expect_real(input), relative };
}

std::unique_ptr<ScriptCommand> expect_line(std::deque<QChar> &input){
	auto identifier = expect_identifier(input).toLower();
	if (identifier == "while"){
		expect_eol(input);
		return std::make_unique<WhileCommand>();
	}
	if (identifier == "endwhile"){
		expect_eol(input);
		return std::make_unique<EndWhileCommand>();
	}
	if (identifier == "scale"){
		auto scale = expect_real(input);
		expect_eol(input);
		return std::make_unique<ScaleCommand>(scale);
	}
	if (identifier == "setorigin"){
		auto x = expect_integer(input);
		auto y = expect_integer(input);
		expect_eol(input);
		return std::make_unique<SetOriginCommand>(x, y);
	}
	if (identifier == "move"){
		auto x = expect_relabs_integer(input);
		auto y = expect_relabs_integer(input);
		expect_eol(input);
		return std::make_unique<MoveCommand>(x, y);
	}
	if (identifier == "rotate"){
		auto theta = expect_relabs_real(input);
		expect_eol(input);
		return std::make_unique<RotateCommand>(theta);
	}
	if (identifier == "fliph"){
		expect_eol(input);
		return std::make_unique<FlipHCommand>();
	}
	if (identifier == "flipv"){
		expect_eol(input);
		return std::make_unique<FlipVCommand>();
	}
	if (identifier == "animmove"){
		auto x = expect_integer(input);
		auto y = expect_integer(input);
		auto speed = expect_real(input);
		expect_eol(input);
		return std::make_unique<AnimMoveCommand>(x, y, speed);
	}
	if (identifier == "animrotate"){
		auto speed = expect_real(input);
		return std::make_unique<AnimRotateCommand>(speed);
	}
	if (identifier == "wait"){
		auto animmove = expect_identifier(input).toLower();
		if (animmove != "animmove")
			throw ParserException("expected EOL but found " + animmove.toStdString());
		auto x = expect_integer(input);
		auto y = expect_integer(input);
		auto speed = expect_real(input);
		expect_eol(input);
		return std::make_unique<WaitAnimMoveCommand>(x, y, speed);
	}
	throw ParserException("Unknown command: " + identifier.toStdString());
}

std::unique_ptr<ScriptCommand> ScriptCommand::parse(std::deque<QChar> &&input){
	return expect_line(input);
}
