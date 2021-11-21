/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#include "ScriptCommand.h"
#include "ImageViewport.h"

void WhileCommand::resume(InterpreterState &state, ImageViewport &image){
	state.last_while = state.currently_running++;
}

void EndWhileCommand::resume(InterpreterState &state, ImageViewport &image){
	state.currently_running = state.last_while;
}

void ScaleCommand::resume(InterpreterState &state, ImageViewport &image){
	image.set_scale(this->scale);
	state.currently_running++;
}

void SetOriginCommand::resume(InterpreterState &state, ImageViewport &image){
	image.set_origin(this->x, this->y);
	state.currently_running++;
}

void MoveCommand::resume(InterpreterState &state, ImageViewport &image){
	image.move_by_command(set(image.get_position(), this->x, this->y));
	state.currently_running++;
}

void RotateCommand::resume(InterpreterState &state, ImageViewport &image){
	auto rotation = image.get_rotation();
	if (this->theta.second)
		rotation += this->theta.first;
	else
		rotation = this->theta.first;
	image.set_rotation(rotation);
	state.currently_running++;
}

void AnimMoveCommand::resume(InterpreterState &state, ImageViewport &image){
	image.anim_move(this->x, this->y, this->speed);
	state.currently_running++;
}

void AnimRotateCommand::resume(InterpreterState &state, ImageViewport &image){
	image.anim_rotate(this->speed);
	state.currently_running++;
}

void FlipHCommand::resume(InterpreterState &state, ImageViewport &image){
	image.fliph();
	state.currently_running++;
}

void FlipVCommand::resume(InterpreterState &state, ImageViewport &image){
	image.flipv();
	state.currently_running++;
}

void WaitAnimMoveCommand::resume(InterpreterState &state, ImageViewport &image){
	if (state.last_line != state.currently_running){
		image.anim_move(this->x, this->y, this->speed, [&state](){
			state.currently_running++;
		});
	}
}
