/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#pragma once

#include "Script.h"

class WhileCommand : public ScriptCommand{
public:
	void resume(InterpreterState &state, ImageViewport &image) override;
};

class EndWhileCommand : public ScriptCommand{
public:
	void resume(InterpreterState &state, ImageViewport &image) override;
};

class ScaleCommand : public ScriptCommand{
	double scale;
public:
	ScaleCommand(double scale): scale(scale){}
	void resume(InterpreterState &state, ImageViewport &image) override;
};

class SetOriginCommand : public ScriptCommand{
	int x;
	int y;
public:
	SetOriginCommand(int x, int y): x(x), y(y){}
	void resume(InterpreterState &state, ImageViewport &image) override;
};

class MoveCommand : public ScriptCommand{
	relabsint x;
	relabsint y;
public:
	MoveCommand(relabsint x, relabsint y): x(x), y(y){}
	void resume(InterpreterState &state, ImageViewport &image) override;
};

class RotateCommand : public ScriptCommand{
	relabsdouble theta;
public:
	RotateCommand(relabsdouble theta): theta(theta){}
	void resume(InterpreterState &state, ImageViewport &image) override;
};

class FlipHCommand : public ScriptCommand{
public:
	FlipHCommand(){}
	void resume(InterpreterState &state, ImageViewport &image) override;
};

class FlipVCommand : public ScriptCommand{
public:
	FlipVCommand(){}
	void resume(InterpreterState &state, ImageViewport &image) override;
};

class AnimMoveCommand : public ScriptCommand{
protected:
	int x;
	int y;
	double speed;
public:
	AnimMoveCommand(int x, int y, double speed): x(x), y(y), speed(speed){}
	virtual ~AnimMoveCommand(){}
	void resume(InterpreterState &state, ImageViewport &image) override;
};

class WaitAnimMoveCommand : public AnimMoveCommand{
public:
	WaitAnimMoveCommand(int x, int y, double speed): AnimMoveCommand(x, y, speed){}
	void resume(InterpreterState &state, ImageViewport &image) override;
};

class AnimRotateCommand : public ScriptCommand{
	double speed;
public:
	AnimRotateCommand(double speed): speed(speed){}
	void resume(InterpreterState &state, ImageViewport &image) override;
};
